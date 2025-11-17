// Copyright Epic Games, Inc. All Rights Reserved.

#include "WFCGenerator.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"

AWFCGenerator::AWFCGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Default settings
	GenerationType = EWFCGenerationType::Village;
	TileSetAsset = nullptr;
	GridWidth = 10;
	GridHeight = 10;
	TileSize = 100.0f;
	Seed = 12345;
	bUseRandomSeed = false;

	bEnableBacktracking = true;
	MaxBacktrackDepth = 50;
	MaxIterations = 10000;
	bGenerateOnBeginPlay = true;
	bAutoRegenerateInEditor = false;

	bSpawnMeshes = true;
	bShowDebugGrid = true;
	DebugGridColor = FColor::Cyan;
	bShowTileIDs = true;
	MeshOffset = FVector::ZeroVector;

	bGenerationSuccess = false;
	LastIterationCount = 0;

	// Create WFC algorithm instance
	WFCAlgorithm = CreateDefaultSubobject<UWaveFunctionCollapse>(TEXT("WFCAlgorithm"));
}

void AWFCGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
	{
		Generate();
	}
}

void AWFCGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShowDebugGrid)
	{
		DrawDebugVisualization();
	}
}

#if WITH_EDITOR
void AWFCGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bAutoRegenerateInEditor && PropertyChangedEvent.Property != nullptr)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();

		// Regenerate on relevant property changes
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AWFCGenerator, GenerationType) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AWFCGenerator, GridWidth) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AWFCGenerator, GridHeight) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AWFCGenerator, TileSize) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AWFCGenerator, Seed) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AWFCGenerator, TileSetAsset))
		{
			Regenerate();
		}
	}
}
#endif

void AWFCGenerator::Generate()
{
	if (!WFCAlgorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCGenerator: WFC Algorithm is null!"));
		OnGenerationFailed.Broadcast();
		return;
	}

	OnGenerationStarted.Broadcast();

	// Clear previous generation
	Clear();

	// Initialize WFC
	InitializeWFC();

	// Run generation
	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Starting generation (%dx%d grid)"), GridWidth, GridHeight);

	bGenerationSuccess = WFCAlgorithm->Generate();

	if (bGenerationSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Generation successful!"));

		// Get statistics
		int32 TotalCells, CollapsedCells;
		float AverageEntropy;
		WFCAlgorithm->GetStatistics(TotalCells, CollapsedCells, AverageEntropy);

		UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Stats - Total: %d, Collapsed: %d, Avg Entropy: %.2f"),
			TotalCells, CollapsedCells, AverageEntropy);

		// Spawn meshes if enabled
		if (bSpawnMeshes)
		{
			SpawnMeshes();
		}

		OnGenerationCompleted.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WFCGenerator: Generation failed!"));
		OnGenerationFailed.Broadcast();
	}
}

void AWFCGenerator::Clear()
{
	ClearMeshes();
	bGenerationSuccess = false;
	LastIterationCount = 0;

	if (WFCAlgorithm)
	{
		WFCAlgorithm->ResetGrid();
	}
}

void AWFCGenerator::Regenerate()
{
	Clear();
	Generate();
}

void AWFCGenerator::GetGenerationStats(int32& OutIterations, int32& OutTotalCells, int32& OutCollapsedCells) const
{
	OutIterations = LastIterationCount;

	if (WFCAlgorithm)
	{
		float AverageEntropy;
		WFCAlgorithm->GetStatistics(OutTotalCells, OutCollapsedCells, AverageEntropy);
	}
	else
	{
		OutTotalCells = 0;
		OutCollapsedCells = 0;
	}
}

