// Copyright Epic Games, Inc. All Rights Reserved.

#include "WaveFunctionCollapse.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UWaveFunctionCollapse::UWaveFunctionCollapse()
	: GridWidth(10)
	, GridHeight(10)
	, RandomSeed(12345)
	, MaxIterations(10000)
	, bEnableBacktracking(true)
	, MaxBacktrackDepth(50)
	, TileSize(100.0f)
	, bGenerateMeshes(true)
	, CurrentIteration(0)
{
}

bool UWaveFunctionCollapse::Generate()
{
	// Initialize random stream
	RandomStream.Initialize(RandomSeed);
	CurrentIteration = 0;
	SavedStates.Empty();

	// Initialize grid with all possibilities
	InitializeGrid();

	// Main WFC loop
	while (!IsFullyCollapsed() && CurrentIteration < MaxIterations)
	{
		CurrentIteration++;

		// Find cell with lowest entropy
		int32 X, Y;
		if (!FindLowestEntropyCell(X, Y))
		{
			// All cells are collapsed - success!
			break;
		}

		// Save state before collapsing (for backtracking)
		if (bEnableBacktracking && SavedStates.Num() < MaxBacktrackDepth)
		{
			SaveState();
		}

		// Collapse the cell
		if (!CollapseCell(X, Y))
		{
			// Contradiction during collapse
			if (bEnableBacktracking && RestoreState())
			{
				continue; // Try again from previous state
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("WFC: Failed to collapse cell at (%d, %d) - contradiction"), X, Y);
				return false;
			}
		}

		// Propagate constraints
		if (!Propagate(X, Y))
		{
			// Contradiction during propagation
			if (bEnableBacktracking && RestoreState())
			{
				continue; // Try again from previous state
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("WFC: Propagation failed from cell (%d, %d) - contradiction"), X, Y);
				return false;
			}
		}

		// Check for contradictions
		if (HasContradiction())
		{
			if (bEnableBacktracking && RestoreState())
			{
				continue;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("WFC: Contradiction detected in grid state"));
				return false;
			}
		}
	}

	if (CurrentIteration >= MaxIterations)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFC: Reached maximum iterations (%d)"), MaxIterations);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("WFC: Generation completed successfully in %d iterations"), CurrentIteration);
	return IsFullyCollapsed();
}

