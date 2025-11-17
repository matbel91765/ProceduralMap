// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "TerrainChunk.h"
#include "ProceduralMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATerrainChunk::ATerrainChunk()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create the procedural mesh component
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;

	// Default generation parameters (can be overridden in editor or at runtime)
	ChunkSize = 1000.0f;
	VerticesPerSide = 100;
	NoiseScale = 0.001f;
	ZMultiplier = 500.0f;
	NoiseOctaves = 4;
	NoiseFrequency = 0.01f;
	NoiseLacunarity = 2.0f;
	NoisePersistence = 0.5f;

	// Collision settings
	bEnableCollision = true;
	bAsyncCollision = true;

	// Runtime state
	ChunkX = 0;
	ChunkY = 0;
	Seed = 1337;
	CurrentLODLevel = 0;
	bIsGenerated = false;
	GenerationTask = nullptr;

	// Configure procedural mesh component
	if (ProceduralMesh)
	{
		ProceduralMesh->bUseAsyncCooking = true;
		ProceduralMesh->SetCastShadow(true);
		ProceduralMesh->bCastDynamicShadow = true;
	}
}

void ATerrainChunk::BeginPlay()
{
	Super::BeginPlay();
}

void ATerrainChunk::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up generation task if still running
	if (GenerationTask)
	{
		GenerationTask->EnsureCompletion();
		delete GenerationTask;
		GenerationTask = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ATerrainChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if generation task is complete and apply mesh if ready
	if (GenerationTask && !bIsGenerated)
	{
		UpdateGeneration();
	}
}

void ATerrainChunk::Initialize(int32 InChunkX, int32 InChunkY, int32 InSeed, int32 InLODLevel)
{
	// Clean up any existing generation
	if (GenerationTask)
	{
		GenerationTask->EnsureCompletion();
		delete GenerationTask;
		GenerationTask = nullptr;
	}

	// Set chunk coordinates and parameters
	ChunkX = InChunkX;
	ChunkY = InChunkY;
	Seed = InSeed;
	CurrentLODLevel = FMath::Max(0, InLODLevel);
	bIsGenerated = false;

	// Update actor world position
	const FVector WorldPosition = GetChunkWorldPosition();
	SetActorLocation(WorldPosition);

	// Clear any existing mesh
	if (ProceduralMesh)
	{
		ProceduralMesh->ClearAllMeshSections();
	}

	// Log chunk initialization (can be disabled in shipping builds)
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Verbose, TEXT("TerrainChunk initialized at (%d, %d) - World Pos: %s, LOD: %d"),
		ChunkX, ChunkY, *WorldPosition.ToString(), CurrentLODLevel);
#endif
}

void ATerrainChunk::StartGeneration()
{
	// Don't start if already generating or already generated
	if (GenerationTask || bIsGenerated)
	{
		return;
	}

	// Prepare generation parameters
	FTerrainGenerationParams Params;
	Params.ChunkSize = ChunkSize;
	Params.VerticesPerSide = VerticesPerSide;
	Params.NoiseScale = NoiseScale;
	Params.ZMultiplier = ZMultiplier;
	Params.ChunkWorldPosition = GetChunkWorldPosition();
	Params.Seed = Seed;
	Params.LODLevel = CurrentLODLevel;
	Params.Octaves = NoiseOctaves;
	Params.Frequency = NoiseFrequency;
	Params.Lacunarity = NoiseLacunarity;
	Params.Persistence = NoisePersistence;

	// Create and start the async generation task
	GenerationTask = new FAsyncTask<FTerrainGenerationTask>(Params);
	GenerationTask->StartBackgroundTask();

#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Verbose, TEXT("TerrainChunk (%d, %d) started generation task (LOD %d)"),
		ChunkX, ChunkY, CurrentLODLevel);
#endif
}

bool ATerrainChunk::UpdateGeneration()
{
	// Check if we have a task and it's complete
	if (!GenerationTask || !GenerationTask->IsDone())
	{
		return false;
	}

	// Ensure task is fully complete
	GenerationTask->EnsureCompletion();

	// Get the generated mesh data
	FTerrainMeshData& MeshData = GenerationTask->GetTask().GetMeshData();

	// Apply the mesh data to the procedural mesh component
	ApplyMeshData(MeshData);

	// Clean up the task
	delete GenerationTask;
	GenerationTask = nullptr;

	// Mark as generated
	bIsGenerated = true;

#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Verbose, TEXT("TerrainChunk (%d, %d) mesh generation complete - %d vertices, %d triangles"),
		ChunkX, ChunkY, MeshData.Vertices.Num(), MeshData.Triangles.Num() / 3);
#endif

	return true;
}

void ATerrainChunk::SetLODLevel(int32 NewLODLevel)
{
	// Clamp LOD level to valid range
	NewLODLevel = FMath::Max(0, NewLODLevel);

	// Only regenerate if LOD actually changed
	if (NewLODLevel != CurrentLODLevel)
	{
		CurrentLODLevel = NewLODLevel;

		// If already generating, cancel and restart with new LOD
		if (GenerationTask)
		{
			GenerationTask->EnsureCompletion();
			delete GenerationTask;
			GenerationTask = nullptr;
		}

		// Clear current mesh and regenerate
		bIsGenerated = false;
		ClearMesh();
		StartGeneration();

#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Verbose, TEXT("TerrainChunk (%d, %d) LOD changed to %d, regenerating..."),
			ChunkX, ChunkY, CurrentLODLevel);
#endif
	}
}

void ATerrainChunk::SetTerrainMaterial(UMaterialInterface* Material)
{
	if (ProceduralMesh && Material)
	{
		ProceduralMesh->SetMaterial(MeshSectionIndex, Material);
	}
}

void ATerrainChunk::ClearMesh()
{
	if (ProceduralMesh)
	{
		ProceduralMesh->ClearAllMeshSections();
	}
	bIsGenerated = false;
}

void ATerrainChunk::ApplyMeshData(const FTerrainMeshData& MeshData)
{
	if (!ProceduralMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainChunk::ApplyMeshData - ProceduralMesh is null!"));
		return;
	}

	// Validate mesh data
	if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainChunk::ApplyMeshData - Empty mesh data for chunk (%d, %d)"),
			ChunkX, ChunkY);
		return;
	}

	// Clear existing sections
	ProceduralMesh->ClearAllMeshSections();

	// Create the mesh section with all the generated data
	ProceduralMesh->CreateMeshSection(
		MeshSectionIndex,                    // Section index
		MeshData.Vertices,                   // Vertices
		MeshData.Triangles,                  // Triangles
		MeshData.Normals,                    // Normals
		MeshData.UVs,                        // UV0
		MeshData.VertexColors,               // Vertex colors
		MeshData.Tangents,                   // Tangents
		bEnableCollision                     // Create collision
	);

	// Update collision settings
	if (bEnableCollision)
	{
		ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ProceduralMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		ProceduralMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		ProceduralMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	}
	else
	{
		ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Update bounds to ensure proper culling
	ProceduralMesh->UpdateBounds();
}
