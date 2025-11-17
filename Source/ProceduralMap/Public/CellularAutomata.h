// Ultimate Procedural Map Generation System - Cellular Automata Cave Generation
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CellularAutomata.generated.h"

/**
 * Structure representing a 3D coordinate
 */
USTRUCT(BlueprintType)
struct FCoord3D
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cellular Automata")
	int32 X;

	UPROPERTY(BlueprintReadWrite, Category = "Cellular Automata")
	int32 Y;

	UPROPERTY(BlueprintReadWrite, Category = "Cellular Automata")
	int32 Z;

	FCoord3D() : X(0), Y(0), Z(0) {}
	FCoord3D(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

	bool operator==(const FCoord3D& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}

	friend uint32 GetTypeHash(const FCoord3D& Coord)
	{
		return HashCombine(HashCombine(GetTypeHash(Coord.X), GetTypeHash(Coord.Y)), GetTypeHash(Coord.Z));
	}
};

/**
 * Structure representing a room/cavern region
 */
USTRUCT(BlueprintType)
struct FRoom
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cellular Automata")
	TArray<FCoord3D> Tiles;

	UPROPERTY(BlueprintReadWrite, Category = "Cellular Automata")
	TArray<FCoord3D> EdgeTiles;

	UPROPERTY(BlueprintReadWrite, Category = "Cellular Automata")
	int32 RoomSize;

	UPROPERTY(BlueprintReadWrite, Category = "Cellular Automata")
	bool bIsAccessibleFromMainRoom;

	UPROPERTY(BlueprintReadWrite, Category = "Cellular Automata")
	bool bIsMainRoom;

	FRoom() : RoomSize(0), bIsAccessibleFromMainRoom(false), bIsMainRoom(false) {}
};

/**
 * Cellular Automata Cave Generation System
 * Supports both 2D and 3D cave generation using cellular automata rules
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAP_API UCellularAutomata : public UObject
{
	GENERATED_BODY()

public:
	UCellularAutomata();

	// ===== Configuration Parameters =====

	/** Width of the map (X axis) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Dimensions")
	int32 MapWidth;

	/** Height of the map (Y axis) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Dimensions")
	int32 MapHeight;

	/** Depth of the map (Z axis) - used for 3D generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Dimensions")
	int32 MapDepth;

	/** Percentage of map initially filled with walls (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Generation", meta = (ClampMin = "0", ClampMax = "100"))
	int32 FillPercent;

	/** Number of smoothing iterations to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Generation", meta = (ClampMin = "0", ClampMax = "20"))
	int32 SmoothingIterations;

	/** Minimum number of neighbors to become/stay a wall */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Generation", meta = (ClampMin = "1", ClampMax = "8"))
	int32 WallThreshold;

	/** Random seed for reproducible generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Generation")
	int32 Seed;

	/** Use 8-neighbor rule (true) or 4-neighbor rule (false) for 2D */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Generation")
	bool bUse8Neighbors;

	/** Minimum room size to keep (smaller rooms are removed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Rooms")
	int32 MinRoomSize;

	/** Minimum wall region size to keep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Rooms")
	int32 MinWallSize;

	/** Connect all rooms to ensure full accessibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Rooms")
	bool bConnectAllRooms;

	/** Width of passages between rooms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cellular Automata|Rooms", meta = (ClampMin = "1", ClampMax = "10"))
	int32 PassageWidth;

	// ===== 2D Cave Map =====
	UPROPERTY(BlueprintReadOnly, Category = "Cellular Automata|Data")
	TArray<int32> CaveMap2D; // Flattened array: Index = Y * Width + X

	// ===== 3D Cave Map =====
	UPROPERTY(BlueprintReadOnly, Category = "Cellular Automata|Data")
	TArray<int32> CaveMap3D; // Flattened array: Index = Z * (Width * Height) + Y * Width + X

	// ===== Generation Methods =====

	/**
	 * Generate a 2D cave map using cellular automata
	 * @return True if generation was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	bool GenerateCaveMap2D();

	/**
	 * Generate a 3D voxel cave map using cellular automata
	 * @return True if generation was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	bool GenerateCaveMap3D();

	/**
	 * Apply smoothing iterations to the 2D map
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	void SmoothMap2D();

	/**
	 * Apply smoothing iterations to the 3D map
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	void SmoothMap3D();

	/**
	 * Get value at specific 2D coordinate
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	int32 GetTile2D(int32 X, int32 Y) const;

	/**
	 * Get value at specific 3D coordinate
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	int32 GetTile3D(int32 X, int32 Y, int32 Z) const;

	/**
	 * Set value at specific 2D coordinate
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	void SetTile2D(int32 X, int32 Y, int32 Value);

	/**
	 * Set value at specific 3D coordinate
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	void SetTile3D(int32 X, int32 Y, int32 Z, int32 Value);

	/**
	 * Process rooms and ensure connectivity
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	void ProcessRooms2D();

	/**
	 * Process rooms in 3D
	 */
	UFUNCTION(BlueprintCallable, Category = "Cellular Automata")
	void ProcessRooms3D();

protected:
	// Random stream for reproducible generation
	FRandomStream RandomStream;

	// Detected rooms
	TArray<FRoom> SurvivingRooms;

	// ===== Helper Methods =====

	/**
	 * Count neighboring walls for a 2D cell
	 */
	int32 GetNeighborWallCount2D(int32 GridX, int32 GridY) const;

	/**
	 * Count neighboring walls for a 3D cell (26-neighbor cube)
	 */
	int32 GetNeighborWallCount3D(int32 GridX, int32 GridY, int32 GridZ) const;

	/**
	 * Flood fill to detect room regions in 2D
	 */
	TArray<FCoord3D> GetRegionTiles2D(int32 StartX, int32 StartY);

	/**
	 * Flood fill to detect room regions in 3D
	 */
	TArray<FCoord3D> GetRegionTiles3D(int32 StartX, int32 StartY, int32 StartZ);

	/**
	 * Get all room regions of a specific tile type
	 */
	TArray<FRoom> GetRegions2D(int32 TileType);

	/**
	 * Get all room regions in 3D
	 */
	TArray<FRoom> GetRegions3D(int32 TileType);

	/**
	 * Connect the closest rooms
	 */
	void ConnectClosestRooms(TArray<FRoom>& AllRooms, bool bForceAccessibilityFromMainRoom = false);

	/**
	 * Create a passage between two rooms
	 */
	void CreatePassage(FRoom& RoomA, FRoom& RoomB, FCoord3D TileA, FCoord3D TileB);

	/**
	 * Draw a circle/sphere to create passage
	 */
	void DrawCircle(FCoord3D Coord, int32 Radius);

	/**
	 * Get line between two points
	 */
	TArray<FCoord3D> GetLine(FCoord3D From, FCoord3D To);

	/**
	 * Check if coordinate is within map bounds
	 */
	bool IsInMapRange(int32 X, int32 Y, int32 Z = 0) const;
};