void AWFCGenerator::InitializeWFC()
{
	if (!WFCAlgorithm)
	{
		return;
	}

	// Apply settings
	WFCAlgorithm->GridWidth = GridWidth;
	WFCAlgorithm->GridHeight = GridHeight;
	WFCAlgorithm->TileSize = TileSize;
	WFCAlgorithm->RandomSeed = GetSeed();
	WFCAlgorithm->bEnableBacktracking = bEnableBacktracking;
	WFCAlgorithm->MaxBacktrackDepth = MaxBacktrackDepth;
	WFCAlgorithm->MaxIterations = MaxIterations;
	WFCAlgorithm->bGenerateMeshes = bSpawnMeshes;

	// Apply tile set
	if (TileSetAsset)
	{
		ApplyTileSetAsset();
	}
	else
	{
		ApplyGenerationType();
	}

	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Initialized with %d tiles, seed: %d"),
		WFCAlgorithm->TileSet.Num(), WFCAlgorithm->RandomSeed);
}

void AWFCGenerator::ApplyGenerationType()
{
	if (!WFCAlgorithm)
	{
		return;
	}

	switch (GenerationType)
	{
	case EWFCGenerationType::Village:
		WFCAlgorithm->TileSet = CreateVillageTileSet();
		break;

	case EWFCGenerationType::Dungeon:
		WFCAlgorithm->TileSet = CreateDungeonTileSet();
		break;

	case EWFCGenerationType::RoadSystem:
		WFCAlgorithm->TileSet = CreateRoadTileSet();
		break;

	case EWFCGenerationType::Building:
		WFCAlgorithm->TileSet = CreateBuildingTileSet();
		break;

	case EWFCGenerationType::Maze:
		WFCAlgorithm->TileSet = CreateMazeTileSet();
		break;

	case EWFCGenerationType::Custom:
	default:
		// Keep existing tile set or create empty one
		if (WFCAlgorithm->TileSet.Num() == 0)
		{
			WFCAlgorithm->TileSet = CreateVillageTileSet();
		}
		break;
	}
}

void AWFCGenerator::ApplyTileSetAsset()
{
	if (!WFCAlgorithm || !TileSetAsset)
	{
		return;
	}

	WFCAlgorithm->TileSet = TileSetAsset->Tiles;

	// Apply default settings from asset
	if (TileSetAsset->DefaultGridWidth > 0)
	{
		GridWidth = TileSetAsset->DefaultGridWidth;
		WFCAlgorithm->GridWidth = GridWidth;
	}

	if (TileSetAsset->DefaultGridHeight > 0)
	{
		GridHeight = TileSetAsset->DefaultGridHeight;
		WFCAlgorithm->GridHeight = GridHeight;
	}

	if (TileSetAsset->DefaultTileSize > 0)
	{
		TileSize = TileSetAsset->DefaultTileSize;
		WFCAlgorithm->TileSize = TileSize;
	}

	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Applied tile set '%s' with %d tiles"),
		*TileSetAsset->TileSetName, TileSetAsset->Tiles.Num());
}

void AWFCGenerator::SpawnMeshes()
{
	if (!WFCAlgorithm)
	{
		return;
	}

	ClearMeshes();

	FVector BaseLocation = GetActorLocation() + MeshOffset;
	GeneratedMeshes = WFCAlgorithm->GenerateMeshes(GetWorld(), BaseLocation, this);

	UE_LOG(LogTemp, Log, TEXT("WFCGenerator: Spawned %d meshes"), GeneratedMeshes.Num());
}

void AWFCGenerator::DrawDebugVisualization()
{
	if (!WFCAlgorithm || !GetWorld())
	{
		return;
	}

	FVector BaseLocation = GetActorLocation();

	// Draw grid
	for (int32 Y = 0; Y <= GridHeight; Y++)
	{
		FVector Start = BaseLocation + FVector(0, Y * TileSize, 0);
		FVector End = BaseLocation + FVector(GridWidth * TileSize, Y * TileSize, 0);
		DrawDebugLine(GetWorld(), Start, End, DebugGridColor, false, -1.0f, 0, 2.0f);
	}

	for (int32 X = 0; X <= GridWidth; X++)
	{
		FVector Start = BaseLocation + FVector(X * TileSize, 0, 0);
		FVector End = BaseLocation + FVector(X * TileSize, GridHeight * TileSize, 0);
		DrawDebugLine(GetWorld(), Start, End, DebugGridColor, false, -1.0f, 0, 2.0f);
	}

	// Draw tile IDs
	if (bShowTileIDs && bGenerationSuccess)
	{
		for (int32 Y = 0; Y < GridHeight; Y++)
		{
			for (int32 X = 0; X < GridWidth; X++)
			{
				const FWFCCell* Cell = WFCAlgorithm->GetCell(X, Y);
				if (Cell && Cell->bCollapsed)
				{
					FVector TextLocation = BaseLocation + FVector((X + 0.5f) * TileSize, (Y + 0.5f) * TileSize, 50.0f);
					FString Text = FString::Printf(TEXT("%d"), Cell->CollapsedTileID);
					DrawDebugString(GetWorld(), TextLocation, Text, nullptr, FColor::White, 0.0f, true);
				}
			}
		}
	}
}

