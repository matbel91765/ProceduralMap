// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ChunkManager.h"
#include "TerrainChunk.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AChunkManager::AChunkManager()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Default configuration
	ChunkSize = 1000.0f;
	VerticesPerSide = 100;
	ViewDistance = 5;
	UpdateInterval = 0.1f;
	MaxChunksPerUpdate = 4;
	MaxChunksToUnloadPerUpdate = 4;
	Seed = 1337;

	// Noise parameters
	NoiseScale = 0.001f;
	ZMultiplier = 500.0f;
	NoiseOctaves = 4;
	NoiseFrequency = 0.01f;
	NoiseLacunarity = 2.0f;
	NoisePersistence = 0.5f;

	// LOD settings
	bEnableLOD = true;
	LODDistances = {3, 6, 10}; // LOD0 within 3 chunks, LOD1 within 6, LOD2 within 10, etc.

	// Performance settings
	bEnableChunkPooling = true;
	MaxPoolSize = 50;
	bEnableCollision = true;

	// Management settings
	TrackedActor = nullptr;
	bAutoStart = true;

	// Runtime state
	UpdateTimer = 0.0f;
	bIsManaging = false;
	LastPlayerChunkCoord = FChunkCoordinate(INT_MAX, INT_MAX);

	// Set chunk class
	ChunkClass = ATerrainChunk::StaticClass();
}

void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	// Auto-start if enabled
	if (bAutoStart)
	{
		StartChunkManagement();
	}
}

void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only update if management is active
	if (!bIsManaging)
	{
		return;
	}

	// Accumulate time
	UpdateTimer += DeltaTime;

	// Check if it's time to update
	if (UpdateInterval <= 0.0f || UpdateTimer >= UpdateInterval)
	{
		UpdateChunks();
		UpdateTimer = 0.0f;
	}
}

void AChunkManager::StartChunkManagement()
{
	if (bIsManaging)
	{
		return;
	}

	bIsManaging = true;
	LastPlayerChunkCoord = FChunkCoordinate(INT_MAX, INT_MAX); // Force initial update
	UpdateTimer = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("ChunkManager started - ViewDistance: %d, ChunkSize: %.0f, UpdateInterval: %.2f"),
		ViewDistance, ChunkSize, UpdateInterval);

	// Do initial update
	UpdateChunks();
}

void AChunkManager::StopChunkManagement()
{
	bIsManaging = false;
	UE_LOG(LogTemp, Log, TEXT("ChunkManager stopped"));
}

void AChunkManager::ClearAllChunks()
{
	UE_LOG(LogTemp, Log, TEXT("ChunkManager clearing all chunks (%d active)"), ActiveChunks.Num());

	// Destroy all active chunks
	TArray<ATerrainChunk*> ChunksToDestroy;
	ActiveChunks.GenerateValueArray(ChunksToDestroy);

	for (ATerrainChunk* Chunk : ChunksToDestroy)
	{
		if (Chunk)
		{
			Chunk->Destroy();
		}
	}

	ActiveChunks.Empty();

	// Destroy all pooled chunks
	for (ATerrainChunk* Chunk : ChunkPool)
	{
		if (Chunk)
		{
			Chunk->Destroy();
		}
	}

	ChunkPool.Empty();

	// Reset state
	LastPlayerChunkCoord = FChunkCoordinate(INT_MAX, INT_MAX);
}

void AChunkManager::UpdateChunks()
{
	// Find the tracked actor (player)
	AActor* TrackedActorPtr = TrackedActor;
	if (!TrackedActorPtr)
	{
		TrackedActorPtr = FindPlayerPawn();
		if (!TrackedActorPtr)
		{
			// No player found, can't update chunks
			return;
		}
	}

	// Get player position and convert to chunk coordinates
	const FVector PlayerPosition = TrackedActorPtr->GetActorLocation();
	const FChunkCoordinate PlayerChunkCoord = WorldPositionToChunkCoordinate(PlayerPosition);

	// Check if player moved to a different chunk
	const bool bPlayerMovedChunks = (PlayerChunkCoord != LastPlayerChunkCoord);

	// Get all chunks that should be loaded
	TArray<FChunkCoordinate> ChunksToLoad;
	GetChunksInRange(PlayerChunkCoord, ChunksToLoad);

	// Find chunks that need to be loaded (not currently active)
	TArray<FChunkCoordinate> NewChunksToLoad;
	for (const FChunkCoordinate& Coord : ChunksToLoad)
	{
		if (!ActiveChunks.Contains(Coord))
		{
			NewChunksToLoad.Add(Coord);
		}
	}

	// Find chunks that need to be unloaded (active but out of range)
	TArray<FChunkCoordinate> ChunksToUnload;
	for (const auto& Pair : ActiveChunks)
	{
		if (!ChunksToLoad.Contains(Pair.Key))
		{
			ChunksToUnload.Add(Pair.Key);
		}
	}

	// Limit number of chunks to load/unload per frame to avoid stuttering
	const int32 ChunksToLoadThisFrame = FMath::Min(NewChunksToLoad.Num(), MaxChunksPerUpdate);
	const int32 ChunksToUnloadThisFrame = FMath::Min(ChunksToUnload.Num(), MaxChunksToUnloadPerUpdate);

	// Load new chunks
	for (int32 i = 0; i < ChunksToLoadThisFrame; i++)
	{
		LoadChunk(NewChunksToLoad[i]);
	}

	// Unload distant chunks
	for (int32 i = 0; i < ChunksToUnloadThisFrame; i++)
	{
		UnloadChunk(ChunksToUnload[i]);
	}

	// Update LOD for all active chunks if player moved or LOD is enabled
	if (bEnableLOD && (bPlayerMovedChunks || NewChunksToLoad.Num() > 0))
	{
		UpdateChunkLODs();
	}

	// Update last player position
	LastPlayerChunkCoord = PlayerChunkCoord;

#if !UE_BUILD_SHIPPING
	// Debug logging
	if (NewChunksToLoad.Num() > 0 || ChunksToUnload.Num() > 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ChunkManager Update - Player at %s, Active: %d, Loaded: %d, Unloaded: %d"),
			*PlayerChunkCoord.ToString(), ActiveChunks.Num(), ChunksToLoadThisFrame, ChunksToUnloadThisFrame);
	}
