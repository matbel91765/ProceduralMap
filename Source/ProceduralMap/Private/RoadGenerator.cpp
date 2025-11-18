// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "RoadGenerator.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "BiomeGenerator.h"

ARoadGenerator::ARoadGenerator()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RoadMesh"));
	RoadMesh->SetupAttachment(RootComponent);

	// Default road parameters
	RoadWidth = 400.0f;
	RoadThickness = 20.0f;
	HeightOffset = 5.0f;
	SegmentsPerSection = 10;
	bEnableBanking = true;
	MaxBankingAngle = 15.0f;

	// Pathfinding settings
	PathfindingStepSize = 500.0f;
	MaxSlope = 20.0f;
	SlopePenalty = 5.0f;
	bPreferExistingRoads = true;
	RoadMergeDistance = 500.0f;

	// Terrain integration
	TargetLandscape = nullptr;
	bFlattenTerrain = false;
	FlatteningRadius = 600.0f;
	TerrainSmoothness = 0.5f;

	// Materials
	RoadMaterial = nullptr;
	IntersectionMaterial = nullptr;

	// PCG
	bEnablePCGDecoration = false;

	// Initialize biome cost multipliers
	BiomeCostMultipliers.Add(EBiomeType::Plains, 1.0f);
	BiomeCostMultipliers.Add(EBiomeType::Desert, 1.2f);
	BiomeCostMultipliers.Add(EBiomeType::Tundra, 1.5f);
	BiomeCostMultipliers.Add(EBiomeType::Mountains, 3.0f);
	BiomeCostMultipliers.Add(EBiomeType::Forest, 1.8f);
	BiomeCostMultipliers.Add(EBiomeType::Rainforest, 2.5f);
	BiomeCostMultipliers.Add(EBiomeType::Swamp, 2.0f);

	bNetworkGenerated = false;
}

void ARoadGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void ARoadGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ARoadGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARoadGenerator::GenerateRoadNetwork()
{
	ClearRoads();

	if (PointsOfInterest.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoadGenerator: Need at least 2 POIs to generate road network"));
		return;
	}

	// Sort POIs by priority
	TArray<FPointOfInterest> SortedPOIs = PointsOfInterest;
	SortedPOIs.Sort([](const FPointOfInterest& A, const FPointOfInterest& B) {
		return A.Priority > B.Priority;
	});

	// Generate roads using minimum spanning tree approach
	TArray<bool> Connected;
	Connected.Init(false, SortedPOIs.Num());
	Connected[0] = true;

	int32 ConnectedCount = 1;

	while (ConnectedCount < SortedPOIs.Num())
	{
		float ShortestDistance = FLT_MAX;
		int32 BestFrom = -1;
		int32 BestTo = -1;

		// Find shortest connection between connected and unconnected POIs
		for (int32 i = 0; i < SortedPOIs.Num(); i++)
		{
			if (!Connected[i]) continue;

			for (int32 j = 0; j < SortedPOIs.Num(); j++)
			{
				if (Connected[j]) continue;

				float Distance = FVector::Dist(SortedPOIs[i].Location, SortedPOIs[j].Location);
				if (Distance < ShortestDistance)
				{
					ShortestDistance = Distance;
					BestFrom = i;
					BestTo = j;
				}
			}
		}

		if (BestFrom >= 0 && BestTo >= 0)
		{
			// Generate road segment
			GenerateRoadSegment(SortedPOIs[BestFrom].Location, SortedPOIs[BestTo].Location);
			Connected[BestTo] = true;
			ConnectedCount++;
		}
		else
		{
			break;
		}
	}

	// Detect and create intersections
	DetectIntersections();

	// Merge nearby roads
	if (bPreferExistingRoads)
	{
		MergeNearbyRoads();
	}

	bNetworkGenerated = true;
	UE_LOG(LogTemp, Log, TEXT("RoadGenerator: Network generated with %d segments and %d intersections"),
		RoadSegments.Num(), Intersections.Num());
}