int32 AWFCGenerator::GetSeed() const
{
	if (bUseRandomSeed)
	{
		return FMath::Rand();
	}
	return Seed;
}

void AWFCGenerator::ClearMeshes()
{
	for (UStaticMeshComponent* MeshComp : GeneratedMeshes)
	{
		if (MeshComp)
		{
			MeshComp->DestroyComponent();
		}
	}
	GeneratedMeshes.Empty();
}

// ========== Tile Set Presets ==========

TArray<FWFCTile> AWFCGenerator::CreateVillageTileSet()
{
	TArray<FWFCTile> TileSet;

	// Tile 0: Empty/Grass
	FWFCTile Empty(0, 5.0f);
	Empty.TileName = TEXT("Grass");
	Empty.AddValidNeighbor(EWFCDirection::North, 0);
	Empty.AddValidNeighbor(EWFCDirection::North, 1);
	Empty.AddValidNeighbor(EWFCDirection::North, 2);
	Empty.AddValidNeighbor(EWFCDirection::East, 0);
	Empty.AddValidNeighbor(EWFCDirection::East, 1);
	Empty.AddValidNeighbor(EWFCDirection::East, 2);
	Empty.AddValidNeighbor(EWFCDirection::South, 0);
	Empty.AddValidNeighbor(EWFCDirection::South, 1);
	Empty.AddValidNeighbor(EWFCDirection::South, 2);
	Empty.AddValidNeighbor(EWFCDirection::West, 0);
	Empty.AddValidNeighbor(EWFCDirection::West, 1);
	Empty.AddValidNeighbor(EWFCDirection::West, 2);
	TileSet.Add(Empty);

	// Tile 1: House
	FWFCTile House(1, 1.0f);
	House.TileName = TEXT("House");
	House.AddValidNeighbor(EWFCDirection::North, 0);
	House.AddValidNeighbor(EWFCDirection::North, 2);
	House.AddValidNeighbor(EWFCDirection::East, 0);
	House.AddValidNeighbor(EWFCDirection::East, 2);
	House.AddValidNeighbor(EWFCDirection::South, 0);
	House.AddValidNeighbor(EWFCDirection::South, 2);
	House.AddValidNeighbor(EWFCDirection::West, 0);
	House.AddValidNeighbor(EWFCDirection::West, 2);
	TileSet.Add(House);

	// Tile 2: Road
	FWFCTile Road(2, 2.0f);
	Road.TileName = TEXT("Road");
	Road.AddValidNeighbor(EWFCDirection::North, 0);
	Road.AddValidNeighbor(EWFCDirection::North, 1);
	Road.AddValidNeighbor(EWFCDirection::North, 2);
	Road.AddValidNeighbor(EWFCDirection::East, 0);
	Road.AddValidNeighbor(EWFCDirection::East, 1);
	Road.AddValidNeighbor(EWFCDirection::East, 2);
	Road.AddValidNeighbor(EWFCDirection::South, 0);
	Road.AddValidNeighbor(EWFCDirection::South, 1);
	Road.AddValidNeighbor(EWFCDirection::South, 2);
	Road.AddValidNeighbor(EWFCDirection::West, 0);
	Road.AddValidNeighbor(EWFCDirection::West, 1);
	Road.AddValidNeighbor(EWFCDirection::West, 2);
	TileSet.Add(Road);

	return TileSet;
}

