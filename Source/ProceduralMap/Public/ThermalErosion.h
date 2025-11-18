// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Async/AsyncWork.h"
#include "ThermalErosion.generated.h"

/**
 * Forward declarations
 */
class FThermalErosionTask;

/**
 * UThermalErosion - Thermal weathering erosion simulation
 *
 * This class implements thermal erosion (also called thermal weathering),
 * which simulates the natural process of material sliding down slopes due to
 * gravity when the slope exceeds the material's angle of repose.
 *
 * Key Features:
 * - Slope-based material transfer
 * - Configurable angle of repose (talus angle)
 * - Iterative stabilization for realistic results
 * - Support for asynchronous execution
 * - GPU acceleration support
 *
 * Algorithm Overview:
 * 1. For each cell in the heightmap:
 *    a. Calculate the height difference with neighbors
 *    b. If difference exceeds talus angle threshold, material slides
 *    c. Distribute material to lower neighbors proportionally
 * 2. Repeat for desired number of iterations until stable
 *
 * Thermal erosion is excellent for:
 * - Creating realistic scree slopes
 * - Smoothing steep cliffs
 * - Generating natural-looking weathering patterns
 * - Complementing hydraulic erosion for complete terrain realism
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAP_API UThermalErosion : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Constructor
	 */
	UThermalErosion();

	// ========================================
	// Main API
	// ========================================

	/**
	 * Apply thermal erosion to the heightmap
	 * @param HeightMap - Heightmap to erode (modified in-place)
	 * @param MapSize - Size of the heightmap (must be square: MapSize x MapSize)
	 * @param bAsync - If true, executes erosion asynchronously (returns immediately)
	 * @return True if erosion started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Erosion")
	bool Erode(TArray<float>& HeightMap, int32 MapSize, bool bAsync = false);

	/**
	 * Apply a single iteration of thermal erosion
	 * @param HeightMap - Heightmap to erode
	 * @param MapSize - Size of heightmap
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Erosion")
	void ApplySingleIteration(TArray<float>& HeightMap, int32 MapSize);

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

	// ========================================
	// Configuration Properties
	// ========================================

	/** Number of erosion iterations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Basic", meta = (ClampMin = "1", ClampMax = "1000"))
	int32 NumIterations;

	/** Angle of repose in degrees (typical values: 30-45 for soil, 35-40 for rock) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float TalusAngle;

	/** Amount of material to transfer per iteration (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ErosionRate;

	/** Cell size in world units (for proper slope calculation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics", meta = (ClampMin = "0.1", ClampMax = "1000.0"))
	float CellSize;

	/**
	 * Use 8-neighbor (Moore) neighborhood instead of 4-neighbor (Von Neumann)
	 * 8-neighbor produces more natural results but is slightly slower
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Advanced")
	bool bUse8Neighbors;

	/**
	 * Apply smoothing to prevent sharp transitions
	 * Recommended for high erosion rates
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Advanced")
	bool bApplySmoothing;

	/** Smoothing factor (only used if bApplySmoothing is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Advanced", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SmoothingFactor;

protected:
	// ========================================
	// Internal Methods
	// ========================================

	/**
	 * Calculate the maximum height difference allowed by talus angle
	 * @return Maximum height difference (in heightmap units)
	 */
	FORCEINLINE float CalculateTalusThreshold() const
	{
		const float TalusRadians = FMath::DegreesToRadians(TalusAngle);
		return FMath::Tan(TalusRadians) * CellSize;
	}

	/**
	 * Get neighbor offsets based on neighborhood type
	 * @param OutOffsets - Array to fill with neighbor offsets
	 */
	void GetNeighborOffsets(TArray<FIntPoint>& OutOffsets) const;

	/**
	 * Calculate total height difference with neighbors that exceeds talus
	 * @param HeightMap - Heightmap to analyze
	 * @param MapSize - Size of heightmap
	 * @param X - X coordinate
	 * @param Y - Y coordinate
	 * @param Neighbors - Neighbor offsets
	 * @return Total height difference that needs to be eroded
	 */
	float CalculateExcessHeight(const TArray<float>& HeightMap, int32 MapSize, int32 X, int32 Y, const TArray<FIntPoint>& Neighbors) const;

	/**
	 * Apply smoothing filter to heightmap
	 * @param HeightMap - Heightmap to smooth
	 * @param MapSize - Size of heightmap
	 */
	void ApplySmoothingFilter(TArray<float>& HeightMap, int32 MapSize);

	// ========================================
	// Internal State
	// ========================================

	/** Async task for background erosion */
	FAsyncTask<FThermalErosionTask>* AsyncErosionTask;

	/** Critical section for thread safety */
	FCriticalSection ErosionMutex;

	friend class FThermalErosionTask;
};

/**
 * FThermalErosionTask - Async task for background thermal erosion
 */
class FThermalErosionTask : public FNonAbandonableTask
{
public:
	FThermalErosionTask(UThermalErosion* InErosion, TArray<float>* InHeightMap, int32 InMapSize)
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(FThermalErosionTask, STATGROUP_ThreadPoolAsyncTasks);
	}

private:
	UThermalErosion* Erosion;
	TArray<float>* HeightMap;
	int32 MapSize;
};
