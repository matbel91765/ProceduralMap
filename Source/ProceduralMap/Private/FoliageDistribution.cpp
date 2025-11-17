// FoliageDistribution.cpp - Foliage Distribution Algorithms
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#include "FoliageDistribution.h"
#include "Kismet/KismetMathLibrary.h"

UFoliageDistribution::UFoliageDistribution()
{
}

int32 UFoliageDistribution::GenerateSpawnPoints(
	const FBox& Bounds,
	float Density,
	FRandomStream& RandomStream,
	TArray<FSpawnPoint>& OutSpawnPoints
)
{
	OutSpawnPoints.Empty();

	// Select algorithm based on type
	int32 PointsGenerated = 0;

	switch (AlgorithmType)
	{
	case EDistributionAlgorithm::PoissonDisk:
		PointsGenerated = GeneratePoissonDiskSampling(Bounds, Density, RandomStream, OutSpawnPoints);
		break;

	case EDistributionAlgorithm::NoiseClustering:
		PointsGenerated = GenerateNoiseClustering(Bounds, Density, RandomStream, OutSpawnPoints);
		break;

	case EDistributionAlgorithm::JitteredGrid:
		PointsGenerated = GenerateJitteredGrid(Bounds, Density, RandomStream, OutSpawnPoints);
		break;

	case EDistributionAlgorithm::Random:
	default:
		PointsGenerated = GenerateRandomDistribution(Bounds, Density, RandomStream, OutSpawnPoints);
		break;
	}

	// Apply clearance zones
	if (ClearanceZones.Num() > 0)
	{
		OutSpawnPoints.RemoveAll([this](const FSpawnPoint& Point)
		{
			return GetClearanceFactor(Point.Location) > 0.5f;
		});
	}

	// Apply age simulation if enabled
	if (bEnableAgeSimulation)
	{
		ApplyAgeSimulation(OutSpawnPoints, RandomStream);
	}

	// Apply species competition if enabled
	if (bEnableSpeciesCompetition)
	{
		ApplySpeciesCompetition(OutSpawnPoints, RandomStream);
	}

	return OutSpawnPoints.Num();
}

int32 UFoliageDistribution::GeneratePoissonDiskSampling(
	const FBox& Bounds,
	float Density,
	FRandomStream& RandomStream,
	TArray<FSpawnPoint>& OutSpawnPoints
)
{
	OutSpawnPoints.Empty();

	FVector Size = Bounds.GetSize();
	float Area = Size.X * Size.Y / 10000.0f; // Convert to square meters
	int32 TargetPoints = FMath::RoundToInt(Area * Density);

	if (TargetPoints <= 0)
	{
		return 0;
	}

	// Poisson disk sampling using Bridson's algorithm
	TArray<FVector> ActiveList;
	FSpatialGrid Grid(MinimumDistance * 2.0f);

	// Start with random point
	FVector InitialPoint = FVector(
		RandomStream.FRandRange(Bounds.Min.X, Bounds.Max.X),
		RandomStream.FRandRange(Bounds.Min.Y, Bounds.Max.Y),
		(Bounds.Min.Z + Bounds.Max.Z) * 0.5f
	);

	ActiveList.Add(InitialPoint);
	Grid.AddPoint(InitialPoint);
	OutSpawnPoints.Add(FSpawnPoint(InitialPoint, FVector::UpVector));

	// Process active list
	while (ActiveList.Num() > 0 && OutSpawnPoints.Num() < TargetPoints)
	{
		int32 ActiveIndex = RandomStream.RandRange(0, ActiveList.Num() - 1);
		FVector ActivePoint = ActiveList[ActiveIndex];

		bool bFoundValidPoint = false;

		for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
		{
			// Generate random point around active point
			float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
			float Distance = RandomStream.FRandRange(MinimumDistance, MinimumDistance * 2.0f);

			FVector NewPoint = ActivePoint + FVector(
				FMath::Cos(Angle) * Distance,
				FMath::Sin(Angle) * Distance,
				0.0f
			);

			// Check if point is in bounds
			if (!Bounds.IsInside(NewPoint))
			{
				continue;
			}

			// Check if too close to existing points
			if (Grid.HasNearbyPoint(NewPoint, MinimumDistance))
			{
				continue;
			}

			// Valid point found
			ActiveList.Add(NewPoint);
			Grid.AddPoint(NewPoint);
			OutSpawnPoints.Add(FSpawnPoint(NewPoint, FVector::UpVector));
			bFoundValidPoint = true;
			break;
		}

		if (!bFoundValidPoint)
		{
			// Remove from active list
			ActiveList.RemoveAtSwap(ActiveIndex);
		}
	}

	return OutSpawnPoints.Num();
}

