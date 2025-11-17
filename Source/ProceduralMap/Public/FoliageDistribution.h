// FoliageDistribution.h - Foliage Distribution Algorithms
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FoliageDistribution.generated.h"

/**
 * Distribution algorithm type
 */
UENUM(BlueprintType)
enum class EDistributionAlgorithm : uint8
{
	/** Uniform random distribution */
	Random UMETA(DisplayName = "Random"),

	/** Poisson disk sampling for natural spacing */
	PoissonDisk UMETA(DisplayName = "Poisson Disk"),

	/** Noise-based clustering */
	NoiseClustering UMETA(DisplayName = "Noise Clustering"),

	/** Grid-based with jitter */
	JitteredGrid UMETA(DisplayName = "Jittered Grid")
};

/**
 * Spawn point structure
 */
USTRUCT(BlueprintType)
struct FSpawnPoint
{
	GENERATED_BODY()

	/** World location */
	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	FVector Location = FVector::ZeroVector;

	/** Surface normal */
	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	FVector Normal = FVector::UpVector;

	/** Foliage type index */
	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	int32 FoliageTypeIndex = 0;

	/** Scale multiplier */
	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	float Scale = 1.0f;

	/** Age factor (0-1, affects size) */
	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	float Age = 1.0f;

	FSpawnPoint() {}

	FSpawnPoint(const FVector& InLocation, const FVector& InNormal, int32 InTypeIndex = 0)
		: Location(InLocation), Normal(InNormal), FoliageTypeIndex(InTypeIndex), Scale(1.0f), Age(1.0f)
	{
	}
};

/**
 * Clearance zone to avoid spawning
 */
USTRUCT(BlueprintType)
struct FClearanceZone
{
	GENERATED_BODY()

	/** Center location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clearance")
	FVector Center = FVector::ZeroVector;

	/** Clearance radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clearance")
	float Radius = 500.0f;

	/** Falloff distance (soft edge) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clearance")
	float Falloff = 100.0f;

	FClearanceZone() {}

	FClearanceZone(const FVector& InCenter, float InRadius, float InFalloff = 100.0f)
		: Center(InCenter), Radius(InRadius), Falloff(InFalloff)
	{
	}

	/** Check if point is in clearance zone (returns 0-1, 0=outside, 1=full clear) */
	float GetClearanceFactor(const FVector& Point) const
	{
		float Distance = FVector::Dist(Center, Point);
		if (Distance >= Radius + Falloff)
		{
			return 0.0f;
		}
		if (Distance <= Radius)
		{
			return 1.0f;
		}
		// Smooth falloff
		float Alpha = (Distance - Radius) / Falloff;
		return 1.0f - Alpha;
	}
};

