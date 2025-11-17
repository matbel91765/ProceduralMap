// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "LakeGenerator.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "WaterBodyLakeActor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "FastNoiseLite.h"
#include "SplineUtilities.h"

ALakeGenerator::ALakeGenerator()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	LakeSpline = CreateDefaultSubobject<USplineComponent>(TEXT("LakeSpline"));
	LakeSpline->SetupAttachment(RootComponent);
	LakeSpline->SetClosedLoop(true);

	// Default lake parameters
	LakeShape = ELakeShape::Organic;
	LakeRadius = 5000.0f;
	LakeDepth = 200.0f;
	WaterLevel = 0.0f;
	BoundaryPoints = 32;
	Irregularity = 0.3f;

	// Generation settings
	LakeCenter = FVector::ZeroVector;
	bAutoDetectLowAreas = false;
	SearchRadius = 10000.0f;
	MinimumAreaSize = 2000.0f;
	MaxLakesCount = 5;

	// Noise settings
	bApplyNoise = true;
	NoiseFrequency = 0.02f;
	NoiseAmplitude = 500.0f;
	NoiseOctaves = 3;
	NoiseSeed = 54321;

	// Terrain integration
	TargetLandscape = nullptr;
	bCarveTerrainForLake = false;
	ShorelineSmoothness = 500.0f;
	BeachWidth = 200.0f;

	// Water system
	bCreateWaterBody = true;
	WaterBodyLake = nullptr;

	// Islands
	bAutoGenerateIslands = false;
	IslandCount = 2;
	IslandMinDistanceFromShore = 1000.0f;

	bLakeGenerated = false;
}

void ALakeGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void ALakeGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ALakeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALakeGenerator::GenerateLake()
{
	ClearLake();

	// Auto-detect low areas if enabled
	if (bAutoDetectLowAreas)
	{
		TArray<FLowAreaData> LowAreas = DetectLowAreas(GetActorLocation(), SearchRadius);

		if (LowAreas.Num() > 0)
		{
			// Use the first detected low area
			LakeCenter = LowAreas[0].Center;
			LakeRadius = FMath::Clamp(LowAreas[0].Radius, MinimumAreaSize, LakeRadius);
			WaterLevel = LowAreas[0].AverageHeight;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LakeGenerator: No suitable low areas found"));
			return;
		}
	}

	// Generate lake based on shape type
	switch (LakeShape)
	{
	case ELakeShape::Circular:
		GenerateCircularLake();
		break;

	case ELakeShape::Organic:
		GenerateOrganicLake();
		break;

	case ELakeShape::Irregular:
		GenerateIrregularLake();
		break;

	case ELakeShape::Custom:
		GenerateCustomLake();
		break;
	}

	// Apply noise if enabled
	if (bApplyNoise)
	{
		ApplyNoiseToLakeBoundary();
	}

	// Smooth boundary
	SmoothLakeBoundary(2);

	// Carve terrain
	if (bCarveTerrainForLake && TargetLandscape)
	{
		CarveTerrainForLake();
	}

	// Create water body
	if (bCreateWaterBody)
	{
		CreateWaterBody();
	}

	// Generate islands
	if (bAutoGenerateIslands)
	{
		GenerateIslands();
	}

	bLakeGenerated = true;
	UE_LOG(LogTemp, Log, TEXT("LakeGenerator: Lake generated successfully at %s with radius %f"),
		*LakeCenter.ToString(), LakeRadius);
}

