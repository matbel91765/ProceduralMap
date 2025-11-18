// Ultimate Procedural Map Generation System - Cave Generator Actor
// Copyright (C) 2025. All Rights Reserved.

#include "CaveGenerator.h"
#include "CaveMeshGenerator.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ACaveGenerator::ACaveGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// Create procedural mesh component
	CaveMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CaveMesh"));
	CaveMesh->SetupAttachment(RootComponent);
	CaveMesh->bUseAsyncCooking = true;

	// Create cellular automata
	CellularAutomata = CreateDefaultSubobject<UCellularAutomata>(TEXT("CellularAutomata"));

	// Default settings
	MeshMode = ECaveMeshMode::MarchingSquares2D;
	bGenerateOnBeginPlay = true;
	VoxelSize = 100.0f;
	WallHeight = 400.0f;
	bGenerateCollision = true;

	// Stalactite/Stalagmite settings
	bGenerateStalactites = true;
	StalactiteChance = 0.1f;
	StalactiteMinHeight = 50.0f;
	StalactiteMaxHeight = 200.0f;

	bGenerateStalagmites = true;
	StalagmiteChance = 0.1f;
	StalagmiteMinHeight = 50.0f;
	StalagmiteMaxHeight = 150.0f;

	bSpawnResources = false;
}

void ACaveGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
	{
		GenerateCave();
	}
}

void ACaveGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACaveGenerator::GenerateCave()
{
	if (!CellularAutomata)
	{
		UE_LOG(LogTemp, Error, TEXT("CellularAutomata is null!"));
		return;
	}

	// Clear previous cave
	ClearCave();

	// Initialize random stream
	RandomStream.Initialize(CellularAutomata->Seed);

	// Generate cellular automata map
	if (MeshMode == ECaveMeshMode::MarchingCubes3D)
	{
		CellularAutomata->GenerateCaveMap3D();
	}
	else
	{
		CellularAutomata->GenerateCaveMap2D();
	}

	// Generate mesh
	GenerateCaveMesh();

	// Generate decorations
	if (bGenerateStalactites || bGenerateStalagmites)
	{
		GenerateSpeleothems();
	}

	// Spawn resources
	if (bSpawnResources)
	{
		SpawnResources();
	}

	UE_LOG(LogTemp, Log, TEXT("Cave generation complete!"));
}

void ACaveGenerator::GenerateCaveMesh()
{
	switch (MeshMode)
	{
		case ECaveMeshMode::MarchingSquares2D:
			GenerateMarchingSquares2D();
			break;

		case ECaveMeshMode::MarchingCubes3D:
			GenerateMarchingCubes3D();
			break;

		case ECaveMeshMode::SimpleBlocks:
			GenerateSimpleBlockMesh();
			break;
	}
}

void ACaveGenerator::GenerateMarchingSquares2D()
{
	if (!CellularAutomata || CellularAutomata->CaveMap2D.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No 2D cave map data available!"));
		return;
	}

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Use CaveMeshGenerator to create marching squares mesh
	UCaveMeshGenerator::GenerateMarchingSquaresMesh(
		CellularAutomata->CaveMap2D,
		CellularAutomata->MapWidth,
		CellularAutomata->MapHeight,
		VoxelSize,
		WallHeight,
		Vertices,
		Triangles,
		Normals,
		UVs
	);

	// Calculate smooth normals
	CalculateSmoothNormals(Vertices, Triangles, Normals);

	// Create mesh section
	CaveMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, bGenerateCollision);

	// Apply material
	if (CaveMaterial)
	{
		CaveMesh->SetMaterial(0, CaveMaterial);
	}

	UE_LOG(LogTemp, Log, TEXT("Generated Marching Squares mesh: %d vertices, %d triangles"),
		Vertices.Num(), Triangles.Num() / 3);
}

void ACaveGenerator::GenerateMarchingCubes3D()
{
	if (!CellularAutomata || CellularAutomata->CaveMap3D.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No 3D cave map data available!"));
		return;
	}

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Use CaveMeshGenerator to create marching cubes mesh
	UCaveMeshGenerator::GenerateMarchingCubesMesh(
		CellularAutomata->CaveMap3D,
		CellularAutomata->MapWidth,
		CellularAutomata->MapHeight,
		CellularAutomata->MapDepth,
		VoxelSize,
		Vertices,
		Triangles,
		Normals,
		UVs
	);

	// Calculate smooth normals
	CalculateSmoothNormals(Vertices, Triangles, Normals);

	// Create mesh section
	CaveMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, bGenerateCollision);

	// Apply material
	if (CaveMaterial)
	{
		CaveMesh->SetMaterial(0, CaveMaterial);
	}

	UE_LOG(LogTemp, Log, TEXT("Generated Marching Cubes mesh: %d vertices, %d triangles"),
		Vertices.Num(), Triangles.Num() / 3);
}