#endif
}

ATerrainChunk* AChunkManager::LoadChunk(FChunkCoordinate ChunkCoord)
{
	// Check if already loaded
	if (ActiveChunks.Contains(ChunkCoord))
	{
		return ActiveChunks[ChunkCoord];
	}

	// Get chunk from pool or spawn new one
	ATerrainChunk* Chunk = GetChunkFromPool();
	if (!Chunk)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get chunk from pool for %s"), *ChunkCoord.ToString());
		return nullptr;
	}

	// Calculate LOD for this chunk
	const FChunkCoordinate PlayerChunkCoord = WorldPositionToChunkCoordinate(
		TrackedActor ? TrackedActor->GetActorLocation() : FindPlayerPawn()->GetActorLocation()
	);
	const int32 LODLevel = CalculateLODForChunk(ChunkCoord, PlayerChunkCoord);

	// Configure chunk properties
	Chunk->ChunkSize = ChunkSize;
	Chunk->VerticesPerSide = VerticesPerSide;
	Chunk->NoiseScale = NoiseScale;
	Chunk->ZMultiplier = ZMultiplier;
	Chunk->NoiseOctaves = NoiseOctaves;
	Chunk->NoiseFrequency = NoiseFrequency;
	Chunk->NoiseLacunarity = NoiseLacunarity;
	Chunk->NoisePersistence = NoisePersistence;
	Chunk->bEnableCollision = bEnableCollision;

	// Apply material if set
	if (TerrainMaterial)
	{
		Chunk->SetTerrainMaterial(TerrainMaterial);
	}

	// Initialize and start generation
	Chunk->Initialize(ChunkCoord.X, ChunkCoord.Y, Seed, LODLevel);
	Chunk->StartGeneration();

	// Add to active chunks
	ActiveChunks.Add(ChunkCoord, Chunk);

	return Chunk;
}

void AChunkManager::UnloadChunk(FChunkCoordinate ChunkCoord)
{
	// Check if chunk exists
	ATerrainChunk** ChunkPtr = ActiveChunks.Find(ChunkCoord);
	if (!ChunkPtr || !*ChunkPtr)
	{
		return;
	}

	ATerrainChunk* Chunk = *ChunkPtr;

	// Remove from active chunks
	ActiveChunks.Remove(ChunkCoord);

	// Return to pool or destroy
	ReturnChunkToPool(Chunk);

#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Verbose, TEXT("Unloaded chunk %s"), *ChunkCoord.ToString());
#endif
}

ATerrainChunk* AChunkManager::GetChunkAt(FChunkCoordinate ChunkCoord) const
{
	const ATerrainChunk* const* ChunkPtr = ActiveChunks.Find(ChunkCoord);
	return ChunkPtr ? *ChunkPtr : nullptr;
}

bool AChunkManager::IsChunkLoaded(FChunkCoordinate ChunkCoord) const
{
	return ActiveChunks.Contains(ChunkCoord);
}

FChunkCoordinate AChunkManager::WorldPositionToChunkCoordinate(FVector WorldPosition) const
{
	// Convert world position to chunk coordinates
	// Use floor division to ensure consistent chunk assignment
	const int32 ChunkX = FMath::FloorToInt(WorldPosition.X / ChunkSize);
	const int32 ChunkY = FMath::FloorToInt(WorldPosition.Y / ChunkSize);

	return FChunkCoordinate(ChunkX, ChunkY);
}

FVector AChunkManager::ChunkCoordinateToWorldPosition(FChunkCoordinate ChunkCoord) const
{
	return FVector(
		ChunkCoord.X * ChunkSize,
		ChunkCoord.Y * ChunkSize,
		0.0f
	);
}