TArray<FLowAreaData> ALakeGenerator::DetectLowAreas(const FVector& ScanCenter, float ScanRadius)
{
	TArray<FLowAreaData> LowAreas;

	if (!TargetLandscape)
	{
		UE_LOG(LogTemp, Warning, TEXT("LakeGenerator: No landscape reference for low area detection"));
		return LowAreas;
	}

	// Sample terrain in a grid pattern
	int32 GridSize = 20;
	float CellSize = (ScanRadius * 2.0f) / GridSize;

	TArray<FVector> LowPoints;

	// Calculate average height first
	float TotalHeight = 0.0f;
	int32 SampleCount = 0;

	for (int32 X = 0; X < GridSize; X++)
	{
		for (int32 Y = 0; Y < GridSize; Y++)
		{
			FVector SamplePos = ScanCenter;
			SamplePos.X += (X - GridSize / 2) * CellSize;
			SamplePos.Y += (Y - GridSize / 2) * CellSize;

			float Height = GetTerrainHeight(SamplePos);
			TotalHeight += Height;
			SampleCount++;
		}
	}

	float AverageHeight = TotalHeight / FMath::Max(1, SampleCount);

	// Find points below average
	for (int32 X = 0; X < GridSize; X++)
	{
		for (int32 Y = 0; Y < GridSize; Y++)
		{
			FVector SamplePos = ScanCenter;
			SamplePos.X += (X - GridSize / 2) * CellSize;
			SamplePos.Y += (Y - GridSize / 2) * CellSize;

			float Height = GetTerrainHeight(SamplePos);

			if (Height < AverageHeight - 50.0f) // 50cm threshold
			{
				SamplePos.Z = Height;
				LowPoints.Add(SamplePos);
			}
		}
	}

	// Cluster low points into areas
	if (LowPoints.Num() > 0)
	{
		// Simple clustering: find local minima
		TArray<bool> Processed;
		Processed.Init(false, LowPoints.Num());

		for (int32 i = 0; i < LowPoints.Num(); i++)
		{
			if (Processed[i])
			{
				continue;
			}

			FLowAreaData Area;
			Area.Center = FindLocalMinimum(LowPoints[i], MinimumAreaSize);
			Area.AverageHeight = GetTerrainHeight(Area.Center);
			Area.Radius = MinimumAreaSize;
			Area.PointCount = 1;

			// Find nearby low points
			for (int32 j = 0; j < LowPoints.Num(); j++)
			{
				if (Processed[j])
				{
					continue;
				}

				float Distance = FVector::Dist2D(Area.Center, LowPoints[j]);
				if (Distance < MinimumAreaSize * 2.0f)
				{
					Processed[j] = true;
					Area.PointCount++;
				}
			}

			if (Area.PointCount >= 5) // Minimum cluster size
			{
				LowAreas.Add(Area);

				if (LowAreas.Num() >= MaxLakesCount)
				{
					break;
				}
			}

			Processed[i] = true;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LakeGenerator: Detected %d low areas"), LowAreas.Num());
	return LowAreas;
}

void ALakeGenerator::GenerateCircularLake()
{
	LakeSpline->ClearSplinePoints();

	TArray<FVector> BoundaryPoints = GenerateCircularPattern(LakeCenter, LakeRadius, BoundaryPoints);

	for (const FVector& Point : BoundaryPoints)
	{
		FVector LocalPos = GetActorTransform().InverseTransformPosition(Point);
		LakeSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
	}

	LakeSpline->UpdateSpline();
	CachedBoundaryPoints = BoundaryPoints;
}

void ALakeGenerator::GenerateOrganicLake()
{
	LakeSpline->ClearSplinePoints();

	TArray<FVector> BoundaryPoints = GenerateCircularPattern(LakeCenter, LakeRadius, BoundaryPoints);

	// Apply radial noise for organic shape
	ApplyRadialNoise(BoundaryPoints, NoiseFrequency * 2.0f, LakeRadius * Irregularity);

	for (const FVector& Point : BoundaryPoints)
	{
		FVector LocalPos = GetActorTransform().InverseTransformPosition(Point);
		LakeSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
	}

	LakeSpline->UpdateSpline();
	CachedBoundaryPoints = BoundaryPoints;
}

void ALakeGenerator::GenerateIrregularLake()
{
	LakeSpline->ClearSplinePoints();

	TArray<FVector> BoundaryPoints = GenerateCircularPattern(LakeCenter, LakeRadius, BoundaryPoints);

	// Apply multiple layers of noise for irregular shape
	ApplyRadialNoise(BoundaryPoints, NoiseFrequency * 0.5f, LakeRadius * Irregularity * 1.5f);
	ApplyRadialNoise(BoundaryPoints, NoiseFrequency * 2.0f, LakeRadius * Irregularity * 0.5f);
	ApplyRadialNoise(BoundaryPoints, NoiseFrequency * 5.0f, LakeRadius * Irregularity * 0.2f);

	for (const FVector& Point : BoundaryPoints)
	{
		FVector LocalPos = GetActorTransform().InverseTransformPosition(Point);
		LakeSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
	}

	LakeSpline->UpdateSpline();
	CachedBoundaryPoints = BoundaryPoints;
}

void ALakeGenerator::GenerateCustomLake()
{
	if (CustomShapePoints.Num() < 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("LakeGenerator: Need at least 3 custom points for lake shape"));
		GenerateCircularLake(); // Fallback
		return;
	}

	LakeSpline->ClearSplinePoints();

	for (const FVector& Point : CustomShapePoints)
	{
		FVector LocalPos = GetActorTransform().InverseTransformPosition(Point);
		LakeSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
	}

	LakeSpline->UpdateSpline();
	CachedBoundaryPoints = CustomShapePoints;
}

void ALakeGenerator::ApplyNoiseToLakeBoundary()
{
	if (!LakeSpline)
	{
		return;
	}

	int32 NumPoints = LakeSpline->GetNumberOfSplinePoints();

	FastNoiseLite Noise;
	Noise.SetSeed(NoiseSeed);
	Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise.SetFrequency(NoiseFrequency);
	Noise.SetFractalOctaves(NoiseOctaves);

	for (int32 i = 0; i < NumPoints; i++)
	{
		FVector WorldPos = LakeSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

		// Calculate noise displacement
		float NoiseValue = Noise.GetNoise(WorldPos.X, WorldPos.Y);
		float Displacement = NoiseValue * NoiseAmplitude;

		// Get direction from center
		FVector ToPoint = (WorldPos - LakeCenter).GetSafeNormal2D();

		// Apply displacement radially
		FVector NewPos = WorldPos + ToPoint * Displacement;
		NewPos.Z = GetTerrainHeight(NewPos) + WaterLevel;

		// Update spline point
		FVector LocalPos = GetActorTransform().InverseTransformPosition(NewPos);
		LakeSpline->SetLocationAtSplinePoint(i, LocalPos, ESplineCoordinateSpace::Local, false);
	}

	LakeSpline->UpdateSpline();
}

void ALakeGenerator::CarveTerrainForLake()
{
	if (!TargetLandscape || !LakeSpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("LakeGenerator: Cannot carve terrain - missing references"));
		return;
	}

	// Sample lake boundary
	float SplineLength = LakeSpline->GetSplineLength();
	int32 Samples = FMath::CeilToInt(SplineLength / 100.0f);

	for (int32 i = 0; i < Samples; i++)
	{
		float Distance = (i / (float)Samples) * SplineLength;
		FVector BoundaryPoint = LakeSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		float TargetHeight = WaterLevel - LakeDepth;
		ModifyTerrainAtPoint(BoundaryPoint, ShorelineSmoothness, TargetHeight);
	}

	// Also carve the center
	ModifyTerrainAtPoint(LakeCenter, LakeRadius * 0.8f, WaterLevel - LakeDepth);

	UE_LOG(LogTemp, Log, TEXT("LakeGenerator: Terrain carving complete"));
}

