// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "RiverGenerator.h"
#include "Landscape.h"
#include "LandscapeEdit.h"
#include "LandscapeInfo.h"
#include "WaterBodyRiverActor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "FastNoiseLite.h"

ARiverGenerator::ARiverGenerator()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	RiverSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RiverSpline"));
	RiverSpline->SetupAttachment(RootComponent);
	RiverSpline->SetClosedLoop(false);

	// Default parameters
	Width = 500.0f;
	Depth = 50.0f;
	FlowSpeed = 100.0f;
	Curvature = 0.3f;
	WidthVariation = 0.2f;

	// Generation settings
	SourceLocation = FVector(0.0f, 0.0f, 1000.0f);
	DestinationLocation = FVector(10000.0f, 10000.0f, 0.0f);
	bAutoFindDestination = false;
	DestinationSearchRadius = 5000.0f;
	PathfindingStepSize = 500.0f;
	DownhillBias = 2.0f;

	// Noise settings
	bApplyCurlNoise = true;
	NoiseFrequency = 0.001f;
	NoiseAmplitude = 200.0f;
	NoiseSeed = 12345;

	// Landscape modification
	TargetLandscape = nullptr;
	bModifyLandscape = false;
	SmoothingRadius = 1000.0f;
	FalloffPower = 2.0f;

	// Water system
	bCreateWaterBody = true;
	WaterBodyRiver = nullptr;

	bRiverGenerated = false;
}

void ARiverGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void ARiverGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ARiverGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARiverGenerator::GenerateRiver()
{
	ClearRiver();

	// Find destination if auto mode
	FVector FinalDestination = DestinationLocation;
	if (bAutoFindDestination)
	{
		FinalDestination = FindLowestPoint(SourceLocation, DestinationSearchRadius);
	}

	// Find optimal path
	TArray<FVector> PathPoints = FindLowestPath(SourceLocation, FinalDestination);

	if (PathPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("RiverGenerator: Failed to find valid path"));
		return;
	}

	// Smooth path
	PathPoints = SmoothPath(PathPoints, 5);

	// Generate spline
	GenerateRiverSpline(PathPoints);

	// Apply curl noise distortion
	if (bApplyCurlNoise)
	{
		ApplyCurlNoise();
	}

	// Modify landscape
	if (bModifyLandscape && TargetLandscape)
	{
		ModifyLandscape();
	}

	// Create water body
	if (bCreateWaterBody)
	{
		CreateWaterBody();
	}

	// Generate tributaries
	GenerateTributaries();

	bRiverGenerated = true;
	UE_LOG(LogTemp, Log, TEXT("RiverGenerator: River generated successfully with %d points"), PathPoints.Num());
}

TArray<FVector> ARiverGenerator::FindLowestPath(const FVector& Start, const FVector& End)
{
	// Use A* for precise pathfinding
	return FindPathAStar(Start, End);
}

