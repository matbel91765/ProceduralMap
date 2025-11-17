// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTile.h"

/**
 * Example tile set configurations and helper functions
 * This class provides examples of how to create custom tile sets for WFC
 */
class PROCEDURALMAP_API FWFCExamples
{
public:
	/**
	 * Example 1: Simple 2-tile binary pattern (wall/floor)
	 * Perfect for testing basic WFC functionality
	 */
	static TArray<FWFCTile> CreateSimpleBinaryTiles()
	{
		TArray<FWFCTile> Tiles;

		// Tile 0: Floor (common)
		FWFCTile Floor(0, 5.0f);
		Floor.TileName = TEXT("Floor");
		// Floor can connect to anything
		for (int32 Dir = 0; Dir < 4; Dir++)
		{
			Floor.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 0);
			Floor.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 1);
		}
		Tiles.Add(Floor);

		// Tile 1: Wall (rare)
		FWFCTile Wall(1, 1.0f);
		Wall.TileName = TEXT("Wall");
		// Wall can also connect to anything
		for (int32 Dir = 0; Dir < 4; Dir++)
		{
			Wall.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 0);
			Wall.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 1);
		}
		Tiles.Add(Wall);

		return Tiles;
	}

	/**
	 * Example 2: Pipe system with strict connectivity rules
	 * Demonstrates complex adjacency constraints
	 */
	static TArray<FWFCTile> CreatePipeSystem()
	{
		TArray<FWFCTile> Tiles;

		// 0: Empty
		FWFCTile Empty(0, 3.0f);
		Empty.TileName = TEXT("Empty");
		Empty.AddValidNeighbor(EWFCDirection::North, 0);
		Empty.AddValidNeighbor(EWFCDirection::East, 0);
		Empty.AddValidNeighbor(EWFCDirection::South, 0);
		Empty.AddValidNeighbor(EWFCDirection::West, 0);
		Tiles.Add(Empty);

		// 1: Vertical pipe (connects North-South)
		FWFCTile Vertical(1, 1.0f);
		Vertical.TileName = TEXT("Pipe_Vertical");
		Vertical.AddValidNeighbor(EWFCDirection::North, 1);
		Vertical.AddValidNeighbor(EWFCDirection::North, 3);
		Vertical.AddValidNeighbor(EWFCDirection::North, 4);
		Vertical.AddValidNeighbor(EWFCDirection::North, 5);
		Vertical.AddValidNeighbor(EWFCDirection::East, 0);
		Vertical.AddValidNeighbor(EWFCDirection::South, 1);
		Vertical.AddValidNeighbor(EWFCDirection::South, 3);
		Vertical.AddValidNeighbor(EWFCDirection::South, 4);
		Vertical.AddValidNeighbor(EWFCDirection::South, 5);
		Vertical.AddValidNeighbor(EWFCDirection::West, 0);
		Tiles.Add(Vertical);

		// 2: Horizontal pipe (connects East-West)
		FWFCTile Horizontal(2, 1.0f);
		Horizontal.TileName = TEXT("Pipe_Horizontal");
		Horizontal.AddValidNeighbor(EWFCDirection::North, 0);
		Horizontal.AddValidNeighbor(EWFCDirection::East, 2);
		Horizontal.AddValidNeighbor(EWFCDirection::East, 3);
		Horizontal.AddValidNeighbor(EWFCDirection::East, 4);
		Horizontal.AddValidNeighbor(EWFCDirection::East, 5);
		Horizontal.AddValidNeighbor(EWFCDirection::South, 0);
		Horizontal.AddValidNeighbor(EWFCDirection::West, 2);
		Horizontal.AddValidNeighbor(EWFCDirection::West, 3);
		Horizontal.AddValidNeighbor(EWFCDirection::West, 4);
		Horizontal.AddValidNeighbor(EWFCDirection::West, 5);
		Tiles.Add(Horizontal);

		// 3: T-Junction (North, East, West)
		FWFCTile TJunction(3, 0.3f);
		TJunction.TileName = TEXT("Pipe_T_Junction");
		TJunction.AddValidNeighbor(EWFCDirection::North, 1);
		TJunction.AddValidNeighbor(EWFCDirection::North, 3);
		TJunction.AddValidNeighbor(EWFCDirection::North, 4);
		TJunction.AddValidNeighbor(EWFCDirection::North, 5);
		TJunction.AddValidNeighbor(EWFCDirection::East, 2);
		TJunction.AddValidNeighbor(EWFCDirection::East, 3);
		TJunction.AddValidNeighbor(EWFCDirection::East, 4);
		TJunction.AddValidNeighbor(EWFCDirection::East, 5);
		TJunction.AddValidNeighbor(EWFCDirection::South, 0);
		TJunction.AddValidNeighbor(EWFCDirection::West, 2);
		TJunction.AddValidNeighbor(EWFCDirection::West, 3);
		TJunction.AddValidNeighbor(EWFCDirection::West, 4);
		TJunction.AddValidNeighbor(EWFCDirection::West, 5);
		Tiles.Add(TJunction);

		// 4: L-Corner (North-East)
		FWFCTile Corner(4, 0.5f);
		Corner.TileName = TEXT("Pipe_Corner_NE");
		Corner.AddValidNeighbor(EWFCDirection::North, 1);
		Corner.AddValidNeighbor(EWFCDirection::North, 3);
		Corner.AddValidNeighbor(EWFCDirection::North, 4);
		Corner.AddValidNeighbor(EWFCDirection::North, 5);
		Corner.AddValidNeighbor(EWFCDirection::East, 2);
		Corner.AddValidNeighbor(EWFCDirection::East, 3);
		Corner.AddValidNeighbor(EWFCDirection::East, 4);
		Corner.AddValidNeighbor(EWFCDirection::East, 5);
		Corner.AddValidNeighbor(EWFCDirection::South, 0);
		Corner.AddValidNeighbor(EWFCDirection::West, 0);
		Tiles.Add(Corner);

		// 5: Cross (all directions)
		FWFCTile Cross(5, 0.1f);
		Cross.TileName = TEXT("Pipe_Cross");
		Cross.AddValidNeighbor(EWFCDirection::North, 1);
		Cross.AddValidNeighbor(EWFCDirection::North, 3);
		Cross.AddValidNeighbor(EWFCDirection::North, 4);
		Cross.AddValidNeighbor(EWFCDirection::North, 5);
		Cross.AddValidNeighbor(EWFCDirection::East, 2);
		Cross.AddValidNeighbor(EWFCDirection::East, 3);
		Cross.AddValidNeighbor(EWFCDirection::East, 4);
		Cross.AddValidNeighbor(EWFCDirection::East, 5);
		Cross.AddValidNeighbor(EWFCDirection::South, 1);
		Cross.AddValidNeighbor(EWFCDirection::South, 3);
		Cross.AddValidNeighbor(EWFCDirection::South, 4);
		Cross.AddValidNeighbor(EWFCDirection::South, 5);
		Cross.AddValidNeighbor(EWFCDirection::West, 2);
		Cross.AddValidNeighbor(EWFCDirection::West, 3);
		Cross.AddValidNeighbor(EWFCDirection::West, 4);
		Cross.AddValidNeighbor(EWFCDirection::West, 5);
		Tiles.Add(Cross);

		return Tiles;
	}

	/**
	 * Example 3: Platformer level tiles
	 * Ground, platform, air with proper physics connectivity
	 */
	static TArray<FWFCTile> CreatePlatformerTiles()
	{
		TArray<FWFCTile> Tiles;

		// 0: Air (most common)
		FWFCTile Air(0, 10.0f);
		Air.TileName = TEXT("Air");
		for (int32 Dir = 0; Dir < 4; Dir++)
		{
			Air.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 0);
			Air.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 1);
			Air.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 2);
		}
		Tiles.Add(Air);

		// 1: Ground (solid block)
		FWFCTile Ground(1, 3.0f);
		Ground.TileName = TEXT("Ground");
		Ground.AddValidNeighbor(EWFCDirection::North, 0);
		Ground.AddValidNeighbor(EWFCDirection::North, 1);
		Ground.AddValidNeighbor(EWFCDirection::North, 2);
		Ground.AddValidNeighbor(EWFCDirection::East, 1);
		Ground.AddValidNeighbor(EWFCDirection::East, 2);
		Ground.AddValidNeighbor(EWFCDirection::South, 1);
		Ground.AddValidNeighbor(EWFCDirection::South, 2);
		Ground.AddValidNeighbor(EWFCDirection::West, 1);
		Ground.AddValidNeighbor(EWFCDirection::West, 2);
		Tiles.Add(Ground);

		// 2: Platform (one-way)
		FWFCTile Platform(2, 1.0f);
		Platform.TileName = TEXT("Platform");
		Platform.AddValidNeighbor(EWFCDirection::North, 0);
		Platform.AddValidNeighbor(EWFCDirection::East, 0);
		Platform.AddValidNeighbor(EWFCDirection::East, 2);
		Platform.AddValidNeighbor(EWFCDirection::South, 1);
		Platform.AddValidNeighbor(EWFCDirection::West, 0);
		Platform.AddValidNeighbor(EWFCDirection::West, 2);
		Tiles.Add(Platform);

		return Tiles;
	}

	/**
	 * Example 4: City block generator
	 * Buildings, roads, parks, with urban planning rules
	 */
	static TArray<FWFCTile> CreateCityBlocks()
	{
		TArray<FWFCTile> Tiles;

		// 0: Park/Open space
		FWFCTile Park(0, 2.0f);
		Park.TileName = TEXT("Park");
		for (int32 Dir = 0; Dir < 4; Dir++)
		{
			Park.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 0);
			Park.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 1);
			Park.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 2);
			Park.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 3);
		}
		Tiles.Add(Park);

		// 1: Residential building
		FWFCTile Residential(1, 3.0f);
		Residential.TileName = TEXT("Residential");
		for (int32 Dir = 0; Dir < 4; Dir++)
		{
			Residential.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 0);
			Residential.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 1);
			Residential.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 2);
			Residential.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 3);
		}
		Tiles.Add(Residential);

		// 2: Commercial building
		FWFCTile Commercial(2, 2.0f);
		Commercial.TileName = TEXT("Commercial");
		for (int32 Dir = 0; Dir < 4; Dir++)
		{
			Commercial.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 0);
			Commercial.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 1);
			Commercial.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 2);
			Commercial.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 3);
		}
		Tiles.Add(Commercial);

		// 3: Main road
		FWFCTile Road(3, 1.5f);
		Road.TileName = TEXT("Main_Road");
		for (int32 Dir = 0; Dir < 4; Dir++)
		{
			Road.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 0);
			Road.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 1);
			Road.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 2);
			Road.AddValidNeighbor(static_cast<EWFCDirection>(Dir), 3);
		}
		Tiles.Add(Road);

		return Tiles;
	}
};
