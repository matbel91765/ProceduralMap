// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "QuadTreeNode.h"
#include "LODManager.h"
#include "QuadTreeTerrain.generated.h"

// Forward declarations
class FastNoiseLite;

/**
 * Terrain generation parameters
 */
USTRUCT(BlueprintType)
struct FTerrainGenerationParams
{
	GENERATED_BODY()

	// Noise seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 Seed;

	// Noise frequency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Frequency;

	// Noise octaves for fractal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 Octaves;

	// Noise lacunarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Lacunarity;

	// Noise gain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Gain;

	// Height scale multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float HeightScale;

	// Height offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float HeightOffset;

	FTerrainGenerationParams()
		: Seed(1337)
		, Frequency(0.005f)
		, Octaves(6)
		, Lacunarity(2.0f)
		, Gain(0.5f)
		, HeightScale(500.0f)
		, HeightOffset(0.0f)
	{
	}
};

/**
 * UQuadTreeTerrain - Main QuadTree terrain management component
 * Handles dynamic LOD, mesh generation, and terrain updates
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROCEDURALMAP_API UQuadTreeTerrain : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuadTreeTerrain();

	// ========================================================================
	// Configuration
	// ========================================================================

	// Total terrain size (width and height)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Configuration")
	float TerrainSize;

	// Maximum LOD level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Configuration")
	int32 MaxLODLevel;

	// LOD distance multiplier (higher = more aggressive LOD)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Configuration")
	float LODDistanceMultiplier;

	// Base mesh resolution (vertices per side at LOD 0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Configuration")
	int32 MeshResolution;

	// Material to apply to terrain meshes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering")
	UMaterialInterface* TerrainMaterial;

	// Enable collision on terrain meshes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Physics")
	bool bEnableCollision;

	// Collision complexity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Physics")
	TEnumAsByte<ECollisionTraceFlag> CollisionComplexity;

	// Terrain generation parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation")
	FTerrainGenerationParams GenerationParams;

	// Biome name for LOD configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Biome")
	FName BiomeName;

	// Update LOD every N frames (1 = every frame, 2 = every other frame, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Performance")
	int32 UpdateFrequency;

	// Enable async mesh generation (experimental)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Performance")
	bool bEnableAsyncGeneration;

	// Maximum number of mesh generations per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Performance")
	int32 MaxMeshGenerationsPerFrame;

	// Enable smooth normals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering")
	bool bSmoothNormals;

	// Use tangent space for normals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering")
	bool bUseTangents;

	// ========================================================================
	// Debug
	// ========================================================================

	// Show QuadTree debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Debug")
	bool bShowDebugQuadTree;

	// Show LOD level colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Debug")
	bool bShowLODColors;

	// Show wireframe
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Debug")
	bool bShowWireframe;

	// ========================================================================
	// Runtime Data
	// ========================================================================

	// Root node of the QuadTree
	TSharedPtr<FQuadTreeNode> RootNode;

	// LOD Manager reference
	UPROPERTY()
	ULODManager* LODManager;

	// FastNoiseLite instance for terrain generation
	TSharedPtr<FastNoiseLite> NoiseGenerator;

	// Frame counter for update frequency
	int32 FrameCounter;

	// Queue of nodes to update
	TArray<TSharedPtr<FQuadTreeNode>> UpdateQueue;

	// Cache of camera position
	FVector CachedCameraPosition;

	// ========================================================================
	// Lifecycle
	// ========================================================================

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ========================================================================
	// Public API
	// ========================================================================

	/**
	 * Initialize the terrain system
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void InitializeTerrain();

	/**
	 * Update LOD based on camera position
	 * @param CameraPosition - World position of the camera
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void UpdateLOD(const FVector& CameraPosition);

	/**
	 * Force complete regeneration of terrain
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void RegenerateTerrain();

	/**
	 * Cleanup all terrain resources
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void CleanupTerrain();

	/**
	 * Get height at world position
	 * @param WorldPosition - Position in world space
	 * @return Height value
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	float GetHeightAtPosition(const FVector& WorldPosition) const;

	/**
	 * Get normal at world position
	 * @param WorldPosition - Position in world space
	 * @return Normal vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	FVector GetNormalAtPosition(const FVector& WorldPosition) const;

	/**
	 * Get current LOD statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	FLODStatistics GetLODStatistics() const;

	// ========================================================================
	// Private Methods
	// ========================================================================

private:
	/**
	 * Recursive LOD update for a node
	 * @param Node - Node to update
	 * @param CameraPosition2D - Camera position in 2D
	 * @param CurrentTime - Current game time
	 */
	void UpdateNodeLOD(TSharedPtr<FQuadTreeNode> Node, const FVector2D& CameraPosition2D, float CurrentTime);

	/**
	 * Split a node into 4 children
	 * @param Node - Node to split
	 * @return True if split was successful
	 */
	bool SplitNode(TSharedPtr<FQuadTreeNode> Node);

	/**
	 * Merge a node's children back into the parent
	 * @param Node - Parent node
	 * @return True if merge was successful
	 */
	bool MergeNode(TSharedPtr<FQuadTreeNode> Node);

	/**
	 * Generate procedural mesh for a node
	 * @param Node - Node to generate mesh for
	 */
	void GenerateMeshForNode(TSharedPtr<FQuadTreeNode> Node);

	/**
	 * Calculate noise height at 2D position
	 * @param Position - 2D position
	 * @return Height value
	 */
	float CalculateHeightAt(const FVector2D& Position) const;

	/**
	 * Calculate normal at 2D position using finite differences
	 * @param Position - 2D position
	 * @param SampleDistance - Distance for sampling neighbors
	 * @return Normal vector
	 */
	FVector CalculateNormalAt(const FVector2D& Position, float SampleDistance = 10.0f) const;

	/**
	 * Create a mesh component for a node
	 * @param Node - Node to create component for
	 * @return Created mesh component
	 */
	UProceduralMeshComponent* CreateMeshComponent(TSharedPtr<FQuadTreeNode> Node);

	/**
	 * Destroy mesh component for a node
	 * @param Node - Node to destroy component for
	 */
	void DestroyMeshComponent(TSharedPtr<FQuadTreeNode> Node);

	/**
	 * Check if node is visible (frustum culling)
	 * @param Node - Node to check
	 * @return True if visible
	 */
	bool IsNodeVisible(TSharedPtr<FQuadTreeNode> Node) const;

	/**
	 * Draw debug visualization for a node
	 * @param Node - Node to visualize
	 */
	void DrawDebugNode(TSharedPtr<FQuadTreeNode> Node) const;

	/**
	 * Recursively collect statistics
	 * @param Node - Starting node
	 * @param OutStats - Output statistics
	 */
	void CollectStatistics(TSharedPtr<FQuadTreeNode> Node, FLODStatistics& OutStats) const;

	/**
	 * Get camera position from player or other source
	 */
	FVector GetCameraPosition() const;

	/**
	 * Initialize noise generator with current parameters
	 */
	void InitializeNoiseGenerator();

	/**
	 * Process update queue (mesh generations)
	 * @param MaxUpdates - Maximum number of updates to process
	 */
	void ProcessUpdateQueue(int32 MaxUpdates);

	/**
	 * Calculate distance from 2D position to camera
	 */
	float CalculateDistanceToCamera(const FVector2D& Position, const FVector2D& CameraPosition2D) const;

	/**
	 * Check if we can perform more splits this frame
	 */
	bool CanSplitThisFrame() const;

	/**
	 * Check if we can perform more merges this frame
	 */
	bool CanMergeThisFrame() const;
};