int32 UFoliageDistribution::GenerateNoiseClustering(
	const FBox& Bounds,
	float Density,
	FRandomStream& RandomStream,
	TArray<FSpawnPoint>& OutSpawnPoints
)
{
	OutSpawnPoints.Empty();

	FVector Size = Bounds.GetSize();
	float Area = Size.X * Size.Y / 10000.0f; // Square meters
	int32 TargetPoints = FMath::RoundToInt(Area * Density);

	FSpatialGrid Grid(MinimumDistance * 2.0f);

	for (int32 i = 0; i < TargetPoints * 2; ++i) // Generate more attempts to account for clustering
	{
		FVector RandomPoint = FVector(
			RandomStream.FRandRange(Bounds.Min.X, Bounds.Max.X),
			RandomStream.FRandRange(Bounds.Min.Y, Bounds.Max.Y),
			(Bounds.Min.Z + Bounds.Max.Z) * 0.5f
		);

		// Check noise value
		float NoiseValue = GetNoiseValue(RandomPoint);
		if (NoiseValue < NoiseThreshold)
		{
			continue;
		}

		// Check minimum distance
		if (Grid.HasNearbyPoint(RandomPoint, MinimumDistance))
		{
			continue;
		}

		// Add point with density based on noise
		float LocalDensity = (NoiseValue - NoiseThreshold) / (1.0f - NoiseThreshold);
		if (RandomStream.FRand() < LocalDensity * ClusterDensityMultiplier)
		{
			Grid.AddPoint(RandomPoint);
			OutSpawnPoints.Add(FSpawnPoint(RandomPoint, FVector::UpVector));

			if (OutSpawnPoints.Num() >= TargetPoints)
			{
				break;
			}
		}
	}

	return OutSpawnPoints.Num();
}

int32 UFoliageDistribution::GenerateJitteredGrid(
	const FBox& Bounds,
	float Density,
	FRandomStream& RandomStream,
	TArray<FSpawnPoint>& OutSpawnPoints
)
{
	OutSpawnPoints.Empty();

	FVector Size = Bounds.GetSize();

	// Calculate grid spacing from density
	float PointsPerMeter = FMath::Sqrt(Density);
	float GridSpacing = 100.0f / PointsPerMeter; // Convert to cm

	if (GridSpacing < MinimumDistance)
	{
		GridSpacing = MinimumDistance;
	}

	int32 GridX = FMath::CeilToInt(Size.X / GridSpacing);
	int32 GridY = FMath::CeilToInt(Size.Y / GridSpacing);

	float JitterAmount = GridSpacing * 0.4f; // 40% jitter

	for (int32 x = 0; x < GridX; ++x)
	{
		for (int32 y = 0; y < GridY; ++y)
		{
			FVector GridPoint = Bounds.Min + FVector(
				x * GridSpacing + GridSpacing * 0.5f,
				y * GridSpacing + GridSpacing * 0.5f,
				(Bounds.Max.Z - Bounds.Min.Z) * 0.5f
			);

			// Add jitter
			FVector JitteredPoint = GridPoint + FVector(
				RandomStream.FRandRange(-JitterAmount, JitterAmount),
				RandomStream.FRandRange(-JitterAmount, JitterAmount),
				0.0f
			);

			// Check bounds
			if (Bounds.IsInside(JitteredPoint))
			{
				OutSpawnPoints.Add(FSpawnPoint(JitteredPoint, FVector::UpVector));
			}
		}
	}

	return OutSpawnPoints.Num();
}

int32 UFoliageDistribution::GenerateRandomDistribution(
	const FBox& Bounds,
	float Density,
	FRandomStream& RandomStream,
	TArray<FSpawnPoint>& OutSpawnPoints
)
{
	OutSpawnPoints.Empty();

	FVector Size = Bounds.GetSize();
	float Area = Size.X * Size.Y / 10000.0f; // Square meters
	int32 NumPoints = FMath::RoundToInt(Area * Density);

	FSpatialGrid Grid(MinimumDistance * 2.0f);

	int32 Attempts = 0;
	int32 MaxTotalAttempts = NumPoints * MaxAttempts;

	while (OutSpawnPoints.Num() < NumPoints && Attempts < MaxTotalAttempts)
	{
		Attempts++;

		FVector RandomPoint = FVector(
			RandomStream.FRandRange(Bounds.Min.X, Bounds.Max.X),
			RandomStream.FRandRange(Bounds.Min.Y, Bounds.Max.Y),
			(Bounds.Min.Z + Bounds.Max.Z) * 0.5f
		);

		// Check minimum distance if enabled
		if (MinimumDistance > 0.0f && Grid.HasNearbyPoint(RandomPoint, MinimumDistance))
		{
			continue;
		}

		Grid.AddPoint(RandomPoint);
		OutSpawnPoints.Add(FSpawnPoint(RandomPoint, FVector::UpVector));
	}

	return OutSpawnPoints.Num();
}