/**
 * Base class for foliage distribution algorithms
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, ClassGroup = "ProceduralMap")
class PROCEDURALMAP_API UFoliageDistribution : public UObject
{
	GENERATED_BODY()

public:
	UFoliageDistribution();

	// ============================================
	// Configuration
	// ============================================

	/** Distribution algorithm type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
	EDistributionAlgorithm AlgorithmType = EDistributionAlgorithm::PoissonDisk;

	/** Minimum distance between instances (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "10.0"))
	float MinimumDistance = 200.0f;

	/** Maximum attempts per spawn point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "1"))
	int32 MaxAttempts = 30;

	/** Enable multi-species competition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
	bool bEnableSpeciesCompetition = false;

	/** Competition radius multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species", meta = (ClampMin = "1.0"))
	float CompetitionRadiusMultiplier = 2.0f;

	/** Enable age simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Age")
	bool bEnableAgeSimulation = false;

	/** Age variation (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Age", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AgeVariation = 0.5f;

	/** Minimum age (affects size) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Age", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinAge = 0.3f;

	/** Clearance zones to avoid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clearance")
	TArray<FClearanceZone> ClearanceZones;

	// ============================================
	// Noise Clustering Settings
	// ============================================

	/** Noise scale for clustering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.0001"))
	float NoiseScale = 0.01f;

	/** Noise threshold (0-1, higher = fewer clusters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NoiseThreshold = 0.3f;

	/** Cluster density multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.1"))
	float ClusterDensityMultiplier = 2.0f;

	// ============================================
	// Methods
	// ============================================

	/**
	 * Generate spawn points using the selected algorithm
	 * @param Bounds - World space bounds to spawn in
	 * @param Density - Base density (points per square meter)
	 * @param RandomStream - Random stream for reproducibility
	 * @param OutSpawnPoints - Generated spawn points
	 * @return Number of spawn points generated
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	virtual int32 GenerateSpawnPoints(
		const FBox& Bounds,
		float Density,
		FRandomStream& RandomStream,
		TArray<FSpawnPoint>& OutSpawnPoints
	);

	/**
	 * Poisson disk sampling algorithm
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	int32 GeneratePoissonDiskSampling(
		const FBox& Bounds,
		float Density,
		FRandomStream& RandomStream,
		TArray<FSpawnPoint>& OutSpawnPoints
	);

	/**
	 * Noise-based clustering algorithm
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	int32 GenerateNoiseClustering(
		const FBox& Bounds,
		float Density,
		FRandomStream& RandomStream,
		TArray<FSpawnPoint>& OutSpawnPoints
	);

	/**
	 * Jittered grid algorithm
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	int32 GenerateJitteredGrid(
		const FBox& Bounds,
		float Density,
		FRandomStream& RandomStream,
		TArray<FSpawnPoint>& OutSpawnPoints
	);

	/**
	 * Random distribution algorithm
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	int32 GenerateRandomDistribution(
		const FBox& Bounds,
		float Density,
		FRandomStream& RandomStream,
		TArray<FSpawnPoint>& OutSpawnPoints
	);

	/**
	 * Check if point is too close to existing points
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	bool IsTooClose(const FVector& Point, const TArray<FSpawnPoint>& ExistingPoints, float MinDistance) const;

	/**
	 * Add clearance zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	void AddClearanceZone(const FVector& Center, float Radius, float Falloff = 100.0f);

	/**
	 * Clear all clearance zones
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	void ClearClearanceZones();

	/**
	 * Check if point is in any clearance zone
	 * @return 0-1, where 0=outside all zones, 1=fully inside zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	float GetClearanceFactor(const FVector& Point) const;

	/**
	 * Apply age simulation to spawn points
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	void ApplyAgeSimulation(TArray<FSpawnPoint>& SpawnPoints, FRandomStream& RandomStream);

	/**
	 * Apply species competition (removes points based on competition)
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	void ApplySpeciesCompetition(TArray<FSpawnPoint>& SpawnPoints, FRandomStream& RandomStream);

	/**
	 * Get noise value at location (used for clustering)
	 */
	UFUNCTION(BlueprintCallable, Category = "Distribution")
	float GetNoiseValue(const FVector& Location) const;

protected:
	/** Spatial grid for fast neighbor lookup */
	struct FSpatialGrid
	{
		TMap<FIntVector, TArray<FVector>> Grid;
		float CellSize;

		FSpatialGrid(float InCellSize = 500.0f) : CellSize(InCellSize) {}

		FIntVector GetCellIndex(const FVector& Point) const
		{
			return FIntVector(
				FMath::FloorToInt(Point.X / CellSize),
				FMath::FloorToInt(Point.Y / CellSize),
				FMath::FloorToInt(Point.Z / CellSize)
			);
		}

		void AddPoint(const FVector& Point)
		{
			FIntVector CellIndex = GetCellIndex(Point);
			Grid.FindOrAdd(CellIndex).Add(Point);
		}

		bool HasNearbyPoint(const FVector& Point, float Radius) const
		{
			FIntVector CenterCell = GetCellIndex(Point);
			int32 SearchRadius = FMath::CeilToInt(Radius / CellSize);

			for (int32 x = -SearchRadius; x <= SearchRadius; ++x)
			{
				for (int32 y = -SearchRadius; y <= SearchRadius; ++y)
				{
					for (int32 z = -SearchRadius; z <= SearchRadius; ++z)
					{
						FIntVector CellIndex = CenterCell + FIntVector(x, y, z);
						const TArray<FVector>* CellPoints = Grid.Find(CellIndex);

						if (CellPoints)
						{
							for (const FVector& ExistingPoint : *CellPoints)
							{
								if (FVector::DistSquared(Point, ExistingPoint) < Radius * Radius)
								{
									return true;
								}
							}
						}
					}
				}
			}
			return false;
		}

		void Clear()
		{
			Grid.Empty();
		}
	};
};