int32 ARoadGenerator::GenerateRoadSegment(const FVector& StartPoint, const FVector& EndPoint)
{
	// Find optimal path
	TArray<FVector> PathPoints = FindOptimalPath(StartPoint, EndPoint);

	if (PathPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoadGenerator: Failed to find path between points"));
		return -1;
	}

	// Smooth path
	PathPoints = SmoothRoadPath(PathPoints, 3);

	// Create spline component
	USplineComponent* RoadSpline = NewObject<USplineComponent>(this);
	RoadSpline->RegisterComponent();
	RoadSpline->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RoadSpline->SetClosedLoop(false);

	// Populate spline
	for (int32 i = 0; i < PathPoints.Num(); i++)
	{
		FVector LocalPos = GetActorTransform().InverseTransformPosition(PathPoints[i]);
		RoadSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
	}

	RoadSpline->UpdateSpline();
	RoadSplines.Add(RoadSpline);

	// Create road segment data
	FRoadSegment Segment;
	Segment.StartPoint = StartPoint;
	Segment.EndPoint = EndPoint;
	Segment.SegmentSpline = RoadSpline;
	Segment.RoadWidth = RoadWidth;

	int32 SegmentIndex = RoadSegments.Add(Segment);

	// Generate mesh
	GenerateRoadMesh(RoadSpline, SegmentIndex);

	// Flatten terrain if enabled
	if (bFlattenTerrain && TargetLandscape)
	{
		FlattenTerrainAlongRoad(RoadSpline);
	}

	return SegmentIndex;
}

TArray<FVector> ARoadGenerator::FindOptimalPath(const FVector& Start, const FVector& End)
{
	return FindPathAStar(Start, End);
}

TArray<FVector> ARoadGenerator::FindPathAStar(const FVector& Start, const FVector& End)
{
	TArray<FRoadPathNode> OpenList;
	TArray<FRoadPathNode> ClosedList;
	TArray<FVector> FinalPath;

	// Initialize start node
	FRoadPathNode StartNode;
	StartNode.Position = Start;
	StartNode.GCost = 0.0f;
	StartNode.HCost = CalculateHeuristic(Start, End);
	StartNode.ParentIndex = -1;
	OpenList.Add(StartNode);

	int32 MaxIterations = 10000;
	int32 Iteration = 0;

	while (OpenList.Num() > 0 && Iteration < MaxIterations)
	{
		Iteration++;

		// Find node with lowest F cost
		int32 CurrentIndex = 0;
		float LowestFCost = OpenList[0].GetFCost();

		for (int32 i = 1; i < OpenList.Num(); i++)
		{
			float FCost = OpenList[i].GetFCost();
			if (FCost < LowestFCost)
			{
				LowestFCost = FCost;
				CurrentIndex = i;
			}
		}

		FRoadPathNode CurrentNode = OpenList[CurrentIndex];
		OpenList.RemoveAt(CurrentIndex);
		ClosedList.Add(CurrentNode);

		// Check if reached destination
		float DistToEnd = FVector::Dist2D(CurrentNode.Position, End);
		if (DistToEnd < PathfindingStepSize * 2.0f)
		{
			// Reconstruct path
			FinalPath.Add(End);
			FinalPath.Add(CurrentNode.Position);

			int32 ParentIdx = CurrentNode.ParentIndex;
			while (ParentIdx >= 0 && ParentIdx < ClosedList.Num())
			{
				FinalPath.Add(ClosedList[ParentIdx].Position);
				ParentIdx = ClosedList[ParentIdx].ParentIndex;
			}

			Algo::Reverse(FinalPath);
			return FinalPath;
		}

		// Get neighbors
		TArray<FVector> Neighbors = GetNeighborPositions(CurrentNode.Position);

		for (const FVector& NeighborPos : Neighbors)
		{
			// Check if in closed list
			bool bInClosedList = false;
			for (const FRoadPathNode& ClosedNode : ClosedList)
			{
				if (FVector::Dist(ClosedNode.Position, NeighborPos) < PathfindingStepSize * 0.5f)
				{
					bInClosedList = true;
					break;
				}
			}

			if (bInClosedList || !IsValidRoadPosition(NeighborPos))
			{
				continue;
			}

			// Calculate cost
			float MoveCost = CalculateMovementCost(CurrentNode.Position, NeighborPos);
			float NewGCost = CurrentNode.GCost + MoveCost;

			// Check if in open list
			int32 ExistingIndex = -1;
			for (int32 i = 0; i < OpenList.Num(); i++)
			{
				if (FVector::Dist(OpenList[i].Position, NeighborPos) < PathfindingStepSize * 0.5f)
				{
					ExistingIndex = i;
					break;
				}
			}

			if (ExistingIndex >= 0)
			{
				if (NewGCost < OpenList[ExistingIndex].GCost)
				{
					OpenList[ExistingIndex].GCost = NewGCost;
					OpenList[ExistingIndex].ParentIndex = ClosedList.Num() - 1;
				}
			}
			else
			{
				FRoadPathNode NewNode;
				NewNode.Position = NeighborPos;
				NewNode.GCost = NewGCost;
				NewNode.HCost = CalculateHeuristic(NeighborPos, End);
				NewNode.ParentIndex = ClosedList.Num() - 1;
				OpenList.Add(NewNode);
			}
		}
	}

	// Fallback: straight line
	UE_LOG(LogTemp, Warning, TEXT("RoadGenerator: A* failed, using straight line"));
	FinalPath.Add(Start);
	FinalPath.Add(End);
	return FinalPath;
}