void ACaveGenerator::GenerateSimpleBlockMesh()
{
	if (!CellularAutomata)
	{
		return;
	}

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	if (MeshMode == ECaveMeshMode::SimpleBlocks && CellularAutomata->CaveMap2D.Num() > 0)
	{
		// Generate simple block mesh from 2D data
		for (int32 X = 0; X < CellularAutomata->MapWidth; X++)
		{
			for (int32 Y = 0; Y < CellularAutomata->MapHeight; Y++)
			{
				if (CellularAutomata->GetTile2D(X, Y) == 1) // Wall
				{
					FVector Position = GetWorldPosition(X, Y, 0);
					UCaveMeshGenerator::AddCube(Position, VoxelSize, WallHeight, Vertices, Triangles, Normals, UVs);
				}
			}
		}
	}

	// Create mesh section
	if (Vertices.Num() > 0)
	{
		CaveMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, bGenerateCollision);

		if (CaveMaterial)
		{
			CaveMesh->SetMaterial(0, CaveMaterial);
		}

		UE_LOG(LogTemp, Log, TEXT("Generated Simple Block mesh: %d vertices, %d triangles"),
			Vertices.Num(), Triangles.Num() / 3);
	}
}

void ACaveGenerator::GenerateSpeleothems()
{
	if (!CellularAutomata || CellularAutomata->CaveMap2D.Num() == 0)
	{
		return;
	}

	// Generate stalactites and stalagmites for ceiling/floor tiles
	for (int32 X = 1; X < CellularAutomata->MapWidth - 1; X++)
	{
		for (int32 Y = 1; Y < CellularAutomata->MapHeight - 1; Y++)
		{
			if (CellularAutomata->GetTile2D(X, Y) == 0) // Empty space
			{
				// Check for ceiling (wall above)
				if (bGenerateStalactites && CellularAutomata->GetTile2D(X, Y - 1) == 1)
				{
					if (RandomStream.FRand() < StalactiteChance)
					{
						FVector Position = GetWorldPosition(X, Y, 0);
						Position.Z += WallHeight / 2.0f;
						float Height = RandomStream.FRandRange(StalactiteMinHeight, StalactiteMaxHeight);
						CreateStalactite(Position, Height);
					}
				}

				// Check for floor (wall below)
				if (bGenerateStalagmites && CellularAutomata->GetTile2D(X, Y + 1) == 1)
				{
					if (RandomStream.FRand() < StalagmiteChance)
					{
						FVector Position = GetWorldPosition(X, Y, 0);
						Position.Z -= WallHeight / 2.0f;
						float Height = RandomStream.FRandRange(StalagmiteMinHeight, StalagmiteMaxHeight);
						CreateStalagmite(Position, Height);
					}
				}
			}
		}
	}
}

void ACaveGenerator::CreateStalactite(FVector Position, float Height)
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;

	// Create cone mesh pointing down
	UCaveMeshGenerator::CreateCone(Position, FVector(0, 0, -1), Height, VoxelSize * 0.3f, 8, Vertices, Triangles, Normals, UVs);

	// Add to procedural mesh as new section
	int32 SectionIndex = CaveMesh->GetNumSections();
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	CaveMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, false);

	if (CaveMaterial)
	{
		CaveMesh->SetMaterial(SectionIndex, CaveMaterial);
	}
}

void ACaveGenerator::CreateStalagmite(FVector Position, float Height)
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;

	// Create cone mesh pointing up
	UCaveMeshGenerator::CreateCone(Position, FVector(0, 0, 1), Height, VoxelSize * 0.3f, 8, Vertices, Triangles, Normals, UVs);

	// Add to procedural mesh as new section
	int32 SectionIndex = CaveMesh->GetNumSections();
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	CaveMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, false);

	if (CaveMaterial)
	{
		CaveMesh->SetMaterial(SectionIndex, CaveMaterial);
	}
}

