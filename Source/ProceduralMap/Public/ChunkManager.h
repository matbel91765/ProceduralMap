// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainChunk.h"
#include "ChunkManager.generated.h"

/**
 * Forward declarations
 */
class ATerrainChunk;
class UMaterialInterface;

/**
 * Structure representing a chunk coordinate in the grid
 */
USTRUCT(BlueprintType)
struct FChunkCoordinate
{
	GENERATED_BODY()

	/** X coordinate in chunk grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
	int32 X;

	/** Y coordinate in chunk grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
	int32 Y;

	/** Default constructor */
	FChunkCoordinate()
		: X(0), Y(0)
	{
	}

	/** Constructor with coordinates */
	FChunkCoordinate(int32 InX, int32 InY)
		: X(InX), Y(InY)
	{
	}

	/** Equality operator for TMap usage */
	bool operator==(const FChunkCoordinate& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

	/** Inequality operator */
	bool operator!=(const FChunkCoordinate& Other) const
	{
		return !(*this == Other);
	}

	/** Hash function for TMap usage */
	friend uint32 GetTypeHash(const FChunkCoordinate& Coord)
	{
		return HashCombine(GetTypeHash(Coord.X), GetTypeHash(Coord.Y));
	}

	/** Convert to string for debugging */
	FString ToString() const
	{
		return FString::Printf(TEXT("(%d, %d)"), X, Y);
	}
};

/**
 * AChunkManager - Main manager for infinite terrain chunk system
 *
 * This actor manages the loading, unloading, and LOD of terrain chunks based on
 * player position. It creates an infinite terrain by:
 * - Tracking active chunks in a TMap
 * - Loading chunks within ViewDistance of the player
 * - Unloading chunks that are too far away
 * - Adjusting LOD based on distance
 * - Pooling chunks for reuse to avoid constant spawning/destruction
 *
 * The manager updates chunks based on the player's position (or a specified tracking actor).
 * Chunks are identified by integer coordinates and positioned in a grid pattern.
 *
 * Usage:
 * 1. Place ChunkManager in the level
 * 2. Set ViewDistance and other parameters
 * 3. Optionally set a PlayerPawn reference, or it will auto-detect
 * 4. Call StartChunkManagement() when ready to begin
 */
UCLASS()
class PROCEDURALMAP_API AChunkManager : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * Constructor
	 */
	AChunkManager();

	/**
	 * Called every frame
	 * @param DeltaTime - Time since last frame
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Start managing chunks
	 * Begins the update loop that loads/unloads chunks based on player position
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Management")
	void StartChunkManagement();

	/**
	 * Stop managing chunks
	 * Pauses chunk updates (existing chunks remain)
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Management")
	void StopChunkManagement();

	/**
	 * Clear all chunks
	 * Destroys all active chunks and clears the pool
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Management")
	void ClearAllChunks();

	/**
	 * Update chunks based on current player position
	 * Loads new chunks and unloads distant ones
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Management")
	void UpdateChunks();

	/**
	 * Load a specific chunk at the given coordinates
	 * @param ChunkCoord - Coordinates of the chunk to load
	 * @return The loaded chunk actor, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Management")
	ATerrainChunk* LoadChunk(FChunkCoordinate ChunkCoord);

	/**
	 * Unload a specific chunk at the given coordinates
	 * @param ChunkCoord - Coordinates of the chunk to unload
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Management")
	void UnloadChunk(FChunkCoordinate ChunkCoord);

	/**
	 * Get chunk at specific coordinates (if loaded)
	 * @param ChunkCoord - Coordinates to check
	 * @return Chunk actor if loaded, nullptr otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain|Management")
	ATerrainChunk* GetChunkAt(FChunkCoordinate ChunkCoord) const;

	/**
	 * Check if a chunk is loaded at the given coordinates
	 * @param ChunkCoord - Coordinates to check
	 * @return True if chunk is loaded
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain|Management")
	bool IsChunkLoaded(FChunkCoordinate ChunkCoord) const;

	/**
	 * Get number of currently active chunks
	 * @return Number of loaded chunks
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain|Management")
	int32 GetActiveChunkCount() const { return ActiveChunks.Num(); }

	/**
	 * Get chunk coordinates from world position
	 * @param WorldPosition - Position in world space
	 * @return Chunk coordinates containing this position
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain|Management")
	FChunkCoordinate WorldPositionToChunkCoordinate(FVector WorldPosition) const;

	/**
	 * Get world position from chunk coordinates
	 * @param ChunkCoord - Chunk coordinates
	 * @return World position of chunk origin
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain|Management")
	FVector ChunkCoordinateToWorldPosition(FChunkCoordinate ChunkCoord) const;

	/**
	 * Set the terrain material for all chunks
	 * @param Material - Material to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Appearance")
	void SetTerrainMaterial(UMaterialInterface* Material);

	/**
	 * Set the actor to track for chunk loading (usually the player pawn)
	 * @param NewTrackedActor - Actor to track
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Management")
	void SetTrackedActor(AActor* NewTrackedActor);

	/**
	 * Force update the LOD of all active chunks based on distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Management")
	void UpdateChunkLODs();

protected:
	/**
	 * Called when the game starts or when spawned
	 */
	virtual void BeginPlay() override;