TArray<FWFCTile> AWFCGenerator::CreateDungeonTileSet()
{
	TArray<FWFCTile> TileSet;

	// Tile 0: Wall
	FWFCTile Wall(0, 3.0f);
	Wall.TileName = TEXT("Wall");
	Wall.AddValidNeighbor(EWFCDirection::North, 0);
	Wall.AddValidNeighbor(EWFCDirection::North, 1);
	Wall.AddValidNeighbor(EWFCDirection::North, 3);
	Wall.AddValidNeighbor(EWFCDirection::East, 0);
	Wall.AddValidNeighbor(EWFCDirection::East, 1);
	Wall.AddValidNeighbor(EWFCDirection::East, 3);
	Wall.AddValidNeighbor(EWFCDirection::South, 0);
	Wall.AddValidNeighbor(EWFCDirection::South, 1);
	Wall.AddValidNeighbor(EWFCDirection::South, 3);
	Wall.AddValidNeighbor(EWFCDirection::West, 0);
	Wall.AddValidNeighbor(EWFCDirection::West, 1);
	Wall.AddValidNeighbor(EWFCDirection::West, 3);
	TileSet.Add(Wall);

	// Tile 1: Corridor
	FWFCTile Corridor(1, 2.0f);
	Corridor.TileName = TEXT("Corridor");
	Corridor.AddValidNeighbor(EWFCDirection::North, 0);
	Corridor.AddValidNeighbor(EWFCDirection::North, 1);
	Corridor.AddValidNeighbor(EWFCDirection::North, 2);
	Corridor.AddValidNeighbor(EWFCDirection::North, 3);
	Corridor.AddValidNeighbor(EWFCDirection::East, 0);
	Corridor.AddValidNeighbor(EWFCDirection::East, 1);
	Corridor.AddValidNeighbor(EWFCDirection::East, 2);
	Corridor.AddValidNeighbor(EWFCDirection::East, 3);
	Corridor.AddValidNeighbor(EWFCDirection::South, 0);
	Corridor.AddValidNeighbor(EWFCDirection::South, 1);
	Corridor.AddValidNeighbor(EWFCDirection::South, 2);
	Corridor.AddValidNeighbor(EWFCDirection::South, 3);
	Corridor.AddValidNeighbor(EWFCDirection::West, 0);
	Corridor.AddValidNeighbor(EWFCDirection::West, 1);
	Corridor.AddValidNeighbor(EWFCDirection::West, 2);
	Corridor.AddValidNeighbor(EWFCDirection::West, 3);
	TileSet.Add(Corridor);

	// Tile 2: Room
	FWFCTile Room(2, 1.0f);
	Room.TileName = TEXT("Room");
	Room.AddValidNeighbor(EWFCDirection::North, 1);
	Room.AddValidNeighbor(EWFCDirection::North, 2);
	Room.AddValidNeighbor(EWFCDirection::East, 1);
	Room.AddValidNeighbor(EWFCDirection::East, 2);
	Room.AddValidNeighbor(EWFCDirection::South, 1);
	Room.AddValidNeighbor(EWFCDirection::South, 2);
	Room.AddValidNeighbor(EWFCDirection::West, 1);
	Room.AddValidNeighbor(EWFCDirection::West, 2);
	TileSet.Add(Room);

	// Tile 3: Door
	FWFCTile Door(3, 0.5f);
	Door.TileName = TEXT("Door");
	Door.AddValidNeighbor(EWFCDirection::North, 0);
	Door.AddValidNeighbor(EWFCDirection::North, 1);
	Door.AddValidNeighbor(EWFCDirection::North, 2);
	Door.AddValidNeighbor(EWFCDirection::East, 0);
	Door.AddValidNeighbor(EWFCDirection::East, 1);
	Door.AddValidNeighbor(EWFCDirection::East, 2);
	Door.AddValidNeighbor(EWFCDirection::South, 0);
	Door.AddValidNeighbor(EWFCDirection::South, 1);
	Door.AddValidNeighbor(EWFCDirection::South, 2);
	Door.AddValidNeighbor(EWFCDirection::West, 0);
	Door.AddValidNeighbor(EWFCDirection::West, 1);
	Door.AddValidNeighbor(EWFCDirection::West, 2);
	TileSet.Add(Door);

	return TileSet;
}