float ARoadGenerator::CalculateMovementCost(const FVector& From, const FVector& To)
{
	float BaseCost = FVector::Dist(From, To);

	// Slope cost
	float Slope = GetTerrainSlope(To);
	if (Slope > MaxSlope)
	{
		BaseCost += (Slope - MaxSlope) * SlopePenalty * 100.0f;
	}
	else
	{
		BaseCost += Slope * SlopePenalty;
	}

	// Biome cost
	EBiomeType BiomeType = GetBiomeAtPosition(To);
	float BiomeMultiplier = GetBiomeCostMultiplier(BiomeType);
	BaseCost *= BiomeMultiplier;

	// Existing road preference
	if (bPreferExistingRoads && RoadSegments.Num() > 0)
	{
		float DistanceToRoad = 0.0f;
		FindNearestRoadPoint(To, DistanceToRoad);

		if (DistanceToRoad < RoadMergeDistance)
		{
			float ProximityBonus = 1.0f - (DistanceToRoad / RoadMergeDistance);
			BaseCost *= (1.0f - ProximityBonus * 0.5f);
		}
	}

	return BaseCost;
}

void ARoadGenerator::GenerateRoadMesh(USplineComponent* RoadSpline, int32 SegmentIndex)
{
	if (!RoadSpline || !RoadMesh)
	{
		return;
	}

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	float SplineLength = RoadSpline->GetSplineLength();
	int32 NumSegments = FMath::CeilToInt(SplineLength / (PathfindingStepSize / SegmentsPerSection));

	for (int32 i = 0; i < NumSegments; i++)
	{
		float Distance1 = (i / (float)NumSegments) * SplineLength;
		float Distance2 = ((i + 1) / (float)NumSegments) * SplineLength;

		FVector Point1 = RoadSpline->GetLocationAtDistanceAlongSpline(Distance1, ESplineCoordinateSpace::World);
		FVector Point2 = RoadSpline->GetLocationAtDistanceAlongSpline(Distance2, ESplineCoordinateSpace::World);

		// Adjust height
		Point1.Z = GetTerrainHeight(Point1) + HeightOffset;
		Point2.Z = GetTerrainHeight(Point2) + HeightOffset;

		// Get direction and right vector
		FVector Direction = (Point2 - Point1).GetSafeNormal();
		FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

		// Calculate banking if enabled
		float BankingAngle = 0.0f;
		if (bEnableBanking)
		{
			BankingAngle = CalculateBankingAngle(RoadSpline, Distance1);
		}

		// Apply banking rotation to right vector
		if (FMath::Abs(BankingAngle) > KINDA_SMALL_NUMBER)
		{
			FQuat BankingRotation = FQuat(Direction, FMath::DegreesToRadians(BankingAngle));
			Right = BankingRotation.RotateVector(Right);
		}

		// Create quad vertices
		float HalfWidth = RoadWidth * 0.5f;
		int32 BaseIndex = Vertices.Num();

		Vertices.Add(Point1 - Right * HalfWidth);
		Vertices.Add(Point1 + Right * HalfWidth);
		Vertices.Add(Point2 - Right * HalfWidth);
		Vertices.Add(Point2 + Right * HalfWidth);

		// Normals
		FVector Normal = FVector::UpVector;
		for (int32 j = 0; j < 4; j++)
		{
			Normals.Add(Normal);
		}

		// UVs
		float UV_V1 = Distance1 / SplineLength;
		float UV_V2 = Distance2 / SplineLength;
		UVs.Add(FVector2D(0.0f, UV_V1));
		UVs.Add(FVector2D(1.0f, UV_V1));
		UVs.Add(FVector2D(0.0f, UV_V2));
		UVs.Add(FVector2D(1.0f, UV_V2));

		// Triangles
		Triangles.Add(BaseIndex + 0);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 1);

		Triangles.Add(BaseIndex + 1);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 3);
	}

	// Create mesh section
	RoadMesh->CreateMeshSection_LinearColor(SegmentIndex, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	if (RoadMaterial)
	{
		RoadMesh->SetMaterial(SegmentIndex, RoadMaterial);
	}
}