	/**
	 * Get all chunk coordinates that should be loaded based on current player position
	 * @param CenterCoord - Center chunk coordinate (usually player position)
	 * @param OutChunks - Array to fill with chunk coordinates
	 */
	void GetChunksInRange(FChunkCoordinate CenterCoord, TArray<FChunkCoordinate>& OutChunks) const;

	/**
	 * Calculate LOD level for a chunk based on distance from player
	 * @param ChunkCoord - Coordinates of the chunk
	 * @param PlayerCoord - Coordinates of the player's chunk
	 * @return LOD level (0 = highest detail)
	 */
	int32 CalculateLODForChunk(FChunkCoordinate ChunkCoord, FChunkCoordinate PlayerCoord) const;

	/**
	 * Get or create a chunk actor from the pool
	 * @return Chunk actor ready to be initialized
	 */
	ATerrainChunk* GetChunkFromPool();

	/**
	 * Return a chunk to the pool for reuse
	 * @param Chunk - Chunk to return to pool
	 */
	void ReturnChunkToPool(ATerrainChunk* Chunk);

	/**
	 * Find the player pawn automatically if not set
	 * @return Player pawn, or nullptr if not found
	 */
	AActor* FindPlayerPawn() const;

public:
	// ========================================
	// Configuration Properties
	// ========================================

	/** Size of each chunk in world units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float ChunkSize;

	/** Number of vertices per side for each chunk */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "2", ClampMax = "500"))
	int32 VerticesPerSide;

	/** Distance from player within which chunks should be loaded (in chunk units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Management", meta = (ClampMin = "1", ClampMax = "50"))
	int32 ViewDistance;

	/** How often to update chunks (in seconds) - 0 = every frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Management", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float UpdateInterval;

	/** Maximum chunks to load per update to avoid stuttering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Management", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxChunksPerUpdate;

	/** Maximum chunks to unload per update to avoid stuttering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Management", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxChunksToUnloadPerUpdate;

	/** Random seed for terrain generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation")
	int32 Seed;

	/** Scale factor for noise sampling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
	float NoiseScale;

	/** Multiplier for height values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float ZMultiplier;

	/** Number of noise octaves */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "1", ClampMax = "8"))
	int32 NoiseOctaves;

	/** Base frequency for noise */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "0.001", ClampMax = "0.1"))
	float NoiseFrequency;

	/** Frequency multiplier for each octave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "1.0", ClampMax = "4.0"))
	float NoiseLacunarity;

	/** Amplitude decay for each octave */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NoisePersistence;

	/** Enable LOD system (distance-based level of detail) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|LOD")
	bool bEnableLOD;

	/** Distance thresholds for LOD transitions (in chunks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|LOD", meta = (EditCondition = "bEnableLOD"))
	TArray<int32> LODDistances;

	/** Enable chunk pooling for performance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Performance")
	bool bEnableChunkPooling;

	/** Maximum size of chunk pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Performance", meta = (ClampMin = "0", ClampMax = "100", EditCondition = "bEnableChunkPooling"))
	int32 MaxPoolSize;

	/** Enable collision on chunks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Collision")
	bool bEnableCollision;

	/** Material to apply to all terrain chunks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Appearance")
	UMaterialInterface* TerrainMaterial;

	/** Actor to track for chunk loading (usually player pawn, auto-detected if null) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Management")
	AActor* TrackedActor;

	/** Automatically start chunk management when play begins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Management")
	bool bAutoStart;

protected:
	// ========================================
	// Runtime State
	// ========================================

	/** Map of active chunks (key = chunk coordinate, value = chunk actor) */
	UPROPERTY()
	TMap<FChunkCoordinate, ATerrainChunk*> ActiveChunks;

	/** Pool of inactive chunks for reuse */
	UPROPERTY()
	TArray<ATerrainChunk*> ChunkPool;

	/** Last player chunk coordinate (to detect when player moves to new chunk) */
	FChunkCoordinate LastPlayerChunkCoord;

	/** Time accumulator for update interval */
	float UpdateTimer;

	/** Whether chunk management is active */
	bool bIsManaging;

	/** Class to use for spawning chunks */
	UPROPERTY()
	TSubclassOf<ATerrainChunk> ChunkClass;
};
