// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LODManager.generated.h"

/**
 * Biome-specific LOD configuration
 */
USTRUCT(BlueprintType)
struct FBiomeLODConfig
{
	GENERATED_BODY()

	// Biome name identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	FName BiomeName;

	// Distance multiplier for this biome (higher = more aggressive LOD)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float DistanceMultiplier;

	// Maximum LOD level for this biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	int32 MaxLODLevel;

	// Mesh resolution override for this biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	int32 MeshResolution;

	// Height scale multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float HeightScale;

	// Enable special rendering features (foliage, detail meshes, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bEnableDetailFeatures;

	FBiomeLODConfig()
		: BiomeName(NAME_None)
		, DistanceMultiplier(1.0f)
		, MaxLODLevel(6)
		, MeshResolution(32)
		, HeightScale(1.0f)
		, bEnableDetailFeatures(true)
	{
	}
};

/**
 * LOD Statistics for debugging and profiling
 */
USTRUCT(BlueprintType)
struct FLODStatistics
{
	GENERATED_BODY()

	// Total number of active nodes in the QuadTree
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalNodes;

	// Number of visible leaf nodes
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 VisibleLeafNodes;

	// Number of nodes per LOD level
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	TArray<int32> NodesPerLODLevel;

	// Total memory usage (bytes)
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int64 TotalMemoryUsage;

	// Number of split operations this frame
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 SplitsThisFrame;

	// Number of merge operations this frame
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 MergesThisFrame;

	// Number of mesh updates this frame
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 MeshUpdatesThisFrame;

	// Frame time for LOD updates (milliseconds)
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float UpdateTimeMS;

	// Average distance to camera for active nodes
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float AverageCameraDistance;

	FLODStatistics()
		: TotalNodes(0)
		, VisibleLeafNodes(0)
		, TotalMemoryUsage(0)
		, SplitsThisFrame(0)
		, MergesThisFrame(0)
		, MeshUpdatesThisFrame(0)
		, UpdateTimeMS(0.0f)
		, AverageCameraDistance(0.0f)
	{
	}

	void Reset()
	{
		TotalNodes = 0;
		VisibleLeafNodes = 0;
		NodesPerLODLevel.Empty();
		TotalMemoryUsage = 0;
		SplitsThisFrame = 0;
		MergesThisFrame = 0;
		MeshUpdatesThisFrame = 0;
		UpdateTimeMS = 0.0f;
		AverageCameraDistance = 0.0f;
	}
};

/**
 * ULODManager - Global LOD management system
 * Handles LOD calculations, biome configurations, and statistics
 */
UCLASS(BlueprintType, Config=Game)
class PROCEDURALMAP_API ULODManager : public UObject
{
	GENERATED_BODY()

public:
	ULODManager();

	// ========================================================================
	// Configuration
	// ========================================================================

	// Base distance multiplier for all LOD calculations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Configuration", Config)
	float BaseLODDistanceMultiplier;

	// Hysteresis factor to prevent flickering (split at distance D, merge at distance D * Hysteresis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Configuration", Config)
	float HysteresisFactor;

	// Minimum time between LOD updates for a node (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Configuration", Config)
	float MinUpdateInterval;

	// Maximum nodes that can be split per frame (performance budget)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Configuration", Config)
	int32 MaxSplitsPerFrame;

	// Maximum nodes that can be merged per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Configuration", Config)
	int32 MaxMergesPerFrame;

	// Enable frustum culling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Configuration", Config)
	bool bEnableFrustumCulling;

	// Enable distance-based culling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Configuration", Config)
	bool bEnableDistanceCulling;

	// Maximum render distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Configuration", Config)
	float MaxRenderDistance;

	// Biome-specific configurations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Biomes", Config)
	TArray<FBiomeLODConfig> BiomeConfigs;

	// ========================================================================
	// Statistics
	// ========================================================================

	// Current frame statistics
	UPROPERTY(BlueprintReadOnly, Category = "LOD|Statistics")
	FLODStatistics CurrentStatistics;

	// Enable statistics collection (has performance cost)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Debug", Config)
	bool bEnableStatistics;

	// Show debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Debug", Config)
	bool bShowDebugVisualization;

	// Debug draw color per LOD level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Debug")
	TArray<FColor> LODDebugColors;

	// ========================================================================
	// Public Methods
	// ========================================================================

	/**
	 * Calculate optimal split distance for a given LOD level
	 * @param LODLevel - Current LOD level
	 * @param NodeSize - Size of the node
	 * @param BiomeName - Optional biome name for custom configuration
	 * @return Distance at which node should split
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	float CalculateSplitDistance(int32 LODLevel, float NodeSize, FName BiomeName = NAME_None) const;

	/**
	 * Calculate merge distance (with hysteresis)
	 * @param LODLevel - Current LOD level
	 * @param NodeSize - Size of the node
	 * @param BiomeName - Optional biome name for custom configuration
	 * @return Distance at which node should merge
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	float CalculateMergeDistance(int32 LODLevel, float NodeSize, FName BiomeName = NAME_None) const;

	/**
	 * Get biome configuration
	 * @param BiomeName - Name of the biome
	 * @return Biome configuration, or default if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	FBiomeLODConfig GetBiomeConfig(FName BiomeName) const;

	/**
	 * Check if node should be split based on camera distance
	 * @param CameraDistance - Distance to camera
	 * @param LODLevel - Current LOD level
	 * @param NodeSize - Size of the node
	 * @param LastUpdateTime - Time of last update (for stability)
	 * @param CurrentTime - Current game time
	 * @param BiomeName - Optional biome name
	 * @return True if node should split
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	bool ShouldSplitNode(float CameraDistance, int32 LODLevel, float NodeSize,
						 float LastUpdateTime, float CurrentTime, FName BiomeName = NAME_None) const;

	/**
	 * Check if node should merge based on camera distance
	 * @param CameraDistance - Distance to camera
	 * @param LODLevel - Current LOD level
	 * @param NodeSize - Size of the node
	 * @param LastUpdateTime - Time of last update (for stability)
	 * @param CurrentTime - Current game time
	 * @param BiomeName - Optional biome name
	 * @return True if node should merge
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	bool ShouldMergeNode(float CameraDistance, int32 LODLevel, float NodeSize,
						 float LastUpdateTime, float CurrentTime, FName BiomeName = NAME_None) const;

	/**
	 * Get optimal mesh resolution for a given LOD level
	 * @param LODLevel - LOD level
	 * @param BaseMeshResolution - Base resolution at LOD 0
	 * @param BiomeName - Optional biome name
	 * @return Mesh resolution (vertices per side)
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	int32 GetMeshResolutionForLOD(int32 LODLevel, int32 BaseMeshResolution, FName BiomeName = NAME_None) const;

	/**
	 * Begin statistics collection for this frame
	 */
	void BeginFrame();

	/**
	 * End statistics collection and finalize
	 */
	void EndFrame(float DeltaTime);

	/**
	 * Record a split operation
	 */
	void RecordSplit();

	/**
	 * Record a merge operation
	 */
	void RecordMerge();

	/**
	 * Record a mesh update
	 */
	void RecordMeshUpdate();

	/**
	 * Get singleton instance
	 */
	static ULODManager* Get(UWorld* World);

	/**
	 * Initialize default biome configurations
	 */
	void InitializeDefaultBiomes();

	/**
	 * Get debug color for LOD level
	 */
	FColor GetDebugColorForLOD(int32 LODLevel) const;

private:
	// Singleton instance per world
	static TMap<UWorld*, ULODManager*> ManagerInstances;

	// Frame statistics accumulator
	double FrameStartTime;
};