TArray<FWFCTile> AWFCGenerator::CreateRoadTileSet()
{
	TArray<FWFCTile> TileSet;

	// Tile 0: Grass
	FWFCTile Grass(0, 5.0f);
	Grass.TileName = TEXT("Grass");
	Grass.AddValidNeighbor(EWFCDirection::North, 0);
	Grass.AddValidNeighbor(EWFCDirection::North, 1);
	Grass.AddValidNeighbor(EWFCDirection::North, 2);
	Grass.AddValidNeighbor(EWFCDirection::North, 3);
	Grass.AddValidNeighbor(EWFCDirection::North, 4);
	Grass.AddValidNeighbor(EWFCDirection::East, 0);
	Grass.AddValidNeighbor(EWFCDirection::East, 1);
	Grass.AddValidNeighbor(EWFCDirection::East, 2);
	Grass.AddValidNeighbor(EWFCDirection::East, 3);
	Grass.AddValidNeighbor(EWFCDirection::East, 4);
	Grass.AddValidNeighbor(EWFCDirection::South, 0);
	Grass.AddValidNeighbor(EWFCDirection::South, 1);
	Grass.AddValidNeighbor(EWFCDirection::South, 2);
	Grass.AddValidNeighbor(EWFCDirection::South, 3);
	Grass.AddValidNeighbor(EWFCDirection::South, 4);
	Grass.AddValidNeighbor(EWFCDirection::West, 0);
	Grass.AddValidNeighbor(EWFCDirection::West, 1);
	Grass.AddValidNeighbor(EWFCDirection::West, 2);
	Grass.AddValidNeighbor(EWFCDirection::West, 3);
	Grass.AddValidNeighbor(EWFCDirection::West, 4);
	TileSet.Add(Grass);

	// Tile 1: Straight Road (Horizontal)
	FWFCTile StraightH(1, 2.0f);
	StraightH.TileName = TEXT("Road_Straight_H");
	StraightH.AddValidNeighbor(EWFCDirection::North, 0);
	StraightH.AddValidNeighbor(EWFCDirection::East, 1);
	StraightH.AddValidNeighbor(EWFCDirection::East, 3);
	StraightH.AddValidNeighbor(EWFCDirection::East, 4);
	StraightH.AddValidNeighbor(EWFCDirection::South, 0);
	StraightH.AddValidNeighbor(EWFCDirection::West, 1);
	StraightH.AddValidNeighbor(EWFCDirection::West, 3);
	StraightH.AddValidNeighbor(EWFCDirection::West, 4);
	TileSet.Add(StraightH);

	// Tile 2: Straight Road (Vertical)
	FWFCTile StraightV(2, 2.0f);
	StraightV.TileName = TEXT("Road_Straight_V");
	StraightV.AddValidNeighbor(EWFCDirection::North, 2);
	StraightV.AddValidNeighbor(EWFCDirection::North, 3);
	StraightV.AddValidNeighbor(EWFCDirection::North, 4);
	StraightV.AddValidNeighbor(EWFCDirection::East, 0);
	StraightV.AddValidNeighbor(EWFCDirection::South, 2);
	StraightV.AddValidNeighbor(EWFCDirection::South, 3);
	StraightV.AddValidNeighbor(EWFCDirection::South, 4);
	StraightV.AddValidNeighbor(EWFCDirection::West, 0);
	TileSet.Add(StraightV);

	// Tile 3: T-Junction
	FWFCTile TJunction(3, 0.5f);
	TJunction.TileName = TEXT("Road_T_Junction");
	TJunction.AddValidNeighbor(EWFCDirection::North, 2);
	TJunction.AddValidNeighbor(EWFCDirection::North, 3);
	TJunction.AddValidNeighbor(EWFCDirection::North, 4);
	TJunction.AddValidNeighbor(EWFCDirection::East, 1);
	TJunction.AddValidNeighbor(EWFCDirection::East, 3);
	TJunction.AddValidNeighbor(EWFCDirection::East, 4);
	TJunction.AddValidNeighbor(EWFCDirection::South, 0);
	TJunction.AddValidNeighbor(EWFCDirection::West, 1);
	TJunction.AddValidNeighbor(EWFCDirection::West, 3);
	TJunction.AddValidNeighbor(EWFCDirection::West, 4);
	TileSet.Add(TJunction);

	// Tile 4: Crossroad
	FWFCTile Cross(4, 0.3f);
	Cross.TileName = TEXT("Road_Crossroad");
	Cross.AddValidNeighbor(EWFCDirection::North, 2);
	Cross.AddValidNeighbor(EWFCDirection::North, 3);
	Cross.AddValidNeighbor(EWFCDirection::North, 4);
	Cross.AddValidNeighbor(EWFCDirection::East, 1);
	Cross.AddValidNeighbor(EWFCDirection::East, 3);
	Cross.AddValidNeighbor(EWFCDirection::East, 4);
	Cross.AddValidNeighbor(EWFCDirection::South, 2);
	Cross.AddValidNeighbor(EWFCDirection::South, 3);
	Cross.AddValidNeighbor(EWFCDirection::South, 4);
	Cross.AddValidNeighbor(EWFCDirection::West, 1);
	Cross.AddValidNeighbor(EWFCDirection::West, 3);
	Cross.AddValidNeighbor(EWFCDirection::West, 4);
	TileSet.Add(Cross);

	return TileSet;
}

