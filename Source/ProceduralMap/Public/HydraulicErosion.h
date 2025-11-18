// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Async/AsyncWork.h"
#include "HydraulicErosion.generated.h"

/**
 * Forward declarations
 */
class FHydraulicErosionTask;

/**
 * FBrushIndices - Stores precomputed brush pattern indices
 * Used for efficient erosion/deposition in a circular area
 */
struct FBrushIndices
{
	TArray<int32> Indices;
	TArray<float> Weights;
	int32 NumIndices;

	FBrushIndices()
		: NumIndices(0)
	{
	}
};

/**
 * FErosionDroplet - Represents a single water droplet for particle-based erosion
 * Based on Sebastian Lague's implementation
 */
struct FErosionDroplet
{
	FVector2D Position;
	FVector2D Direction;
	float Speed;
	float Water;
	float Sediment;

	FErosionDroplet()
		: Position(FVector2D::ZeroVector)
		, Direction(FVector2D::ZeroVector)
		, Speed(0.0f)
		, Water(1.0f)
		, Sediment(0.0f)
	{
	}
};

/**
 * UHydraulicErosion - Particle-based hydraulic erosion simulation
 *
 * This class implements a sophisticated hydraulic erosion algorithm based on
 * Sebastian Lague's particle simulation approach. Each iteration simulates
 * thousands of water droplets flowing down the terrain, picking up and
 * depositing sediment to create realistic erosion patterns.
 *
 * Key Features:
 * - Particle-based simulation for realistic erosion patterns
 * - Configurable parameters for fine control over erosion characteristics
 * - Bilinear interpolation for smooth height sampling
 * - Precomputed brush patterns for performance
 * - Support for asynchronous execution
 * - GPU acceleration support via compute shaders
 *
 * Algorithm Overview:
 * 1. Initialize random droplet positions
 * 2. For each droplet:
 *    a. Calculate height gradient at current position
 *    b. Update direction and speed based on gravity
 *    c. Erode terrain based on speed and capacity
 *    d. Deposit sediment when speed decreases
 *    e. Evaporate water over time
 * 3. Repeat until droplet stops or max iterations reached
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAP_API UHydraulicErosion : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Constructor
	 */
	UHydraulicErosion();

	// ========================================
	// Main API
	// ========================================

	/**
	 * Erode the heightmap using particle-based hydraulic erosion
	 * @param HeightMap - Heightmap to erode (modified in-place)
	 * @param MapSize - Size of the heightmap (must be square: MapSize x MapSize)
	 * @param bAsync - If true, executes erosion asynchronously (returns immediately)
	 * @return True if erosion started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Erosion")
	bool Erode(TArray<float>& HeightMap, int32 MapSize, bool bAsync = false);

	/**
	 * Check if async erosion is complete
	 * @return True if no erosion task is running
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain|Erosion")
	bool IsErosionComplete() const;

	/**
	 * Wait for async erosion to complete (blocking)
	 * @param TimeoutSeconds - Maximum time to wait (0 = infinite)
	 * @return True if erosion completed, false if timeout
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Erosion")
	bool WaitForCompletion(float TimeoutSeconds = 0.0f);

	/**
	 * Initialize brush indices for the given erosion radius
	 * Must be called before erosion if ErosionRadius changes
	 * Automatically called by Erode() if needed
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Erosion")
	void InitializeBrushIndices();

	// ========================================
	// Configuration Properties
	// ========================================

	/** Number of erosion iterations (droplets to simulate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Basic", meta = (ClampMin = "1", ClampMax = "1000000"))
	int32 NumIterations;

	/** Radius of erosion brush in cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Basic", meta = (ClampMin = "1", ClampMax = "16"))
	int32 ErosionRadius;

	/** Random seed for droplet placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Basic")
	int32 RandomSeed;

	/** Max number of steps a droplet can take */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Advanced", meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxDropletLifetime;

	/** Droplet inertia (0-1, higher = more momentum, follows old direction more) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Inertia;

	/** Multiplier for sediment capacity (how much sediment droplet can carry) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float SedimentCapacityFactor;

	/** Minimum slope required for sediment capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinSedimentCapacity;

	/** Speed at which terrain is eroded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Rates", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ErodeSpeed;

	/** Speed at which sediment is deposited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Rates", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DepositSpeed;

	/** Rate at which water evaporates (0-1, higher = faster evaporation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Rates", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EvaporateSpeed;

	/** Gravity strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float Gravity;

	/** Initial water volume for each droplet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float InitialWaterVolume;

	/** Initial speed for each droplet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float InitialSpeed;

protected:
	// ========================================
	// Internal Methods
	// ========================================

	/**
	 * Simulate a single erosion droplet
	 * @param HeightMap - Heightmap to erode
	 * @param MapSize - Size of heightmap
	 * @param StartX - Starting X position (0-1)
	 * @param StartY - Starting Y position (0-1)
	 * @param RandomStream - Random stream for deterministic randomness
	 */
	void SimulateDroplet(TArray<float>& HeightMap, int32 MapSize, float StartX, float StartY, FRandomStream& RandomStream);

	/**
	 * Calculate height and gradient at a position using bilinear interpolation
	 * @param HeightMap - Heightmap to sample
	 * @param MapSize - Size of heightmap
	 * @param PosX - X position to sample (in cells)
	 * @param PosY - Y position to sample (in cells)
	 * @param OutGradient - Calculated gradient vector (normalized)
	 * @return Interpolated height value
	 */
	float CalculateHeightAndGradient(const TArray<float>& HeightMap, int32 MapSize, float PosX, float PosY, FVector2D& OutGradient) const;

	/**
	 * Erode or deposit sediment at a position
	 * @param HeightMap - Heightmap to modify
	 * @param MapSize - Size of heightmap
	 * @param PosX - X position (in cells)
	 * @param PosY - Y position (in cells)
	 * @param Amount - Amount to erode (positive) or deposit (negative)
	 */
	void ErodeOrDeposit(TArray<float>& HeightMap, int32 MapSize, float PosX, float PosY, float Amount);

	// ========================================
	// Internal State
	// ========================================

	/** Precomputed brush indices for erosion/deposition */
	TArray<FBrushIndices> BrushIndices;

	/** Current heightmap size (for brush indices validation) */
	int32 CurrentMapSize;

	/** Current erosion radius (for brush indices validation) */
	int32 CurrentBrushRadius;

	/** Async task for background erosion */
	FAsyncTask<FHydraulicErosionTask>* AsyncErosionTask;

	/** Critical section for thread safety */
	FCriticalSection ErosionMutex;

	friend class FHydraulicErosionTask;
};

/**
 * FHydraulicErosionTask - Async task for background erosion
 */
class FHydraulicErosionTask : public FNonAbandonableTask
{
public:
	FHydraulicErosionTask(UHydraulicErosion* InErosion, TArray<float>* InHeightMap, int32 InMapSize)
		: Erosion(InErosion)
		, HeightMap(InHeightMap)
		, MapSize(InMapSize)
	{
	}

	void DoWork()
	{
		if (Erosion && HeightMap)
		{
			Erosion->Erode(*HeightMap, MapSize, false);
		}
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FHydraulicErosionTask, STATGROUP_ThreadPoolAsyncTasks);
	}

private:
	UHydraulicErosion* Erosion;
	TArray<float>* HeightMap;
	int32 MapSize;
};