TArray<FVector> ARiverGenerator::FindPathAStar(const FVector& Start, const FVector& End)
{
	TArray<FRiverPathNode> OpenList;
	TArray<FRiverPathNode> ClosedList;
	TArray<FVector> FinalPath;

	// Initialize start node
	FRiverPathNode StartNode;
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

		FRiverPathNode CurrentNode = OpenList[CurrentIndex];
		OpenList.RemoveAt(CurrentIndex);
		ClosedList.Add(CurrentNode);

		// Check if we reached the destination
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
		TArray<FVector> Neighbors = GetNeighbors(CurrentNode.Position);

		for (const FVector& NeighborPos : Neighbors)
		{
			// Check if already in closed list
			bool bInClosedList = false;
			for (const FRiverPathNode& ClosedNode : ClosedList)
			{
				if (FVector::Dist(ClosedNode.Position, NeighborPos) < PathfindingStepSize * 0.5f)
				{
					bInClosedList = true;
					break;
				}
			}

			if (bInClosedList || !IsValidRiverPosition(NeighborPos))
			{
				continue;
			}

			// Calculate costs
			float MoveCost = FVector::Dist(CurrentNode.Position, NeighborPos);

			// Add elevation bias (prefer downhill)
			float CurrentHeight = GetTerrainHeight(CurrentNode.Position);
			float NeighborHeight = GetTerrainHeight(NeighborPos);
			float ElevationDelta = NeighborHeight - CurrentHeight;

			if (ElevationDelta > 0)
			{
				// Going uphill - penalize heavily
				MoveCost += ElevationDelta * DownhillBias * 10.0f;
			}
			else
			{
				// Going downhill - reward
				MoveCost += ElevationDelta * DownhillBias;
			}

			float NewGCost = CurrentNode.GCost + MoveCost;

			// Check if neighbor is already in open list
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
				FRiverPathNode NewNode;
				NewNode.Position = NeighborPos;
				NewNode.GCost = NewGCost;
				NewNode.HCost = CalculateHeuristic(NeighborPos, End);
				NewNode.ParentIndex = ClosedList.Num() - 1;
				OpenList.Add(NewNode);
			}
		}
	}

	// Fallback to gradient descent if A* fails
	UE_LOG(LogTemp, Warning, TEXT("RiverGenerator: A* pathfinding failed, using gradient descent"));
	return FindPathGradientDescent(Start, End);
}

TArray<FVector> ARiverGenerator::FindPathGradientDescent(const FVector& Start, const FVector& End)
{
	TArray<FVector> Path;
	Path.Add(Start);

	FVector CurrentPos = Start;
	int32 MaxSteps = 1000;
	int32 Step = 0;

	while (FVector::Dist2D(CurrentPos, End) > PathfindingStepSize && Step < MaxSteps)
	{
		Step++;

		// Find steepest descent direction
		TArray<FVector> Neighbors = GetNeighbors(CurrentPos);

		FVector BestNeighbor = CurrentPos;
		float LowestHeight = GetTerrainHeight(CurrentPos);

		// Also consider direction to end
		FVector ToEnd = (End - CurrentPos).GetSafeNormal2D();

		for (const FVector& Neighbor : Neighbors)
		{
			float NeighborHeight = GetTerrainHeight(Neighbor);

			// Combine height preference with direction to end
			FVector ToNeighbor = (Neighbor - CurrentPos).GetSafeNormal2D();
			float DirectionScore = FVector::DotProduct(ToNeighbor, ToEnd);

			float Score = NeighborHeight - (DirectionScore * 50.0f);

			if (Score < LowestHeight)
			{
				LowestHeight = Score;
				BestNeighbor = Neighbor;
			}
		}

		if (BestNeighbor.Equals(CurrentPos))
		{
			// Stuck, move toward end
			FVector Direction = (End - CurrentPos).GetSafeNormal2D();
			BestNeighbor = CurrentPos + Direction * PathfindingStepSize;
		}

		CurrentPos = BestNeighbor;
		Path.Add(CurrentPos);
	}

	Path.Add(End);
	return Path;
}

void ARiverGenerator::GenerateRiverSpline(const TArray<FVector>& PathPoints)
{
	if (!RiverSpline || PathPoints.Num() < 2)
	{
		return;
	}

	RiverSpline->ClearSplinePoints();
	CachedPathPoints = PathPoints;

	for (int32 i = 0; i < PathPoints.Num(); i++)
	{
		FVector LocalPos = GetActorTransform().InverseTransformPosition(PathPoints[i]);
		RiverSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);

		// Vary width along spline
		float WidthAtPoint = Width * (1.0f + FMath::FRandRange(-WidthVariation, WidthVariation));
		RiverSpline->SetScaleAtSplinePoint(i, FVector(WidthAtPoint / 100.0f, 1.0f, 1.0f));
	}

	RiverSpline->UpdateSpline();
}