TArray<FWFCTile> AWFCGenerator::CreateMazeTileSet()
{
	TArray<FWFCTile> TileSet;

	// Tile 0: Wall
	FWFCTile Wall(0, 2.0f);
	Wall.TileName = TEXT("Wall");
	Wall.AddValidNeighbor(EWFCDirection::North, 0);
	Wall.AddValidNeighbor(EWFCDirection::North, 1);
	Wall.AddValidNeighbor(EWFCDirection::East, 0);
	Wall.AddValidNeighbor(EWFCDirection::East, 1);
	Wall.AddValidNeighbor(EWFCDirection::South, 0);
	Wall.AddValidNeighbor(EWFCDirection::South, 1);
	Wall.AddValidNeighbor(EWFCDirection::West, 0);
	Wall.AddValidNeighbor(EWFCDirection::West, 1);
	TileSet.Add(Wall);

	// Tile 1: Path
	FWFCTile Path(1, 1.0f);
	Path.TileName = TEXT("Path");
	Path.AddValidNeighbor(EWFCDirection::North, 0);
	Path.AddValidNeighbor(EWFCDirection::North, 1);
	Path.AddValidNeighbor(EWFCDirection::East, 0);
	Path.AddValidNeighbor(EWFCDirection::East, 1);
	Path.AddValidNeighbor(EWFCDirection::South, 0);
	Path.AddValidNeighbor(EWFCDirection::South, 1);
	Path.AddValidNeighbor(EWFCDirection::West, 0);
	Path.AddValidNeighbor(EWFCDirection::West, 1);
	TileSet.Add(Path);

	return TileSet;
}