void UWaveFunctionCollapse::InitializeGrid()
{
	Grid.Empty();
	Grid.SetNum(GridWidth * GridHeight);

	// Create array of all tile IDs
	TArray<int32> AllTileIDs;
	for (const FWFCTile& Tile : TileSet)
	{
		AllTileIDs.Add(Tile.TileID);
	}

	// Initialize each cell with all possible tiles
	for (int32 Y = 0; Y < GridHeight; Y++)
	{
		for (int32 X = 0; X < GridWidth; X++)
		{
			FWFCCell* Cell = GetCell(X, Y);
			if (Cell)
			{
				Cell->Reset(AllTileIDs);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("WFC: Grid initialized (%dx%d) with %d tile types"), GridWidth, GridHeight, TileSet.Num());
}

bool UWaveFunctionCollapse::FindLowestEntropyCell(int32& OutX, int32& OutY)
{
	float LowestEntropy = FLT_MAX;
	bool bFoundCell = false;

	for (int32 Y = 0; Y < GridHeight; Y++)
	{
		for (int32 X = 0; X < GridWidth; X++)
		{
			const FWFCCell* Cell = GetCell(X, Y);
			if (Cell && !Cell->bCollapsed && Cell->PossibleTiles.Num() > 0)
			{
				float Entropy = AddEntropyNoise(Cell->GetEntropy(TileSet));

				if (Entropy < LowestEntropy)
				{
					LowestEntropy = Entropy;
					OutX = X;
					OutY = Y;
					bFoundCell = true;
				}
			}
		}
	}

	return bFoundCell;
}

bool UWaveFunctionCollapse::CollapseCell(int32 X, int32 Y)
{
	FWFCCell* Cell = GetCell(X, Y);
	if (!Cell || Cell->bCollapsed)
	{
		return false;
	}

	if (Cell->PossibleTiles.Num() == 0)
	{
		// Contradiction - no possible tiles
		return false;
	}

	// Select a tile using weighted random
	int32 SelectedTileID = WeightedRandomSelect(Cell->PossibleTiles);

	if (SelectedTileID < 0)
	{
		return false;
	}

	// Collapse the cell
	Cell->Collapse(SelectedTileID);

	UE_LOG(LogTemp, VeryVerbose, TEXT("WFC: Collapsed cell (%d, %d) to tile %d"), X, Y, SelectedTileID);
	return true;
}

bool UWaveFunctionCollapse::Propagate(int32 StartX, int32 StartY)
{
	// Use a stack for propagation (depth-first)
	TArray<FIntPoint> Stack;
	Stack.Add(FIntPoint(StartX, StartY));

	TSet<FIntPoint> Visited;

	while (Stack.Num() > 0)
	{
		FIntPoint Current = Stack.Pop();

		if (Visited.Contains(Current))
		{
			continue;
		}
		Visited.Add(Current);

		// Check all four directions
		for (int32 DirInt = 0; DirInt < 4; DirInt++)
		{
			EWFCDirection Direction = static_cast<EWFCDirection>(DirInt);
			int32 NeighborX, NeighborY;

			if (GetNeighborPosition(Current.X, Current.Y, Direction, NeighborX, NeighborY))
			{
				FWFCCell* Neighbor = GetCell(NeighborX, NeighborY);

				if (Neighbor && !Neighbor->bCollapsed)
				{
					// Store original possibilities count
					int32 OriginalCount = Neighbor->PossibleTiles.Num();

					// Get valid tiles for this neighbor based on all its neighbors
					TSet<int32> ValidTiles = GetValidNeighborTiles(NeighborX, NeighborY, Direction);

					// Intersect with current possibilities
					TSet<int32> NewPossibilities = Neighbor->PossibleTiles.Intersect(ValidTiles);

					if (NewPossibilities.Num() == 0)
					{
						// Contradiction!
						UE_LOG(LogTemp, Verbose, TEXT("WFC: Contradiction at (%d, %d) during propagation"), NeighborX, NeighborY);
						return false;
					}

					// Update if possibilities changed
					if (NewPossibilities.Num() < OriginalCount)
					{
						Neighbor->PossibleTiles = NewPossibilities;

						// Add to stack to propagate further
						if (!Visited.Contains(FIntPoint(NeighborX, NeighborY)))
						{
							Stack.Add(FIntPoint(NeighborX, NeighborY));
						}
					}
				}
			}
		}
	}

	return true;
}

bool UWaveFunctionCollapse::ConstrainNeighbor(int32 X, int32 Y)
{
	FWFCCell* Cell = GetCell(X, Y);
	if (!Cell || Cell->bCollapsed)
	{
		return true;
	}

	TSet<int32> AllValidTiles;
	bool bHasConstraints = false;

	// Check all four directions
	for (int32 DirInt = 0; DirInt < 4; DirInt++)
	{
		EWFCDirection Direction = static_cast<EWFCDirection>(DirInt);
		int32 NeighborX, NeighborY;

		if (GetNeighborPosition(X, Y, Direction, NeighborX, NeighborY))
		{
			const FWFCCell* Neighbor = GetCell(NeighborX, NeighborY);

			if (Neighbor && Neighbor->bCollapsed)
			{
				bHasConstraints = true;
				const FWFCTile* NeighborTile = GetTile(Neighbor->CollapsedTileID);

				if (NeighborTile)
				{
					// Get opposite direction for compatibility check
					EWFCDirection OppositeDir = GetOppositeDirection(Direction);
					int32 OppDirIndex = static_cast<int32>(OppositeDir);

					// Add all valid neighbors from this direction
					for (int32 ValidID : NeighborTile->ValidNeighbors[OppDirIndex])
					{
						AllValidTiles.Add(ValidID);
					}
				}
			}
		}
	}

	if (bHasConstraints)
	{
		// Intersect with current possibilities
		Cell->PossibleTiles = Cell->PossibleTiles.Intersect(AllValidTiles);
	}

	return Cell->PossibleTiles.Num() > 0;
}

FWFCCell* UWaveFunctionCollapse::GetCell(int32 X, int32 Y)
{
	if (!IsValidPosition(X, Y))
	{
		return nullptr;
	}

	int32 Index = Y * GridWidth + X;
	return &Grid[Index];
}

const FWFCTile* UWaveFunctionCollapse::GetTile(int32 TileID) const
{
	return TileSet.FindByPredicate([TileID](const FWFCTile& Tile)
	{
		return Tile.TileID == TileID;
	});
}

bool UWaveFunctionCollapse::IsValidPosition(int32 X, int32 Y) const
{
	return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight;
}

bool UWaveFunctionCollapse::GetNeighborPosition(int32 X, int32 Y, EWFCDirection Direction, int32& OutX, int32& OutY) const
{
	OutX = X;
	OutY = Y;

	switch (Direction)
	{
	case EWFCDirection::North:
		OutY = Y - 1;
		break;
	case EWFCDirection::East:
		OutX = X + 1;
		break;
	case EWFCDirection::South:
		OutY = Y + 1;
		break;
	case EWFCDirection::West:
		OutX = X - 1;
		break;
	}

	return IsValidPosition(OutX, OutY);
}

bool UWaveFunctionCollapse::IsFullyCollapsed() const
{
	for (const FWFCCell& Cell : Grid)
	{
		if (!Cell.bCollapsed)
		{
			return false;
		}
	}
	return true;
}

void UWaveFunctionCollapse::ResetGrid()
{
	InitializeGrid();
	SavedStates.Empty();
	CurrentIteration = 0;
}

EWFCDirection UWaveFunctionCollapse::GetOppositeDirection(EWFCDirection Direction)
{
	switch (Direction)
	{
	case EWFCDirection::North:
		return EWFCDirection::South;
	case EWFCDirection::East:
		return EWFCDirection::West;
	case EWFCDirection::South:
		return EWFCDirection::North;
	case EWFCDirection::West:
		return EWFCDirection::East;
	default:
		return EWFCDirection::North;
	}
}

FVector UWaveFunctionCollapse::GridToWorld(int32 X, int32 Y, const FVector& BaseLocation) const
{
	return BaseLocation + FVector(X * TileSize, Y * TileSize, 0.0f);
}

TArray<UStaticMeshComponent*> UWaveFunctionCollapse::GenerateMeshes(UWorld* World, const FVector& BaseLocation, AActor* Owner)
{
	TArray<UStaticMeshComponent*> MeshComponents;

	if (!World || !Owner || !bGenerateMeshes)
	{
		return MeshComponents;
	}

	for (int32 Y = 0; Y < GridHeight; Y++)
	{
		for (int32 X = 0; X < GridWidth; X++)
		{
			const FWFCCell* Cell = GetCell(X, Y);
			if (Cell && Cell->bCollapsed)
			{
				const FWFCTile* Tile = GetTile(Cell->CollapsedTileID);
				if (Tile && !Tile->TileMesh.IsNull())
				{
					// Create mesh component
					UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Owner);
					if (MeshComp)
					{
						MeshComp->RegisterComponent();
						MeshComp->SetStaticMesh(Tile->TileMesh.LoadSynchronous());

						FVector WorldLocation = GridToWorld(X, Y, BaseLocation);
						MeshComp->SetWorldLocation(WorldLocation);
						MeshComp->SetWorldRotation(Tile->MeshRotation);

						MeshComponents.Add(MeshComp);
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("WFC: Generated %d mesh components"), MeshComponents.Num());
	return MeshComponents;
}

void UWaveFunctionCollapse::GetStatistics(int32& OutTotalCells, int32& OutCollapsedCells, float& OutAverageEntropy) const
{
	OutTotalCells = Grid.Num();
	OutCollapsedCells = 0;
	float TotalEntropy = 0.0f;
	int32 UnCollapsedCount = 0;

	for (const FWFCCell& Cell : Grid)
	{
		if (Cell.bCollapsed)
		{
			OutCollapsedCells++;
		}
		else
		{
			TotalEntropy += Cell.GetEntropy(TileSet);
			UnCollapsedCount++;
		}
	}

	OutAverageEntropy = UnCollapsedCount > 0 ? (TotalEntropy / UnCollapsedCount) : 0.0f;
}

void UWaveFunctionCollapse::SaveState()
{
	FWFCGridState State;
	State.SavedGrid = Grid;
	State.StateIndex = SavedStates.Num();
	SavedStates.Add(State);
}

bool UWaveFunctionCollapse::RestoreState()
{
	if (SavedStates.Num() == 0)
	{
		return false;
	}

	// Pop the last state
	FWFCGridState State = SavedStates.Pop();
	Grid = State.SavedGrid;

	UE_LOG(LogTemp, Verbose, TEXT("WFC: Restored state %d (backtracking)"), State.StateIndex);
	return true;
}

int32 UWaveFunctionCollapse::WeightedRandomSelect(const TSet<int32>& PossibleTiles)
{
	if (PossibleTiles.Num() == 0)
	{
		return -1;
	}

	if (PossibleTiles.Num() == 1)
	{
		return PossibleTiles.Array()[0];
	}

	// Calculate total weight
	float TotalWeight = 0.0f;
	TArray<float> Weights;
	TArray<int32> TileIDs;

	for (int32 TileID : PossibleTiles)
	{
		const FWFCTile* Tile = GetTile(TileID);
		if (Tile)
		{
			TotalWeight += Tile->Weight;
			Weights.Add(Tile->Weight);
			TileIDs.Add(TileID);
		}
	}

	if (TotalWeight <= 0.0f || TileIDs.Num() == 0)
	{
		return -1;
	}

	// Random weighted selection
	float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (int32 i = 0; i < TileIDs.Num(); i++)
	{
		CurrentWeight += Weights[i];
		if (RandomValue <= CurrentWeight)
		{
			return TileIDs[i];
		}
	}

	// Fallback to last tile (should not happen)
	return TileIDs.Last();
}

bool UWaveFunctionCollapse::HasContradiction() const
{
	for (const FWFCCell& Cell : Grid)
	{
		if (!Cell.bCollapsed && Cell.PossibleTiles.Num() == 0)
		{
			return true;
		}
	}
	return false;
}

TSet<int32> UWaveFunctionCollapse::GetValidNeighborTiles(int32 X, int32 Y, EWFCDirection Direction) const
{
	TSet<int32> ValidTiles;

	// Check all neighbors in all directions
	for (int32 DirInt = 0; DirInt < 4; DirInt++)
	{
		EWFCDirection CheckDir = static_cast<EWFCDirection>(DirInt);
		int32 NeighborX, NeighborY;

		if (GetNeighborPosition(X, Y, CheckDir, NeighborX, NeighborY))
		{
			const FWFCCell* Neighbor = GetCell(NeighborX, NeighborY);

			if (Neighbor && Neighbor->bCollapsed)
			{
				const FWFCTile* NeighborTile = GetTile(Neighbor->CollapsedTileID);

				if (NeighborTile)
				{
					// Get opposite direction
					EWFCDirection OppositeDir = GetOppositeDirection(CheckDir);
					int32 OppDirIndex = static_cast<int32>(OppositeDir);

					// Add valid neighbors from this tile
					for (int32 ValidID : NeighborTile->ValidNeighbors[OppDirIndex])
					{
						ValidTiles.Add(ValidID);
					}
				}
			}
		}
	}

	// If no constraints found, all tiles are valid
	if (ValidTiles.Num() == 0)
	{
		for (const FWFCTile& Tile : TileSet)
		{
			ValidTiles.Add(Tile.TileID);
		}
	}

	return ValidTiles;
}

float UWaveFunctionCollapse::AddEntropyNoise(float Entropy) const
{
	// Add small random noise to break ties
	return Entropy + RandomStream.FRandRange(0.0f, 0.001f);
}
