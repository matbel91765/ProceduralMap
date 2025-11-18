// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTile.generated.h"

/**
 * Enum representing the four cardinal directions for tile adjacency
 */
UENUM(BlueprintType)
enum class EWFCDirection : uint8
{
	North = 0,
	East = 1,
	South = 2,
	West = 3
};

/**
 * Structure representing a single tile in the Wave Function Collapse algorithm
 * Contains tile identification, adjacency rules, and probability weight
 */
USTRUCT(BlueprintType)
struct PROCEDURALMAP_API FWFCTile
{
	GENERATED_BODY()

	/** Unique identifier for this tile type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	int32 TileID;

	/** Array of valid neighbor tile IDs for each direction [North, East, South, West] */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	TArray<int32> ValidNeighbors[4];

	/** Probability weight for this tile during collapse (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	float Weight;

	/** Optional mesh to spawn for this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	TSoftObjectPtr<UStaticMesh> TileMesh;

	/** Rotation to apply to the mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FRotator MeshRotation;

	/** Display name for debugging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FString TileName;

	FWFCTile()
		: TileID(-1)
		, Weight(1.0f)
		, MeshRotation(FRotator::ZeroRotator)
		, TileName(TEXT("Empty"))
	{
	}

	FWFCTile(int32 InTileID, float InWeight = 1.0f)
		: TileID(InTileID)
		, Weight(InWeight)
		, MeshRotation(FRotator::ZeroRotator)
		, TileName(FString::Printf(TEXT("Tile_%d"), InTileID))
	{
	}

	/**
	 * Add a valid neighbor for a specific direction
	 * @param Direction The direction (North, East, South, West)
	 * @param NeighborTileID The ID of the tile that can be adjacent in this direction
	 */
	void AddValidNeighbor(EWFCDirection Direction, int32 NeighborTileID)
	{
		int32 DirIndex = static_cast<int32>(Direction);
		ValidNeighbors[DirIndex].AddUnique(NeighborTileID);
	}

	/**
	 * Check if a tile ID is valid for a specific direction
	 * @param Direction The direction to check
	 * @param NeighborTileID The tile ID to validate
	 * @return True if the neighbor is valid in this direction
	 */
	bool IsValidNeighbor(EWFCDirection Direction, int32 NeighborTileID) const
	{
		int32 DirIndex = static_cast<int32>(Direction);
		return ValidNeighbors[DirIndex].Contains(NeighborTileID);
	}
};

/**
 * Structure representing a single cell in the WFC grid
 * Tracks possible tiles, collapse state, and entropy
 */
USTRUCT(BlueprintType)
struct PROCEDURALMAP_API FWFCCell
{
	GENERATED_BODY()

	/** Set of possible tile IDs that could occupy this cell */
	UPROPERTY(BlueprintReadWrite, Category = "WFC")
	TSet<int32> PossibleTiles;

	/** Whether this cell has been collapsed to a single tile */
	UPROPERTY(BlueprintReadWrite, Category = "WFC")
	bool bCollapsed;

	/** The final tile ID after collapse (-1 if not collapsed) */
	UPROPERTY(BlueprintReadWrite, Category = "WFC")
	int32 CollapsedTileID;

	FWFCCell()
		: bCollapsed(false)
		, CollapsedTileID(-1)
	{
	}

	/**
	 * Calculate Shannon entropy for this cell
	 * Entropy = sum of (weight * log(weight)) for all possible tiles
	 * Lower entropy = more constrained = should be collapsed first
	 *
	 * @param TileSet Reference to the array of all available tiles
	 * @return The calculated entropy value (lower is more constrained)
	 */
	float GetEntropy(const TArray<FWFCTile>& TileSet) const
	{
		if (bCollapsed || PossibleTiles.Num() == 0)
		{
			return 0.0f;
		}

		if (PossibleTiles.Num() == 1)
		{
			return 0.0f;
		}

		float SumWeights = 0.0f;
		float SumWeightLogWeight = 0.0f;

		for (int32 TileID : PossibleTiles)
		{
			// Find the tile in the tileset
			const FWFCTile* Tile = TileSet.FindByPredicate([TileID](const FWFCTile& T)
			{
				return T.TileID == TileID;
			});

			if (Tile)
			{
				float Weight = FMath::Max(Tile->Weight, 0.001f); // Prevent log(0)
				SumWeights += Weight;
				SumWeightLogWeight += Weight * FMath::Loge(Weight);
			}
		}

		// Shannon entropy formula: H = log(sum(weights)) - (sum(weight*log(weight)) / sum(weights))
		if (SumWeights > 0.0f)
		{
			return FMath::Loge(SumWeights) - (SumWeightLogWeight / SumWeights);
		}

		return 0.0f;
	}

	/**
	 * Remove a tile from the set of possible tiles
	 * @param TileID The tile to remove
	 * @return True if the tile was removed
	 */
	bool RemovePossibility(int32 TileID)
	{
		return PossibleTiles.Remove(TileID) > 0;
	}

	/**
	 * Collapse this cell to a specific tile
	 * @param TileID The tile to collapse to
	 */
	void Collapse(int32 TileID)
	{
		bCollapsed = true;
		CollapsedTileID = TileID;
		PossibleTiles.Empty();
		PossibleTiles.Add(TileID);
	}

	/**
	 * Reset this cell to uncollapsed state
	 * @param AllTileIDs All possible tile IDs to reset to
	 */
	void Reset(const TArray<int32>& AllTileIDs)
	{
		bCollapsed = false;
		CollapsedTileID = -1;
		PossibleTiles.Empty();
		for (int32 ID : AllTileIDs)
		{
			PossibleTiles.Add(ID);
		}
	}
};