void ARoadGenerator::DetectIntersections()
{
	Intersections.Empty();

	// Check all road segment pairs for intersections
	for (int32 i = 0; i < RoadSegments.Num(); i++)
	{
		for (int32 j = i + 1; j < RoadSegments.Num(); j++)
		{
			FVector IntersectionPoint;
			if (CheckSegmentIntersection(RoadSegments[i], RoadSegments[j], IntersectionPoint))
			{
				FRoadIntersection Intersection;
				Intersection.Location = IntersectionPoint;
				Intersection.ConnectedRoadIndices.Add(i);
				Intersection.ConnectedRoadIndices.Add(j);
				Intersection.IntersectionType = 1; // Cross

				Intersections.Add(Intersection);

				// Create intersection mesh
				CreateIntersectionMesh(IntersectionPoint, Intersection.IntersectionType);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RoadGenerator: Detected %d intersections"), Intersections.Num());
}

void ARoadGenerator::CreateIntersectionMesh(const FVector& Location, int32 IntersectionType)
{
	// Create a simple circular intersection mesh
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;

	float IntersectionRadius = RoadWidth * 1.5f;
	int32 Segments = 16;
	FVector Center = Location;
	Center.Z = GetTerrainHeight(Center) + HeightOffset;

	// Center vertex
	Vertices.Add(Center);
	Normals.Add(FVector::UpVector);
	UVs.Add(FVector2D(0.5f, 0.5f));

	// Ring vertices
	for (int32 i = 0; i <= Segments; i++)
	{
		float Angle = (i / (float)Segments) * 2.0f * PI;
		FVector Offset(FMath::Cos(Angle) * IntersectionRadius, FMath::Sin(Angle) * IntersectionRadius, 0);

		Vertices.Add(Center + Offset);
		Normals.Add(FVector::UpVector);
		UVs.Add(FVector2D(0.5f + FMath::Cos(Angle) * 0.5f, 0.5f + FMath::Sin(Angle) * 0.5f));
	}

	// Create triangles
	for (int32 i = 1; i <= Segments; i++)
	{
		Triangles.Add(0);
		Triangles.Add(i);
		Triangles.Add(i + 1);
	}

	// Create mesh section
	int32 SectionIndex = RoadSegments.Num() + Intersections.Num();
	RoadMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);

	if (IntersectionMaterial)
	{
		RoadMesh->SetMaterial(SectionIndex, IntersectionMaterial);
	}
}

void ARoadGenerator::FlattenTerrainAlongRoad(USplineComponent* RoadSpline)
{
	if (!TargetLandscape || !RoadSpline)
	{
		return;
	}

	// Implementation would use landscape editing API
	UE_LOG(LogTemp, Log, TEXT("RoadGenerator: Terrain flattening along road"));
}

void ARoadGenerator::ClearRoads()
{
	RoadSegments.Empty();
	Intersections.Empty();

	for (USplineComponent* Spline : RoadSplines)
	{
		if (Spline)
		{
			Spline->DestroyComponent();
		}
	}
	RoadSplines.Empty();

	if (RoadMesh)
	{
		RoadMesh->ClearAllMeshSections();
	}

	HeightCache.Empty();
	BiomeCache.Empty();
	bNetworkGenerated = false;
}

float ARoadGenerator::GetTerrainHeight(const FVector& WorldPosition) const
{
	// Check cache
	FIntPoint CacheKey(FMath::FloorToInt(WorldPosition.X / 100.0f), FMath::FloorToInt(WorldPosition.Y / 100.0f));

	if (HeightCache.Contains(CacheKey))
	{
		return HeightCache[CacheKey];
	}

	float Height = WorldPosition.Z;

	if (TargetLandscape)
	{
		ULandscapeInfo* LandscapeInfo = TargetLandscape->GetLandscapeInfo();
		if (LandscapeInfo)
		{
			Height = LandscapeInfo->GetCachedZ(WorldPosition.X, WorldPosition.Y);
		}
	}
	else
	{
		// Fallback: line trace
		UWorld* World = GetWorld();
		if (World)
		{
			FHitResult HitResult;
			FVector Start = WorldPosition + FVector(0, 0, 10000.0f);
			FVector End = WorldPosition - FVector(0, 0, 10000.0f);

			if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
			{
				Height = HitResult.Location.Z;
			}
		}
	}

	HeightCache.Add(CacheKey, Height);
	return Height;
}

float ARoadGenerator::GetTerrainSlope(const FVector& WorldPosition) const
{
	float SampleDist = 100.0f;

	float HeightCenter = GetTerrainHeight(WorldPosition);
	float HeightX = GetTerrainHeight(WorldPosition + FVector(SampleDist, 0, 0));
	float HeightY = GetTerrainHeight(WorldPosition + FVector(0, SampleDist, 0));

	float SlopeX = FMath::Abs(HeightX - HeightCenter) / SampleDist;
	float SlopeY = FMath::Abs(HeightY - HeightCenter) / SampleDist;

	float MaxSlope = FMath::Max(SlopeX, SlopeY);
	return FMath::RadiansToDegrees(FMath::Atan(MaxSlope));
}

EBiomeType ARoadGenerator::GetBiomeAtPosition(const FVector& WorldPosition) const
{
	// Check cache
	FIntPoint CacheKey(FMath::FloorToInt(WorldPosition.X / 100.0f), FMath::FloorToInt(WorldPosition.Y / 100.0f));

	if (BiomeCache.Contains(CacheKey))
	{
		return BiomeCache[CacheKey];
	}

	// Try to find biome generator in world
	EBiomeType BiomeType = EBiomeType::Plains; // Default

	UWorld* World = GetWorld();
	if (World)
	{
		TArray<AActor*> BiomeGenerators;
		UGameplayStatics::GetAllActorsOfClass(World, ABiomeGenerator::StaticClass(), BiomeGenerators);

		if (BiomeGenerators.Num() > 0)
		{
			ABiomeGenerator* Generator = Cast<ABiomeGenerator>(BiomeGenerators[0]);
			if (Generator)
			{
				float Height = GetTerrainHeight(WorldPosition);
				float NormalizedHeight = Height / 1000.0f; // Normalize to 0-1 range
				BiomeType = Generator->GetBiomeFromClimate(0.5f, 0.5f, NormalizedHeight);
			}
		}
	}

	BiomeCache.Add(CacheKey, BiomeType);
	return BiomeType;
}

bool ARoadGenerator::CheckSegmentIntersection(const FRoadSegment& Segment1, const FRoadSegment& Segment2, FVector& OutIntersection)
{
	if (!Segment1.SegmentSpline || !Segment2.SegmentSpline)
	{
		return false;
	}

	// Sample both splines and check for intersections
	float Length1 = Segment1.SegmentSpline->GetSplineLength();
	float Length2 = Segment2.SegmentSpline->GetSplineLength();

	int32 Samples1 = FMath::Max(10, FMath::CeilToInt(Length1 / 500.0f));
	int32 Samples2 = FMath::Max(10, FMath::CeilToInt(Length2 / 500.0f));

	for (int32 i = 0; i < Samples1; i++)
	{
		float Dist1 = (i / (float)Samples1) * Length1;
		FVector Point1 = Segment1.SegmentSpline->GetLocationAtDistanceAlongSpline(Dist1, ESplineCoordinateSpace::World);

		for (int32 j = 0; j < Samples2; j++)
		{
			float Dist2 = (j / (float)Samples2) * Length2;
			FVector Point2 = Segment2.SegmentSpline->GetLocationAtDistanceAlongSpline(Dist2, ESplineCoordinateSpace::World);

			float Distance = FVector::Dist2D(Point1, Point2);
			if (Distance < RoadWidth)
			{
				OutIntersection = (Point1 + Point2) * 0.5f;
				return true;
			}
		}
	}

	return false;
}

TArray<FVector> ARoadGenerator::GetNeighborPositions(const FVector& Position) const
{
	TArray<FVector> Neighbors;

	// 8 directions
	const FVector Directions[] = {
		FVector(1, 0, 0),
		FVector(-1, 0, 0),
		FVector(0, 1, 0),
		FVector(0, -1, 0),
		FVector(1, 1, 0),
		FVector(-1, -1, 0),
		FVector(1, -1, 0),
		FVector(-1, 1, 0)
	};

	for (const FVector& Dir : Directions)
	{
		FVector Neighbor = Position + Dir.GetSafeNormal() * PathfindingStepSize;
		Neighbor.Z = GetTerrainHeight(Neighbor);
		Neighbors.Add(Neighbor);
	}

	return Neighbors;
}

float ARoadGenerator::CalculateHeuristic(const FVector& From, const FVector& To) const
{
	return FVector::Dist2D(From, To);
}

bool ARoadGenerator::IsValidRoadPosition(const FVector& Position) const
{
	// Check slope
	float Slope = GetTerrainSlope(Position);
	if (Slope > MaxSlope * 1.5f) // Allow some tolerance
	{
		return false;
	}

	return true;
}

TArray<FVector> ARoadGenerator::SmoothRoadPath(const TArray<FVector>& Path, int32 Iterations) const
{
	if (Path.Num() < 3)
	{
		return Path;
	}

	TArray<FVector> SmoothedPath = Path;

	for (int32 Iter = 0; Iter < Iterations; Iter++)
	{
		TArray<FVector> TempPath;
		TempPath.Add(SmoothedPath[0]); // Keep first point

		for (int32 i = 1; i < SmoothedPath.Num() - 1; i++)
		{
			FVector Prev = SmoothedPath[i - 1];
			FVector Current = SmoothedPath[i];
			FVector Next = SmoothedPath[i + 1];

			// Average position
			FVector Smoothed = (Prev + Current * 2.0f + Next) * 0.25f;
			Smoothed.Z = GetTerrainHeight(Smoothed);

			TempPath.Add(Smoothed);
		}

		TempPath.Add(SmoothedPath.Last()); // Keep last point
		SmoothedPath = TempPath;
	}

	return SmoothedPath;
}

float ARoadGenerator::CalculateBankingAngle(USplineComponent* Spline, float Distance) const
{
	if (!Spline)
	{
		return 0.0f;
	}

	// Calculate curvature
	FVector Tangent1 = Spline->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	FVector Tangent2 = Spline->GetTangentAtDistanceAlongSpline(Distance + 100.0f, ESplineCoordinateSpace::World);

	float Curvature = FVector::CrossProduct(Tangent1, Tangent2).Size();

	// Banking angle proportional to curvature
	float BankingAngle = Curvature * MaxBankingAngle;
	return FMath::Clamp(BankingAngle, -MaxBankingAngle, MaxBankingAngle);
}

void ARoadGenerator::MergeNearbyRoads()
{
	// Implementation for merging roads that come close together
	UE_LOG(LogTemp, Log, TEXT("RoadGenerator: Merging nearby roads"));
}

FVector ARoadGenerator::FindNearestRoadPoint(const FVector& Position, float& OutDistance) const
{
	FVector NearestPoint = Position;
	OutDistance = FLT_MAX;

	for (const FRoadSegment& Segment : RoadSegments)
	{
		if (!Segment.SegmentSpline)
		{
			continue;
		}

		float SplineLength = Segment.SegmentSpline->GetSplineLength();
		int32 Samples = FMath::Max(10, FMath::CeilToInt(SplineLength / 100.0f));

		for (int32 i = 0; i < Samples; i++)
		{
			float Distance = (i / (float)Samples) * SplineLength;
			FVector Point = Segment.SegmentSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

			float Dist = FVector::Dist(Point, Position);
			if (Dist < OutDistance)
			{
				OutDistance = Dist;
				NearestPoint = Point;
			}
		}
	}

	return NearestPoint;
}

float ARoadGenerator::GetBiomeCostMultiplier(EBiomeType BiomeType) const
{
	if (BiomeCostMultipliers.Contains(BiomeType))
	{
		return BiomeCostMultipliers[BiomeType];
	}

	return 1.0f; // Default multiplier
}
