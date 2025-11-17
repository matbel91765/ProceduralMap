// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WFCTile.h"
#include "WaveFunctionCollapse.generated.h"

/**
 * Structure to save grid state for backtracking
 */
USTRUCT()
struct FWFCGridState
{
	GENERATED_BODY()

	TArray<FWFCCell> SavedGrid;
	int32 StateIndex;

	FWFCGridState() : StateIndex(-1) {}
};

/**
 * Wave Function Collapse algorithm implementation
 * Generates procedural structures by iteratively collapsing quantum superpositions
 * of tiles based on adjacency constraints
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAP_API UWaveFunctionCollapse : public UObject
{
	GENERATED_BODY()

public:
	UWaveFunctionCollapse();

	/** Width of the generation grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Grid")
	int32 GridWidth;

	/** Height of the generation grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Grid")
	int32 GridHeight;

	/** The set of all available tiles with their rules */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Tiles")
	TArray<FWFCTile> TileSet;

	/** The grid of cells being generated */
	UPROPERTY(BlueprintReadOnly, Category = "WFC|Grid")
	TArray<FWFCCell> Grid;

	/** Random seed for deterministic generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
	int32 RandomSeed;

	/** Maximum number of iterations before giving up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
	int32 MaxIterations;

	/** Enable backtracking on contradiction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
	bool bEnableBacktracking;

	/** Maximum backtrack attempts before failing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
	int32 MaxBacktrackDepth;

	/** Size of each tile in world units (for mesh generation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
	float TileSize;

	/** Whether to generate meshes automatically */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
	bool bGenerateMeshes;

	/**
	 * Main generation function - runs the complete WFC algorithm
	 * @return True if generation succeeded, false if contradiction occurred
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool Generate();

	/**
	 * Initialize the grid with all tiles possible in each cell
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	void InitializeGrid();

	/**
	 * Find the cell with lowest entropy (most constrained but not collapsed)
	 * @param OutX X coordinate of the cell
	 * @param OutY Y coordinate of the cell
	 * @return True if a cell was found, false if all collapsed
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool FindLowestEntropyCell(int32& OutX, int32& OutY);

	/**
	 * Collapse a specific cell to a single tile using weighted random selection
	 * @param X X coordinate of the cell
	 * @param Y Y coordinate of the cell
	 * @return True if collapse succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool CollapseCell(int32 X, int32 Y);

	/**
	 * Propagate constraints from a collapsed cell to its neighbors
	 * @param StartX X coordinate of the starting cell
	 * @param StartY Y coordinate of the starting cell
	 * @return True if propagation succeeded without contradiction
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool Propagate(int32 StartX, int32 StartY);

	/**
	 * Constrain a neighbor cell based on adjacency rules
	 * @param X X coordinate of the cell to constrain
	 * @param Y Y coordinate of the cell to constrain
	 * @return True if cell still has valid possibilities
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool ConstrainNeighbor(int32 X, int32 Y);

	/**
	 * Get the cell at a specific grid position
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @return Pointer to the cell, or nullptr if out of bounds
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	FWFCCell* GetCell(int32 X, int32 Y);

	/**
	 * Get a tile from the tileset by ID
	 * @param TileID The tile ID to find
	 * @return Pointer to the tile, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	const FWFCTile* GetTile(int32 TileID) const;

	/**
	 * Check if a grid position is valid
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @return True if position is within grid bounds
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool IsValidPosition(int32 X, int32 Y) const;

	/**
	 * Get neighboring position in a specific direction
	 * @param X Current X coordinate
	 * @param Y Current Y coordinate
	 * @param Direction The direction to look
	 * @param OutX Output X coordinate
	 * @param OutY Output Y coordinate
	 * @return True if neighbor exists (within bounds)
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool GetNeighborPosition(int32 X, int32 Y, EWFCDirection Direction, int32& OutX, int32& OutY) const;

	/**
	 * Check if the grid is fully collapsed
	 * @return True if all cells are collapsed
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool IsFullyCollapsed() const;

	/**
	 * Reset the grid to initial state
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	void ResetGrid();

	/**
	 * Get the opposite direction
	 * @param Direction Input direction
	 * @return Opposite direction
	 */
	static EWFCDirection GetOppositeDirection(EWFCDirection Direction);

	/**
	 * Convert grid position to world location
	 * @param X Grid X coordinate
	 * @param Y Grid Y coordinate
	 * @param BaseLocation Base world location
	 * @return World location for this grid position
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	FVector GridToWorld(int32 X, int32 Y, const FVector& BaseLocation) const;

	/**
	 * Generate visual meshes for the collapsed grid
	 * @param World The world to spawn meshes in
	 * @param BaseLocation Base location for the grid
	 * @param Owner Owner actor for spawned components
	 * @return Array of spawned static mesh components
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	TArray<UStaticMeshComponent*> GenerateMeshes(UWorld* World, const FVector& BaseLocation, AActor* Owner);

	/**
	 * Get generation statistics
	 * @param OutTotalCells Total number of cells
	 * @param OutCollapsedCells Number of collapsed cells
	 * @param OutAverageEntropy Average entropy across uncollapsed cells
	 */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	void GetStatistics(int32& OutTotalCells, int32& OutCollapsedCells, float& OutAverageEntropy) const;

protected:
	/** Random stream for deterministic generation */
	FRandomStream RandomStream;

	/** Saved states for backtracking */
	TArray<FWFCGridState> SavedStates;

	/** Current iteration count */
	int32 CurrentIteration;

	/**
	 * Save current grid state for potential backtracking
	 */
	void SaveState();

	/**
	 * Restore a previous grid state
	 * @return True if a state was restored
	 */
	bool RestoreState();

	/**
	 * Select a tile from possible tiles using weighted random
	 * @param PossibleTiles Set of possible tile IDs
	 * @return Selected tile ID
	 */
	int32 WeightedRandomSelect(const TSet<int32>& PossibleTiles);

	/**
	 * Check if current state has any contradictions
	 * @return True if contradiction found
	 */
	bool HasContradiction() const;

	/**
	 * Get all valid tile IDs that can be neighbors to a cell
	 * @param X Cell X coordinate
	 * @param Y Cell Y coordinate
	 * @param Direction Direction to check
	 * @return Set of valid neighbor tile IDs
	 */
	TSet<int32> GetValidNeighborTiles(int32 X, int32 Y, EWFCDirection Direction) const;

	/**
	 * Add noise to entropy for breaking ties randomly
	 * @param Entropy Base entropy value
	 * @return Entropy with small random noise
	 */
	float AddEntropyNoise(float Entropy) const;
};