void ACaveGenerator::SpawnResources()
{
	if (!CellularAutomata || ResourceTypes.Num() == 0)
	{
		return;
	}

	// For each room, spawn resources
	for (const FRoom& Room : CellularAutomata->SurvivingRooms)
	{
		for (const FResourcePlacement& ResourceType : ResourceTypes)
		{
			if (!ResourceType.ResourceClass || RandomStream.FRand() > ResourceType.SpawnChance)
			{
				continue;
			}

			int32 NumToSpawn = RandomStream.RandRange(ResourceType.MinPerRoom, ResourceType.MaxPerRoom);

			for (int32 i = 0; i < NumToSpawn; i++)
			{
				// Try to find valid spawn location
				int32 MaxAttempts = 10;
				for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
				{
					// Pick random tile in room
					int32 RandomTileIndex = RandomStream.RandRange(0, Room.Tiles.Num() - 1);
					FCoord3D Tile = Room.Tiles[RandomTileIndex];

					FVector SpawnLocation = GetWorldPosition(Tile.X, Tile.Y, Tile.Z);
					SpawnLocation.Z -= WallHeight / 2.0f + 50.0f; // Place on floor

					if (IsValidSpawnLocation(SpawnLocation, ResourceType.MinDistanceBetween))
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.Owner = this;
						AActor* SpawnedResource = GetWorld()->SpawnActor<AActor>(
							ResourceType.ResourceClass,
							SpawnLocation,
							FRotator::ZeroRotator,
							SpawnParams
						);

						if (SpawnedResource)
						{
							SpawnedDecorations.Add(SpawnedResource);
						}
						break;
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d resources in cave"), SpawnedDecorations.Num());
}

bool ACaveGenerator::IsValidSpawnLocation(FVector Location, float MinDistance) const
{
	for (const AActor* Decoration : SpawnedDecorations)
	{
		if (Decoration)
		{
			float Distance = FVector::Dist(Location, Decoration->GetActorLocation());
			if (Distance < MinDistance)
			{
				return false;
			}
		}
	}
	return true;
}

void ACaveGenerator::ClearCave()
{
	// Clear mesh
	CaveMesh->ClearAllMeshSections();

	// Destroy spawned decorations
	for (AActor* Decoration : SpawnedDecorations)
	{
		if (Decoration && !Decoration->IsPendingKill())
		{
			Decoration->Destroy();
		}
	}
	SpawnedDecorations.Empty();
}

FVector ACaveGenerator::GetWorldPosition(int32 X, int32 Y, int32 Z) const
{
	return FVector(
		X * VoxelSize,
		Y * VoxelSize,
		Z * VoxelSize
	);
}

FIntVector ACaveGenerator::GetGridPosition(FVector WorldPosition) const
{
	return FIntVector(
		FMath::RoundToInt(WorldPosition.X / VoxelSize),
		FMath::RoundToInt(WorldPosition.Y / VoxelSize),
		FMath::RoundToInt(WorldPosition.Z / VoxelSize)
	);
}

void ACaveGenerator::CalculateSmoothNormals(TArray<FVector>& Vertices, const TArray<int32>& Triangles, TArray<FVector>& Normals)
{
	if (Vertices.Num() == 0)
	{
		return;
	}

	// Initialize normals to zero
	Normals.SetNum(Vertices.Num());
	for (int32 i = 0; i < Normals.Num(); i++)
	{
		Normals[i] = FVector::ZeroVector;
	}

	// Accumulate face normals
	for (int32 i = 0; i < Triangles.Num(); i += 3)
	{
		int32 Index0 = Triangles[i];
		int32 Index1 = Triangles[i + 1];
		int32 Index2 = Triangles[i + 2];

		FVector V0 = Vertices[Index0];
		FVector V1 = Vertices[Index1];
		FVector V2 = Vertices[Index2];

		FVector Edge1 = V1 - V0;
		FVector Edge2 = V2 - V0;
		FVector FaceNormal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

		Normals[Index0] += FaceNormal;
		Normals[Index1] += FaceNormal;
		Normals[Index2] += FaceNormal;
	}

	// Normalize
	for (int32 i = 0; i < Normals.Num(); i++)
	{
		Normals[i] = Normals[i].GetSafeNormal();
	}
}

void ACaveGenerator::GenerateUVs(const TArray<FVector>& Vertices, TArray<FVector2D>& UVs)
{
	UVs.SetNum(Vertices.Num());

	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		UVs[i] = FVector2D(
			Vertices[i].X / VoxelSize,
			Vertices[i].Y / VoxelSize
		);
	}
}