void AChunkManager::SetTerrainMaterial(UMaterialInterface* Material)
{
	TerrainMaterial = Material;

	// Apply to all active chunks
	for (const auto& Pair : ActiveChunks)
	{
		if (Pair.Value)
		{
			Pair.Value->SetTerrainMaterial(Material);
		}
	}
}

void AChunkManager::SetTrackedActor(AActor* NewTrackedActor)
{
	TrackedActor = NewTrackedActor;
	LastPlayerChunkCoord = FChunkCoordinate(INT_MAX, INT_MAX); // Force update on next tick
}

void AChunkManager::UpdateChunkLODs()
{
	if (!bEnableLOD)
	{
		return;
	}

	// Get player chunk coordinate
	AActor* TrackedActorPtr = TrackedActor ? TrackedActor : FindPlayerPawn();
	if (!TrackedActorPtr)
	{
		return;
	}

	const FVector PlayerPosition = TrackedActorPtr->GetActorLocation();
	const FChunkCoordinate PlayerChunkCoord = WorldPositionToChunkCoordinate(PlayerPosition);

	// Update LOD for each active chunk
	for (const auto& Pair : ActiveChunks)
	{
		ATerrainChunk* Chunk = Pair.Value;
		if (!Chunk)
		{
			continue;
		}

		const int32 TargetLOD = CalculateLODForChunk(Pair.Key, PlayerChunkCoord);
		const int32 CurrentLOD = Chunk->GetLODLevel();

		// Only update if LOD changed
		if (TargetLOD != CurrentLOD)
		{
			Chunk->SetLODLevel(TargetLOD);
		}
	}
}

void AChunkManager::GetChunksInRange(FChunkCoordinate CenterCoord, TArray<FChunkCoordinate>& OutChunks) const
{
	OutChunks.Empty();

	// Generate all chunks within ViewDistance
	for (int32 Y = -ViewDistance; Y <= ViewDistance; Y++)
	{
		for (int32 X = -ViewDistance; X <= ViewDistance; X++)
		{
			// Optional: Use circular loading pattern instead of square
			// const int32 DistSquared = X * X + Y * Y;
			// if (DistSquared > ViewDistance * ViewDistance)
			// {
			//     continue;
			// }

			OutChunks.Add(FChunkCoordinate(
				CenterCoord.X + X,
				CenterCoord.Y + Y
			));
		}
	}
}

int32 AChunkManager::CalculateLODForChunk(FChunkCoordinate ChunkCoord, FChunkCoordinate PlayerCoord) const
{
	if (!bEnableLOD || LODDistances.Num() == 0)
	{
		return 0;
	}

	// Calculate Manhattan distance (could also use Euclidean)
	const int32 DistX = FMath::Abs(ChunkCoord.X - PlayerCoord.X);
	const int32 DistY = FMath::Abs(ChunkCoord.Y - PlayerCoord.Y);
	const int32 Distance = FMath::Max(DistX, DistY); // Chebyshev distance
	// Alternative: const int32 Distance = DistX + DistY; // Manhattan distance

	// Find appropriate LOD level based on distance
	for (int32 i = 0; i < LODDistances.Num(); i++)
	{
		if (Distance < LODDistances[i])
		{
			return i;
		}
	}

	// If beyond all thresholds, use highest LOD level
	return LODDistances.Num();
}

ATerrainChunk* AChunkManager::GetChunkFromPool()
{
	ATerrainChunk* Chunk = nullptr;

	// Try to get from pool first
	if (bEnableChunkPooling && ChunkPool.Num() > 0)
	{
		Chunk = ChunkPool.Pop();
		Chunk->SetActorHiddenInGame(false);
		Chunk->SetActorEnableCollision(bEnableCollision);
		Chunk->SetActorTickEnabled(true);
	}
	else
	{
		// Spawn new chunk
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("ChunkManager::GetChunkFromPool - World is null"));
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		Chunk = World->SpawnActor<ATerrainChunk>(ChunkClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (!Chunk)
		{
			UE_LOG(LogTemp, Error, TEXT("ChunkManager::GetChunkFromPool - Failed to spawn chunk"));
			return nullptr;
		}
	}

	return Chunk;
}

void AChunkManager::ReturnChunkToPool(ATerrainChunk* Chunk)
{
	if (!Chunk)
	{
		return;
	}

	// Clear the chunk
	Chunk->ClearMesh();

	if (bEnableChunkPooling && ChunkPool.Num() < MaxPoolSize)
	{
		// Add to pool
		Chunk->SetActorHiddenInGame(true);
		Chunk->SetActorEnableCollision(false);
		Chunk->SetActorTickEnabled(false);
		ChunkPool.Add(Chunk);
	}
	else
	{
		// Destroy if pool is full or pooling is disabled
		Chunk->Destroy();
	}
}

AActor* AChunkManager::FindPlayerPawn() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Try to get the first player controller's pawn
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC)
	{
		APawn* Pawn = PC->GetPawn();
		if (Pawn)
		{
			return Pawn;
		}
	}

	// Fallback: try to get player 0's pawn directly
	return UGameplayStatics::GetPlayerPawn(World, 0);
}
