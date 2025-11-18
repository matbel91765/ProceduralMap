// FoliageCulling.h - Foliage Culling and Optimization System
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "FoliageCulling.generated.h"

/**
 * Culling strategy type
 */
UENUM(BlueprintType)
enum class ECullingStrategy : uint8
{
	/** Distance-based culling only */
	Distance UMETA(DisplayName = "Distance"),

	/** Frustum culling only */
	Frustum UMETA(DisplayName = "Frustum"),

	/** Combined distance and frustum */
	Combined UMETA(DisplayName = "Combined"),

	/** Distance + Frustum + Occlusion hints */
	Advanced UMETA(DisplayName = "Advanced")
};

/**
 * LOD configuration for culling
 */
USTRUCT(BlueprintType)
struct FFoliageLODSettings
{
	GENERATED_BODY()

	/** LOD 0 distance (highest detail) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float LOD0Distance = 0.0f;

	/** LOD 1 distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float LOD1Distance = 2000.0f;

	/** LOD 2 distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float LOD2Distance = 5000.0f;

	/** LOD 3 distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float LOD3Distance = 10000.0f;

	/** Maximum draw distance (culled beyond) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float MaxDrawDistance = 20000.0f;

	/** Enable LOD dithering for smooth transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bEnableLODDithering = true;

	/** LOD bias (negative = higher quality, positive = lower quality) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "-3.0", ClampMax = "3.0"))
	float LODBias = 0.0f;

	FFoliageLODSettings()
	{
	}

	/** Get LOD level for distance */
	int32 GetLODForDistance(float Distance) const
	{
		Distance += LODBias * 1000.0f; // Apply bias

		if (Distance <= LOD0Distance)
			return 0;
		if (Distance <= LOD1Distance)
			return 1;
		if (Distance <= LOD2Distance)
			return 2;
		if (Distance <= LOD3Distance)
			return 3;
		return 4; // Culled
	}
};

/**
 * Culling statistics
 */
USTRUCT(BlueprintType)
struct FFoliageCullingStats
{
	GENERATED_BODY()

	/** Total instances */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalInstances = 0;

	/** Visible instances */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 VisibleInstances = 0;

	/** Distance culled instances */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 DistanceCulled = 0;

	/** Frustum culled instances */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 FrustumCulled = 0;

	/** Occlusion culled instances */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 OcclusionCulled = 0;

	/** Current LOD 0 count */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 LOD0Count = 0;

	/** Current LOD 1 count */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 LOD1Count = 0;

	/** Current LOD 2 count */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 LOD2Count = 0;

	/** Current LOD 3 count */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 LOD3Count = 0;

	/** Last update time (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float LastUpdateTime = 0.0f;

	FFoliageCullingStats()
	{
	}

	void Reset()
	{
		TotalInstances = 0;
		VisibleInstances = 0;
		DistanceCulled = 0;
		FrustumCulled = 0;
		OcclusionCulled = 0;
		LOD0Count = 0;
		LOD1Count = 0;
		LOD2Count = 0;
		LOD3Count = 0;
		LastUpdateTime = 0.0f;
	}

	float GetCullPercentage() const
	{
		return TotalInstances > 0 ? (100.0f * (TotalInstances - VisibleInstances) / TotalInstances) : 0.0f;
	}
};

/**
 * Foliage Culling Manager
 * Manages distance culling, frustum culling, and LOD for foliage instances
 */
UCLASS(Blueprintable, ClassGroup = "ProceduralMap")
class PROCEDURALMAP_API UFoliageCulling : public UObject
{
	GENERATED_BODY()

public:
	UFoliageCulling();

	// ============================================
	// Configuration
	// ============================================