void ARiverGenerator::ApplyCurlNoise()
{
	if (!RiverSpline)
	{
		return;
	}

	int32 NumPoints = RiverSpline->GetNumberOfSplinePoints();

	for (int32 i = 1; i < NumPoints - 1; i++) // Skip first and last
	{
		FVector WorldPos = RiverSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

		// Calculate curl noise
		FVector2D Curl = CalculateCurlNoise(WorldPos);

		// Apply displacement
		FVector Displacement(Curl.X * NoiseAmplitude, Curl.Y * NoiseAmplitude, 0.0f);
		FVector NewPos = WorldPos + Displacement;

		// Update spline point
		FVector LocalPos = GetActorTransform().InverseTransformPosition(NewPos);
		RiverSpline->SetLocationAtSplinePoint(i, LocalPos, ESplineCoordinateSpace::Local, false);
	}

	RiverSpline->UpdateSpline();
}

void ARiverGenerator::ModifyLandscape()
{
	if (!TargetLandscape || !RiverSpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("RiverGenerator: Cannot modify landscape - missing references"));
		return;
	}

	ULandscapeInfo* LandscapeInfo = TargetLandscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		return;
	}

	// Get spline points
	int32 NumPoints = RiverSpline->GetNumberOfSplinePoints();
	float SplineLength = RiverSpline->GetSplineLength();
	int32 SampleCount = FMath::CeilToInt(SplineLength / 100.0f); // Sample every 100 cm

	for (int32 i = 0; i < SampleCount; i++)
	{
		float Distance = (i / (float)SampleCount) * SplineLength;
		FVector WorldPos = RiverSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		float TargetHeight = WorldPos.Z - Depth;
		ModifyLandscapeAtPoint(WorldPos, SmoothingRadius, TargetHeight);
	}

	UE_LOG(LogTemp, Log, TEXT("RiverGenerator: Landscape modification complete"));
}