TArray<FWFCTile> AWFCGenerator::CreateBuildingTileSet()
{
	TArray<FWFCTile> TileSet;

	// Tile 0: Wall
	FWFCTile Wall(0, 3.0f);
	Wall.TileName = TEXT("Wall");
	Wall.AddValidNeighbor(EWFCDirection::North, 0);
	Wall.AddValidNeighbor(EWFCDirection::North, 1);
	Wall.AddValidNeighbor(EWFCDirection::North, 2);
	Wall.AddValidNeighbor(EWFCDirection::North, 3);
	Wall.AddValidNeighbor(EWFCDirection::East, 0);
	Wall.AddValidNeighbor(EWFCDirection::East, 1);
	Wall.AddValidNeighbor(EWFCDirection::East, 2);
	Wall.AddValidNeighbor(EWFCDirection::East, 3);
	Wall.AddValidNeighbor(EWFCDirection::South, 0);
	Wall.AddValidNeighbor(EWFCDirection::South, 1);
	Wall.AddValidNeighbor(EWFCDirection::South, 2);
	Wall.AddValidNeighbor(EWFCDirection::South, 3);
	Wall.AddValidNeighbor(EWFCDirection::West, 0);
	Wall.AddValidNeighbor(EWFCDirection::West, 1);
	Wall.AddValidNeighbor(EWFCDirection::West, 2);
	Wall.AddValidNeighbor(EWFCDirection::West, 3);
	TileSet.Add(Wall);

	// Tile 1: Floor
	FWFCTile Floor(1, 5.0f);
	Floor.TileName = TEXT("Floor");
	Floor.AddValidNeighbor(EWFCDirection::North, 0);
	Floor.AddValidNeighbor(EWFCDirection::North, 1);
	Floor.AddValidNeighbor(EWFCDirection::North, 2);
	Floor.AddValidNeighbor(EWFCDirection::North, 3);
	Floor.AddValidNeighbor(EWFCDirection::East, 0);
	Floor.AddValidNeighbor(EWFCDirection::East, 1);
	Floor.AddValidNeighbor(EWFCDirection::East, 2);
	Floor.AddValidNeighbor(EWFCDirection::East, 3);
	Floor.AddValidNeighbor(EWFCDirection::South, 0);
	Floor.AddValidNeighbor(EWFCDirection::South, 1);
	Floor.AddValidNeighbor(EWFCDirection::South, 2);
	Floor.AddValidNeighbor(EWFCDirection::South, 3);
	Floor.AddValidNeighbor(EWFCDirection::West, 0);
	Floor.AddValidNeighbor(EWFCDirection::West, 1);
	Floor.AddValidNeighbor(EWFCDirection::West, 2);
	Floor.AddValidNeighbor(EWFCDirection::West, 3);
	TileSet.Add(Floor);

	// Tile 2: Door
	FWFCTile Door(2, 0.5f);
	Door.TileName = TEXT("Door");
	Door.AddValidNeighbor(EWFCDirection::North, 0);
	Door.AddValidNeighbor(EWFCDirection::North, 1);
	Door.AddValidNeighbor(EWFCDirection::East, 0);
	Door.AddValidNeighbor(EWFCDirection::East, 1);
	Door.AddValidNeighbor(EWFCDirection::South, 0);
	Door.AddValidNeighbor(EWFCDirection::South, 1);
	Door.AddValidNeighbor(EWFCDirection::West, 0);
	Door.AddValidNeighbor(EWFCDirection::West, 1);
	TileSet.Add(Door);

	// Tile 3: Furniture
	FWFCTile Furniture(3, 1.0f);
	Furniture.TileName = TEXT("Furniture");
	Furniture.AddValidNeighbor(EWFCDirection::North, 0);
	Furniture.AddValidNeighbor(EWFCDirection::North, 1);
	Furniture.AddValidNeighbor(EWFCDirection::North, 3);
	Furniture.AddValidNeighbor(EWFCDirection::East, 0);
	Furniture.AddValidNeighbor(EWFCDirection::East, 1);
	Furniture.AddValidNeighbor(EWFCDirection::East, 3);
	Furniture.AddValidNeighbor(EWFCDirection::South, 0);
	Furniture.AddValidNeighbor(EWFCDirection::South, 1);
	Furniture.AddValidNeighbor(EWFCDirection::South, 3);
	Furniture.AddValidNeighbor(EWFCDirection::West, 0);
	Furniture.AddValidNeighbor(EWFCDirection::West, 1);
	Furniture.AddValidNeighbor(EWFCDirection::West, 3);
	TileSet.Add(Furniture);

	return TileSet;
}