	/** Culling strategy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	ECullingStrategy CullingStrategy = ECullingStrategy::Combined;

	/** Enable distance-based culling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Culling")
	bool bEnableDistanceCulling = true;

	/** Default culling distance (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Culling", meta = (ClampMin = "1000.0"))
	float DefaultCullDistance = 20000.0f;

	/** Distance culling fade range (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Culling", meta = (ClampMin = "0.0"))
	float CullDistanceFadeRange = 1000.0f;

	/** Enable frustum culling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustum Culling")
	bool bEnableFrustumCulling = true;

	/** Frustum culling margin (extra culling distance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustum Culling", meta = (ClampMin = "0.0"))
	float FrustumCullingMargin = 500.0f;

	/** Enable occlusion culling hints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion Culling")
	bool bEnableOcclusionHints = false;

	/** Occlusion query distance (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion Culling", meta = (ClampMin = "1000.0"))
	float OcclusionQueryDistance = 10000.0f;

	/** LOD settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	FFoliageLODSettings LODSettings;

	/** Enable dynamic LOD based on performance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bEnableDynamicLOD = false;

	/** Target frame time (ms) for dynamic LOD */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "8.0", ClampMax = "33.0"))
	float TargetFrameTime = 16.67f;

	/** Culling update frequency (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
	float UpdateFrequency = 0.1f;

	/** Enable spatial partitioning for faster culling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableSpatialPartitioning = true;

	/** Spatial partition cell size (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000.0"))
	float PartitionCellSize = 10000.0f;

	/** Max instances to update per frame (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0"))
	int32 MaxInstancesPerFrame = 1000;

	/** Enable statistics tracking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableStats = false;

	/** Draw debug culling visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = false;

	// ============================================
	// Methods
	// ============================================

	/**
	 * Update culling for HISM component
	 * @param HISMComponent - Component to update
	 * @param ViewLocation - Camera/view location
	 * @param ViewRotation - Camera/view rotation
	 * @param DeltaTime - Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category = "Culling")
	void UpdateCulling(
		UHierarchicalInstancedStaticMeshComponent* HISMComponent,
		const FVector& ViewLocation,
		const FRotator& ViewRotation,
		float DeltaTime
	);

	/**
	 * Update culling for multiple HISM components
	 */
	UFUNCTION(BlueprintCallable, Category = "Culling")
	void UpdateCullingBatch(
		const TArray<UHierarchicalInstancedStaticMeshComponent*>& HISMComponents,
		const FVector& ViewLocation,
		const FRotator& ViewRotation,
		float DeltaTime
	);

	/**
	 * Set custom culling distance for component
	 */
	UFUNCTION(BlueprintCallable, Category = "Culling")
	void SetCullDistance(UHierarchicalInstancedStaticMeshComponent* HISMComponent, float Distance);

	/**
	 * Set custom LOD distances for component
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void SetLODDistances(
		UHierarchicalInstancedStaticMeshComponent* HISMComponent,
		const FFoliageLODSettings& InLODSettings
	);

	/**
	 * Calculate optimal LOD bias based on current performance
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	float CalculateOptimalLODBias(float CurrentFrameTime);

	/**
	 * Get culling statistics
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	FFoliageCullingStats GetCullingStats() const { return CullingStats; }

	/**
	 * Reset culling statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ResetStats();

	/**
	 * Check if instance should be culled by distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Culling")
	bool ShouldCullByDistance(const FVector& InstanceLocation, const FVector& ViewLocation, float CullDistance) const;

	/**
	 * Check if instance should be culled by frustum
	 */
	UFUNCTION(BlueprintCallable, Category = "Culling")
	bool ShouldCullByFrustum(const FVector& InstanceLocation, const FVector& ViewLocation, const FRotator& ViewRotation) const;

	/**
	 * Get LOD level for instance based on distance
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	int32 GetLODLevel(const FVector& InstanceLocation, const FVector& ViewLocation) const;

	/**
	 * Enable/disable culling system
	 */
	UFUNCTION(BlueprintCallable, Category = "Culling")
	void SetCullingEnabled(bool bEnabled);

	/**
	 * Get total instances managed
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	int32 GetTotalInstanceCount() const;

	/**
	 * Get visible instance count
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	int32 GetVisibleInstanceCount() const;

protected:
	// ============================================
	// Internal Methods
	// ============================================

	/** Update distance culling */
	void UpdateDistanceCulling(
		UHierarchicalInstancedStaticMeshComponent* HISMComponent,
		const FVector& ViewLocation
	);

	/** Update frustum culling */
	void UpdateFrustumCulling(
		UHierarchicalInstancedStaticMeshComponent* HISMComponent,
		const FVector& ViewLocation,
		const FRotator& ViewRotation
	);

	/** Update LOD levels */
	void UpdateLODLevels(
		UHierarchicalInstancedStaticMeshComponent* HISMComponent,
		const FVector& ViewLocation
	);

	/** Build spatial partition */
	void BuildSpatialPartition(UHierarchicalInstancedStaticMeshComponent* HISMComponent);

	/** Draw debug visualization */
	void DrawDebugCulling(
		UHierarchicalInstancedStaticMeshComponent* HISMComponent,
		const FVector& ViewLocation
	);

protected:
	// ============================================
	// Internal Data
	// ============================================

	/** Culling statistics */
	UPROPERTY()
	FFoliageCullingStats CullingStats;

	/** Last update time */
	float LastUpdateTime = 0.0f;

	/** Accumulated delta time */
	float AccumulatedDeltaTime = 0.0f;

	/** Spatial partition grid */
	struct FSpatialPartition
	{
		TMap<FIntVector, TArray<int32>> InstanceGrid;
		float CellSize = 10000.0f;

		FIntVector GetCellIndex(const FVector& Location) const
		{
			return FIntVector(
				FMath::FloorToInt(Location.X / CellSize),
				FMath::FloorToInt(Location.Y / CellSize),
				FMath::FloorToInt(Location.Z / CellSize)
			);
		}

		void Clear()
		{
			InstanceGrid.Empty();
		}
	};

	/** Spatial partitions per component */
	TMap<UHierarchicalInstancedStaticMeshComponent*, FSpatialPartition> SpatialPartitions;

	/** Custom cull distances per component */
	TMap<UHierarchicalInstancedStaticMeshComponent*, float> CustomCullDistances;

	/** Custom LOD settings per component */
	TMap<UHierarchicalInstancedStaticMeshComponent*, FFoliageLODSettings> CustomLODSettings;

	/** Performance tracking for dynamic LOD */
	TArray<float> FrameTimeHistory;
	static constexpr int32 MaxFrameTimeHistory = 60;
};
