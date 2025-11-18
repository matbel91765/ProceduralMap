// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "TerrainGenerationTask.h"
#include "TerrainChunk.generated.h"

/**
 * Forward declarations
 */
class UProceduralMeshComponent;
class UMaterialInterface;

/**
 * ATerrainChunk - Represents a single chunk of procedurally generated terrain
 *
 * This actor manages one chunk of the infinite terrain system. Each chunk:
 * - Has a ProceduralMeshComponent for rendering
 * - Generates its mesh asynchronously to avoid frame hitches
 * - Supports multiple LOD levels based on distance from player
 * - Can be pooled and reused for different coordinates
 *
 * The chunk is identified by integer coordinates (ChunkX, ChunkY) in the chunk grid.
 * The actual world position is calculated as (ChunkX * ChunkSize, ChunkY * ChunkSize, 0)
 */
UCLASS()
class PROCEDURALMAP_API ATerrainChunk : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * Constructor
	 */
	ATerrainChunk();

	/**
	 * Called every frame
	 * @param DeltaTime - Time since last frame
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Initialize the chunk with generation parameters
	 * @param InChunkX - X coordinate in chunk grid
	 * @param InChunkY - Y coordinate in chunk grid
	 * @param InSeed - Random seed for procedural generation
	 * @param InLODLevel - Level of detail (0 = highest, higher = lower detail)
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void Initialize(int32 InChunkX, int32 InChunkY, int32 InSeed, int32 InLODLevel = 0);

	/**
	 * Start generating the terrain mesh asynchronously
	 * This spawns a background task that will generate the mesh data
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void StartGeneration();

	/**
	 * Check if generation is complete and apply the mesh if ready
	 * Call this every frame or on a timer to check for completion
	 * @return True if mesh was applied this frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	bool UpdateGeneration();

	/**
	 * Update the LOD level for this chunk
	 * @param NewLODLevel - New LOD level to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void SetLODLevel(int32 NewLODLevel);

	/**
	 * Get the current LOD level
	 * @return Current LOD level
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	int32 GetLODLevel() const { return CurrentLODLevel; }

	/**
	 * Check if this chunk is currently generating
	 * @return True if generation task is running
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	bool IsGenerating() const { return GenerationTask != nullptr && !GenerationTask->IsDone(); }

	/**
	 * Check if this chunk has finished generating
	 * @return True if mesh is ready
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	bool IsReady() const { return bIsGenerated; }

	/**
	 * Get chunk coordinates
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	void GetChunkCoordinates(int32& OutChunkX, int32& OutChunkY) const
	{
		OutChunkX = ChunkX;
		OutChunkY = ChunkY;
	}

	/**
	 * Get the world position of this chunk
	 * @return World position of chunk origin
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	FVector GetChunkWorldPosition() const
	{
		return FVector(ChunkX * ChunkSize, ChunkY * ChunkSize, 0.0f);
	}

	/**
	 * Set the material for the terrain mesh
	 * @param Material - Material to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void SetTerrainMaterial(UMaterialInterface* Material);

	/**
	 * Clear the mesh and reset generation state
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void ClearMesh();

protected:
	/**
	 * Called when the game starts or when spawned
	 */
	virtual void BeginPlay() override;

	/**
	 * Called when actor is destroyed
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Apply the generated mesh data to the ProceduralMeshComponent
	 * Called on the game thread once generation is complete
	 * @param MeshData - The generated mesh data
	 */
	void ApplyMeshData(const FTerrainMeshData& MeshData);

public:
	// ========================================
	// Configuration Properties
	// ========================================

	/** Size of the chunk in world units (one side of the square) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float ChunkSize;

	/** Number of vertices per side (higher = more detailed but more expensive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "2", ClampMax = "500"))
	int32 VerticesPerSide;

	/** Scale factor for noise sampling (smaller = more stretched/smooth terrain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
	float NoiseScale;

	/** Multiplier for height values (controls terrain elevation range) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float ZMultiplier;

	/** Number of noise octaves for fractal detail */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "1", ClampMax = "8"))
	int32 NoiseOctaves;

	/** Base frequency for noise generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "0.001", ClampMax = "0.1"))
	float NoiseFrequency;

	/** Frequency multiplier for each octave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "1.0", ClampMax = "4.0"))
	float NoiseLacunarity;

	/** Amplitude decay for each octave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NoisePersistence;

	/** Enable collision for this chunk */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Collision")
	bool bEnableCollision;

	/** Generate collision asynchronously (recommended for large chunks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Collision")
	bool bAsyncCollision;

protected:
	// ========================================
	// Components
	// ========================================

	/** Procedural mesh component that renders the terrain */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	UProceduralMeshComponent* ProceduralMesh;

	// ========================================
	// Runtime State
	// ========================================

	/** X coordinate in the chunk grid */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Terrain|State")
	int32 ChunkX;

	/** Y coordinate in the chunk grid */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Terrain|State")
	int32 ChunkY;

	/** Random seed for procedural generation */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Terrain|State")
	int32 Seed;

	/** Current LOD level (0 = highest detail) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Terrain|State")
	int32 CurrentLODLevel;

	/** Whether the mesh has been generated and applied */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Terrain|State")
	bool bIsGenerated;

	/** Async task for terrain generation (nullptr when not generating) */
	FAsyncTask<FTerrainGenerationTask>* GenerationTask;

	/** Mesh section index (always 0 for single-section chunks) */
	static constexpr int32 MeshSectionIndex = 0;
};