bool UFoliageDistribution::IsTooClose(const FVector& Point, const TArray<FSpawnPoint>& ExistingPoints, float MinDistance) const
{
	float MinDistSq = MinDistance * MinDistance;

	for (const FSpawnPoint& ExistingPoint : ExistingPoints)
	{
		if (FVector::DistSquared(Point, ExistingPoint.Location) < MinDistSq)
		{
			return true;
		}
	}

	return false;
}

void UFoliageDistribution::AddClearanceZone(const FVector& Center, float Radius, float Falloff)
{
	ClearanceZones.Add(FClearanceZone(Center, Radius, Falloff));
}

void UFoliageDistribution::ClearClearanceZones()
{
	ClearanceZones.Empty();
}

float UFoliageDistribution::GetClearanceFactor(const FVector& Point) const
{
	float MaxFactor = 0.0f;

	for (const FClearanceZone& Zone : ClearanceZones)
	{
		float Factor = Zone.GetClearanceFactor(Point);
		if (Factor > MaxFactor)
		{
			MaxFactor = Factor;
		}
	}

	return MaxFactor;
}

void UFoliageDistribution::ApplyAgeSimulation(TArray<FSpawnPoint>& SpawnPoints, FRandomStream& RandomStream)
{
	for (FSpawnPoint& Point : SpawnPoints)
	{
		// Generate random age
		float Age = RandomStream.FRandRange(MinAge, 1.0f);

		// Apply variation
		if (AgeVariation > 0.0f)
		{
			float Variation = RandomStream.FRandRange(-AgeVariation, AgeVariation);
			Age = FMath::Clamp(Age + Variation, MinAge, 1.0f);
		}

		Point.Age = Age;
		Point.Scale = Age; // Scale correlates with age
	}
}

void UFoliageDistribution::ApplySpeciesCompetition(TArray<FSpawnPoint>& SpawnPoints, FRandomStream& RandomStream)
{
	if (SpawnPoints.Num() <= 1)
	{
		return;
	}

	// Build spatial grid for fast lookup
	FSpatialGrid Grid(MinimumDistance * CompetitionRadiusMultiplier);
	for (const FSpawnPoint& Point : SpawnPoints)
	{
		Grid.AddPoint(Point.Location);
	}

	// Mark points for removal based on competition
	TArray<bool> ToRemove;
	ToRemove.SetNumZeroed(SpawnPoints.Num());

	float CompetitionRadius = MinimumDistance * CompetitionRadiusMultiplier;

	for (int32 i = 0; i < SpawnPoints.Num(); ++i)
	{
		if (ToRemove[i])
		{
			continue;
		}

		const FSpawnPoint& Point = SpawnPoints[i];

		// Count nearby competitors
		int32 NearbyCount = 0;
		for (int32 j = 0; j < SpawnPoints.Num(); ++j)
		{
			if (i == j || ToRemove[j])
			{
				continue;
			}

			float Distance = FVector::Dist(Point.Location, SpawnPoints[j].Location);
			if (Distance < CompetitionRadius)
			{
				NearbyCount++;

				// Random competition - weaker/younger plants have higher removal chance
				if (Point.Age < SpawnPoints[j].Age && RandomStream.FRand() < 0.5f)
				{
					ToRemove[i] = true;
					break;
				}
			}
		}
	}

	// Remove competed points
	for (int32 i = SpawnPoints.Num() - 1; i >= 0; --i)
	{
		if (ToRemove[i])
		{
			SpawnPoints.RemoveAtSwap(i);
		}
	}
}

float UFoliageDistribution::GetNoiseValue(const FVector& Location) const
{
	// Simple Perlin-like noise using sine waves
	// For production, you would use FastNoiseLite or UE's noise functions

	float X = Location.X * NoiseScale;
	float Y = Location.Y * NoiseScale;

	float Noise = 0.0f;

	// Octave 1
	Noise += FMath::Sin(X * 1.0f) * FMath::Sin(Y * 1.0f) * 0.5f;

	// Octave 2
	Noise += FMath::Sin(X * 2.0f + 1.5f) * FMath::Sin(Y * 2.0f + 1.5f) * 0.25f;

	// Octave 3
	Noise += FMath::Sin(X * 4.0f + 3.0f) * FMath::Sin(Y * 4.0f + 3.0f) * 0.125f;

	// Normalize to 0-1
	Noise = (Noise + 0.875f) / 1.75f;

	return FMath::Clamp(Noise, 0.0f, 1.0f);
}