void ARiverGenerator::CreateWaterBody()
{
	if (!WaterBodyClass || !RiverSpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("RiverGenerator: Cannot create water body - missing water body class"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Destroy old water body if exists
	if (WaterBodyRiver)
	{
		WaterBodyRiver->Destroy();
	}

	// Spawn new water body
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	WaterBodyRiver = World->SpawnActor<AWaterBodyRiver>(WaterBodyClass, GetActorTransform(), SpawnParams);

	if (WaterBodyRiver)
	{
		// Copy spline data to water body
		USplineComponent* WaterSpline = WaterBodyRiver->GetWaterSpline();
		if (WaterSpline)
		{
			WaterSpline->ClearSplinePoints();

			int32 NumPoints = RiverSpline->GetNumberOfSplinePoints();
			for (int32 i = 0; i < NumPoints; i++)
			{
				FVector Location = RiverSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
				FVector LocalLocation = WaterBodyRiver->GetActorTransform().InverseTransformPosition(Location);
				WaterSpline->AddSplinePoint(LocalLocation, ESplineCoordinateSpace::Local, false);
			}

			WaterSpline->UpdateSpline();
		}

		UE_LOG(LogTemp, Log, TEXT("RiverGenerator: Water body created successfully"));
	}
}

void ARiverGenerator::GenerateTributaries()
{
	// Clear old tributaries
	for (ARiverGenerator* Tributary : TributaryActors)
	{
		if (Tributary)
		{
			Tributary->Destroy();
		}
	}
	TributaryActors.Empty();

	UWorld* World = GetWorld();
	if (!World || Tributaries.Num() == 0)
	{
		return;
	}

	for (const FTributaryConfig& TribConfig : Tributaries)
	{
		// Find nearest point on main river spline
		FVector NearestPoint = FindNearestSplinePoint(TribConfig.SourceLocation);

		// Spawn tributary
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		ARiverGenerator* Tributary = World->SpawnActor<ARiverGenerator>(ARiverGenerator::StaticClass(), GetActorTransform(), SpawnParams);

		if (Tributary)
		{
			Tributary->SourceLocation = TribConfig.SourceLocation;
			Tributary->DestinationLocation = NearestPoint;
			Tributary->Width = Width * TribConfig.WidthMultiplier;
			Tributary->FlowSpeed = FlowSpeed * TribConfig.FlowSpeedMultiplier;
			Tributary->Depth = Depth * 0.7f;
			Tributary->TargetLandscape = TargetLandscape;
			Tributary->bModifyLandscape = bModifyLandscape;
			Tributary->bCreateWaterBody = bCreateWaterBody;

			Tributary->GenerateRiver();
			TributaryActors.Add(Tributary);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RiverGenerator: Generated %d tributaries"), TributaryActors.Num());
}

void ARiverGenerator::ClearRiver()
{
	if (RiverSpline)
	{
		RiverSpline->ClearSplinePoints();
	}

	if (WaterBodyRiver)
	{
		WaterBodyRiver->Destroy();
		WaterBodyRiver = nullptr;
	}

	for (ARiverGenerator* Tributary : TributaryActors)
	{
		if (Tributary)
		{
			Tributary->Destroy();
		}
	}
	TributaryActors.Empty();

	CachedPathPoints.Empty();
	bRiverGenerated = false;
}

float ARiverGenerator::GetTerrainHeight(const FVector& WorldPosition) const
{
	if (!TargetLandscape)
	{
		// Fallback: use simple line trace
		UWorld* World = GetWorld();
		if (World)
		{
			FHitResult HitResult;
			FVector Start = WorldPosition + FVector(0, 0, 10000.0f);
			FVector End = WorldPosition - FVector(0, 0, 10000.0f);

			if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
			{
				return HitResult.Location.Z;
			}
		}
		return WorldPosition.Z;
	}

	// Use landscape query
	ULandscapeInfo* LandscapeInfo = TargetLandscape->GetLandscapeInfo();
	if (LandscapeInfo)
	{
		FVector Location = WorldPosition;
		float Height = LandscapeInfo->GetCachedZ(Location.X, Location.Y);
		return Height;
	}

	return WorldPosition.Z;
}

FVector ARiverGenerator::GetTerrainNormal(const FVector& WorldPosition) const
{
	float SampleDist = 10.0f;

	float HeightCenter = GetTerrainHeight(WorldPosition);
	float HeightX = GetTerrainHeight(WorldPosition + FVector(SampleDist, 0, 0));
	float HeightY = GetTerrainHeight(WorldPosition + FVector(0, SampleDist, 0));

	FVector TangentX(SampleDist, 0, HeightX - HeightCenter);
	FVector TangentY(0, SampleDist, HeightY - HeightCenter);

	return FVector::CrossProduct(TangentX, TangentY).GetSafeNormal();
}

float ARiverGenerator::CalculateSlope(const FVector& Point1, const FVector& Point2) const
{
	float HorizontalDist = FVector::Dist2D(Point1, Point2);
	if (HorizontalDist < KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	float VerticalDist = Point2.Z - Point1.Z;
	return FMath::RadiansToDegrees(FMath::Atan2(VerticalDist, HorizontalDist));
}

TArray<FVector> ARiverGenerator::GetNeighbors(const FVector& Position) const
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

float ARiverGenerator::CalculateHeuristic(const FVector& From, const FVector& To) const
{
	float Distance = FVector::Dist2D(From, To);

	// Add elevation component
	float ElevationDiff = To.Z - From.Z;
	if (ElevationDiff > 0)
	{
		Distance += ElevationDiff * 2.0f; // Penalty for going uphill
	}

	return Distance;
}

bool ARiverGenerator::IsValidRiverPosition(const FVector& Position) const
{
	// Add custom validation logic here
	// For now, just return true
	return true;
}

TArray<FVector> ARiverGenerator::SmoothPath(const TArray<FVector>& Path, int32 WindowSize) const
{
	if (Path.Num() < WindowSize || WindowSize < 2)
	{
		return Path;
	}

	TArray<FVector> SmoothedPath;
	SmoothedPath.Add(Path[0]); // Keep first point

	int32 HalfWindow = WindowSize / 2;

	for (int32 i = HalfWindow; i < Path.Num() - HalfWindow; i++)
	{
		FVector Sum = FVector::ZeroVector;
		int32 Count = 0;

		for (int32 j = i - HalfWindow; j <= i + HalfWindow; j++)
		{
			if (j >= 0 && j < Path.Num())
			{
				Sum += Path[j];
				Count++;
			}
		}

		if (Count > 0)
		{
			FVector Smoothed = Sum / Count;
			Smoothed.Z = GetTerrainHeight(Smoothed); // Snap to terrain
			SmoothedPath.Add(Smoothed);
		}
	}

	SmoothedPath.Add(Path.Last()); // Keep last point
	return SmoothedPath;
}

FVector2D ARiverGenerator::CalculateCurlNoise(const FVector& Position) const
{
	float Epsilon = 1.0f;

	// Calculate derivatives
	float DxDy = NoiseDerivative(Position.X, Position.Y, false);
	float DyDx = NoiseDerivative(Position.X, Position.Y, true);

	// Curl = (dy/dx, -dx/dy)
	return FVector2D(DxDy, -DyDx);
}

float ARiverGenerator::NoiseDerivative(float X, float Y, bool bXDerivative) const
{
	float Epsilon = 10.0f;

	if (bXDerivative)
	{
		float N1 = GetNoise2D(X + Epsilon, Y);
		float N2 = GetNoise2D(X - Epsilon, Y);
		return (N1 - N2) / (2.0f * Epsilon);
	}
	else
	{
		float N1 = GetNoise2D(X, Y + Epsilon);
		float N2 = GetNoise2D(X, Y - Epsilon);
		return (N1 - N2) / (2.0f * Epsilon);
	}
}

float ARiverGenerator::GetNoise2D(float X, float Y) const
{
	FastNoiseLite Noise;
	Noise.SetSeed(NoiseSeed);
	Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise.SetFrequency(NoiseFrequency);

	return Noise.GetNoise(X, Y);
}

FVector ARiverGenerator::FindLowestPoint(const FVector& Center, float Radius) const
{
	FVector LowestPoint = Center;
	float LowestHeight = GetTerrainHeight(Center);

	int32 Samples = 32;
	for (int32 i = 0; i < Samples; i++)
	{
		float Angle = (i / (float)Samples) * 2.0f * PI;
		FVector SamplePos = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0) * Radius;
		float Height = GetTerrainHeight(SamplePos);

		if (Height < LowestHeight)
		{
			LowestHeight = Height;
			LowestPoint = SamplePos;
		}
	}

	LowestPoint.Z = LowestHeight;
	return LowestPoint;
}

void ARiverGenerator::ModifyLandscapeAtPoint(const FVector& Position, float Radius, float TargetHeight)
{
	// Note: Landscape editing at runtime requires proper landscape edit layer setup
	// This is a simplified version
	if (!TargetLandscape)
	{
		return;
	}

	ULandscapeInfo* LandscapeInfo = TargetLandscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		return;
	}

	// In a full implementation, you would use FLandscapeEditDataInterface
	// to modify heightmap data. This requires more complex setup.
	UE_LOG(LogTemp, Log, TEXT("RiverGenerator: Landscape modification at %s"), *Position.ToString());
}

FVector ARiverGenerator::FindNearestSplinePoint(const FVector& Position) const
{
	if (!RiverSpline || RiverSpline->GetNumberOfSplinePoints() < 2)
	{
		return Position;
	}

	float NearestDistance = FLT_MAX;
	FVector NearestPoint = Position;

	float SplineLength = RiverSpline->GetSplineLength();
	int32 Samples = FMath::CeilToInt(SplineLength / 100.0f);

	for (int32 i = 0; i < Samples; i++)
	{
		float Distance = (i / (float)Samples) * SplineLength;
		FVector Point = RiverSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		float Dist = FVector::Dist(Point, Position);
		if (Dist < NearestDistance)
		{
			NearestDistance = Dist;
			NearestPoint = Point;
		}
	}

	return NearestPoint;
}