void ALakeGenerator::CreateWaterBody()
{
	if (!WaterBodyClass || !LakeSpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("LakeGenerator: Cannot create water body - missing water body class"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Destroy old water body if exists
	if (WaterBodyLake)
	{
		WaterBodyLake->Destroy();
	}

	// Spawn new water body
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	WaterBodyLake = World->SpawnActor<AWaterBodyLake>(WaterBodyClass, GetActorTransform(), SpawnParams);

	if (WaterBodyLake)
	{
		// Copy spline data to water body
		USplineComponent* WaterSpline = WaterBodyLake->GetWaterSpline();
		if (WaterSpline)
		{
			WaterSpline->ClearSplinePoints();

			int32 NumPoints = LakeSpline->GetNumberOfSplinePoints();
			for (int32 i = 0; i < NumPoints; i++)
			{
				FVector Location = LakeSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
				Location.Z = WaterLevel; // Set to water level
				FVector LocalLocation = WaterBodyLake->GetActorTransform().InverseTransformPosition(Location);
				WaterSpline->AddSplinePoint(LocalLocation, ESplineCoordinateSpace::Local, false);
			}

			WaterSpline->SetClosedLoop(true);
			WaterSpline->UpdateSpline();
		}

		UE_LOG(LogTemp, Log, TEXT("LakeGenerator: Water body created successfully"));
	}
}

void ALakeGenerator::GenerateIslands()
{
	if (IslandCount <= 0 || !LakeSpline)
	{
		return;
	}

	Islands.Empty();

	for (int32 i = 0; i < IslandCount; i++)
	{
		FIslandConfig Island;
		Island.Location = GenerateIslandPosition();
		Island.Radius = FMath::RandRange(200.0f, 500.0f);
		Island.Height = FMath::RandRange(50.0f, 150.0f);

		Islands.Add(Island);
		CreateIsland(Island);
	}

	UE_LOG(LogTemp, Log, TEXT("LakeGenerator: Generated %d islands"), Islands.Num());
}

void ALakeGenerator::ClearLake()
{
	if (LakeSpline)
	{
		LakeSpline->ClearSplinePoints();
	}

	if (WaterBodyLake)
	{
		WaterBodyLake->Destroy();
		WaterBodyLake = nullptr;
	}

	CachedBoundaryPoints.Empty();
	Islands.Empty();
	bLakeGenerated = false;
}

float ALakeGenerator::GetTerrainHeight(const FVector& WorldPosition) const
{
	if (!TargetLandscape)
	{
		// Fallback: use line trace
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
		float Height = LandscapeInfo->GetCachedZ(WorldPosition.X, WorldPosition.Y);
		return Height;
	}

	return WorldPosition.Z;
}

bool ALakeGenerator::IsPointInsideLake(const FVector& WorldPosition) const
{
	if (!LakeSpline || CachedBoundaryPoints.Num() < 3)
	{
		return false;
	}

	// Convert to 2D polygon test
	TArray<FVector2D> Polygon;
	for (const FVector& Point : CachedBoundaryPoints)
	{
		Polygon.Add(FVector2D(Point.X, Point.Y));
	}

	FVector2D TestPoint(WorldPosition.X, WorldPosition.Y);
	return IsPointInPolygon(TestPoint, Polygon);
}

float ALakeGenerator::GetDistanceToShore(const FVector& WorldPosition) const
{
	if (!LakeSpline)
	{
		return 0.0f;
	}

	float NearestDistance;
	USplineUtilities::FindNearestPointOnSpline(LakeSpline, WorldPosition, NearestDistance);

	return NearestDistance;
}

float ALakeGenerator::CalculateLakeSurfaceArea() const
{
	if (CachedBoundaryPoints.Num() < 3)
	{
		return 0.0f;
	}

	// Simple polygon area calculation (Shoelace formula)
	float Area = 0.0f;

	for (int32 i = 0; i < CachedBoundaryPoints.Num(); i++)
	{
		int32 j = (i + 1) % CachedBoundaryPoints.Num();

		Area += CachedBoundaryPoints[i].X * CachedBoundaryPoints[j].Y;
		Area -= CachedBoundaryPoints[j].X * CachedBoundaryPoints[i].Y;
	}

	Area = FMath::Abs(Area) * 0.5f;
	return Area;
}

float ALakeGenerator::CalculateLakeVolume() const
{
	float SurfaceArea = CalculateLakeSurfaceArea();
	return SurfaceArea * LakeDepth;
}

// ========== Private Helper Functions ==========

TArray<FVector> ALakeGenerator::GenerateCircularPattern(const FVector& Center, float Radius, int32 NumPoints)
{
	TArray<FVector> Points;

	for (int32 i = 0; i < NumPoints; i++)
	{
		float Angle = (i / (float)NumPoints) * 2.0f * PI;

		FVector Point;
		Point.X = Center.X + FMath::Cos(Angle) * Radius;
		Point.Y = Center.Y + FMath::Sin(Angle) * Radius;
		Point.Z = GetTerrainHeight(Point) + WaterLevel;

		Points.Add(Point);
	}

	return Points;
}

void ALakeGenerator::ApplyRadialNoise(TArray<FVector>& Points, float Frequency, float Amplitude)
{
	FastNoiseLite Noise;
	Noise.SetSeed(NoiseSeed);
	Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise.SetFrequency(Frequency);

	for (int32 i = 0; i < Points.Num(); i++)
	{
		float Angle = (i / (float)Points.Num()) * 2.0f * PI;

		float NoiseValue = Noise.GetNoise(FMath::Cos(Angle) * 1000.0f, FMath::Sin(Angle) * 1000.0f);
		float RadiusOffset = NoiseValue * Amplitude;

		FVector ToPoint = (Points[i] - LakeCenter).GetSafeNormal2D();
		Points[i] += ToPoint * RadiusOffset;
		Points[i].Z = GetTerrainHeight(Points[i]) + WaterLevel;
	}
}

float ALakeGenerator::GetRadialNoise(float Angle, int32 Seed) const
{
	FastNoiseLite Noise;
	Noise.SetSeed(Seed);
	Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise.SetFrequency(NoiseFrequency);

	return Noise.GetNoise(FMath::Cos(Angle), FMath::Sin(Angle));
}

void ALakeGenerator::SmoothLakeBoundary(int32 Iterations)
{
	if (!LakeSpline)
	{
		return;
	}

	USplineUtilities::SmoothSpline(LakeSpline, 0.5f, Iterations);
}

bool ALakeGenerator::IsSuitableForLake(const FVector& Center, float Radius) const
{
	// Check if area is relatively flat and low
	float AverageHeight = CalculateAverageHeight(Center, Radius, 20);
	float CenterHeight = GetTerrainHeight(Center);

	// Should be a depression (center lower than average)
	return CenterHeight < AverageHeight - 20.0f;
}

float ALakeGenerator::CalculateAverageHeight(const FVector& Center, float Radius, int32 Samples) const
{
	float TotalHeight = 0.0f;

	for (int32 i = 0; i < Samples; i++)
	{
		float Angle = (i / (float)Samples) * 2.0f * PI;
		FVector SamplePos = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0);

		TotalHeight += GetTerrainHeight(SamplePos);
	}

	return TotalHeight / Samples;
}

FVector ALakeGenerator::FindLocalMinimum(const FVector& StartPosition, float SearchRadius) const
{
	FVector CurrentPos = StartPosition;
	float CurrentHeight = GetTerrainHeight(CurrentPos);

	int32 Directions = 8;
	bool bFoundLower = true;

	while (bFoundLower)
	{
		bFoundLower = false;

		for (int32 i = 0; i < Directions; i++)
		{
			float Angle = (i / (float)Directions) * 2.0f * PI;
			FVector TestPos = CurrentPos + FVector(FMath::Cos(Angle) * 100.0f, FMath::Sin(Angle) * 100.0f, 0);

			float TestHeight = GetTerrainHeight(TestPos);

			if (TestHeight < CurrentHeight)
			{
				CurrentPos = TestPos;
				CurrentHeight = TestHeight;
				bFoundLower = true;
				break;
			}
		}
	}

	CurrentPos.Z = CurrentHeight;
	return CurrentPos;
}

void ALakeGenerator::ModifyTerrainAtPoint(const FVector& Position, float Radius, float TargetHeight)
{
	// Note: Landscape editing at runtime requires proper landscape edit layer setup
	if (!TargetLandscape)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("LakeGenerator: Terrain modification at %s"), *Position.ToString());
}

FVector ALakeGenerator::GenerateIslandPosition() const
{
	if (!LakeSpline)
	{
		return LakeCenter;
	}

	// Generate random position within lake, maintaining distance from shore
	int32 MaxAttempts = 20;
	for (int32 i = 0; i < MaxAttempts; i++)
	{
		float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		float Distance = FMath::FRandRange(IslandMinDistanceFromShore, LakeRadius - IslandMinDistanceFromShore);

		FVector Position = LakeCenter;
		Position.X += FMath::Cos(Angle) * Distance;
		Position.Y += FMath::Sin(Angle) * Distance;

		if (IsPointInsideLake(Position))
		{
			Position.Z = WaterLevel;
			return Position;
		}
	}

	return LakeCenter;
}

void ALakeGenerator::CreateIsland(const FIslandConfig& Config)
{
	// In a full implementation, this would create terrain or mesh for the island
	UE_LOG(LogTemp, Log, TEXT("LakeGenerator: Creating island at %s with radius %f"),
		*Config.Location.ToString(), Config.Radius);
}

bool ALakeGenerator::IsPointInPolygon(const FVector2D& Point, const TArray<FVector2D>& Polygon) const
{
	if (Polygon.Num() < 3)
	{
		return false;
	}

	bool bInside = false;
	int32 j = Polygon.Num() - 1;

	for (int32 i = 0; i < Polygon.Num(); i++)
	{
		if (((Polygon[i].Y > Point.Y) != (Polygon[j].Y > Point.Y)) &&
			(Point.X < (Polygon[j].X - Polygon[i].X) * (Point.Y - Polygon[i].Y) / (Polygon[j].Y - Polygon[i].Y) + Polygon[i].X))
		{
			bInside = !bInside;
		}
		j = i;
	}

	return bInside;
}
