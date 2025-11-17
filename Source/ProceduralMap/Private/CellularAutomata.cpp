// Ultimate Procedural Map Generation System - Cellular Automata Cave Generation
// Copyright (C) 2025. All Rights Reserved.

#include "CellularAutomata.h"
#include "Math/UnrealMathUtility.h"

UCellularAutomata::UCellularAutomata()
{
	// Default parameters
	MapWidth = 128;
	MapHeight = 128;
	MapDepth = 32;
	FillPercent = 45;
	SmoothingIterations = 5;
	WallThreshold = 4;
	Seed = 12345;
	bUse8Neighbors = true;
	MinRoomSize = 50;
	MinWallSize = 50;
	bConnectAllRooms = true;
	PassageWidth = 3;
}

bool UCellularAutomata::GenerateCaveMap2D()
{
	if (MapWidth <= 0 || MapHeight <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid map dimensions for 2D generation"));
		return false;
	}

	// Initialize random stream
	RandomStream.Initialize(Seed);

	// Initialize map
	CaveMap2D.SetNum(MapWidth * MapHeight);

	// Random fill
	for (int32 X = 0; X < MapWidth; X++)
	{
		for (int32 Y = 0; Y < MapHeight; Y++)
		{
			// Borders are always walls
			if (X == 0 || X == MapWidth - 1 || Y == 0 || Y == MapHeight - 1)
			{
				CaveMap2D[Y * MapWidth + X] = 1;
			}
			else
			{
				CaveMap2D[Y * MapWidth + X] = (RandomStream.FRand() < (FillPercent / 100.0f)) ? 1 : 0;
			}
		}
	}

	// Apply smoothing
	for (int32 i = 0; i < SmoothingIterations; i++)
	{
		SmoothMap2D();
	}

	// Process rooms and connectivity
	ProcessRooms2D();

	UE_LOG(LogTemp, Log, TEXT("Generated 2D cave map: %dx%d"), MapWidth, MapHeight);
	return true;
}

bool UCellularAutomata::GenerateCaveMap3D()
{
	if (MapWidth <= 0 || MapHeight <= 0 || MapDepth <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid map dimensions for 3D generation"));
		return false;
	}

	// Initialize random stream
	RandomStream.Initialize(Seed);

	// Initialize map
	CaveMap3D.SetNum(MapWidth * MapHeight * MapDepth);

	// Random fill
	for (int32 X = 0; X < MapWidth; X++)
	{
		for (int32 Y = 0; Y < MapHeight; Y++)
		{
			for (int32 Z = 0; Z < MapDepth; Z++)
			{
				// Borders are always walls
				if (X == 0 || X == MapWidth - 1 ||
					Y == 0 || Y == MapHeight - 1 ||
					Z == 0 || Z == MapDepth - 1)
				{
					CaveMap3D[Z * (MapWidth * MapHeight) + Y * MapWidth + X] = 1;
				}
				else
				{
					CaveMap3D[Z * (MapWidth * MapHeight) + Y * MapWidth + X] =
						(RandomStream.FRand() < (FillPercent / 100.0f)) ? 1 : 0;
				}
			}
		}
	}

	// Apply smoothing
	for (int32 i = 0; i < SmoothingIterations; i++)
	{
		SmoothMap3D();
	}

	// Process rooms and connectivity
	ProcessRooms3D();

	UE_LOG(LogTemp, Log, TEXT("Generated 3D cave map: %dx%dx%d"), MapWidth, MapHeight, MapDepth);
	return true;
}

void UCellularAutomata::SmoothMap2D()
{
	TArray<int32> NewMap = CaveMap2D;

	for (int32 X = 0; X < MapWidth; X++)
	{
		for (int32 Y = 0; Y < MapHeight; Y++)
		{
			int32 NeighborWallCount = GetNeighborWallCount2D(X, Y);

			// Apply cellular automata rules
			if (NeighborWallCount > WallThreshold)
			{
				NewMap[Y * MapWidth + X] = 1; // Wall
			}
			else if (NeighborWallCount < WallThreshold)
			{
				NewMap[Y * MapWidth + X] = 0; // Empty
			}
		}
	}

	CaveMap2D = NewMap;
}

void UCellularAutomata::SmoothMap3D()
{
	TArray<int32> NewMap = CaveMap3D;

	for (int32 X = 0; X < MapWidth; X++)
	{
		for (int32 Y = 0; Y < MapHeight; Y++)
		{
			for (int32 Z = 0; Z < MapDepth; Z++)
			{
				int32 NeighborWallCount = GetNeighborWallCount3D(X, Y, Z);

				// Apply cellular automata rules (adjusted threshold for 3D - 26 neighbors)
				int32 Threshold3D = 13; // Half of 26 neighbors
				if (NeighborWallCount > Threshold3D)
				{
					NewMap[Z * (MapWidth * MapHeight) + Y * MapWidth + X] = 1; // Wall
				}
				else if (NeighborWallCount < Threshold3D)
				{
					NewMap[Z * (MapWidth * MapHeight) + Y * MapWidth + X] = 0; // Empty
				}
			}
		}
	}

	CaveMap3D = NewMap;
}

int32 UCellularAutomata::GetNeighborWallCount2D(int32 GridX, int32 GridY) const
{
	int32 WallCount = 0;

	if (bUse8Neighbors)
	{
		// 8-neighbor (Moore neighborhood)
		for (int32 NeighborX = GridX - 1; NeighborX <= GridX + 1; NeighborX++)
		{
			for (int32 NeighborY = GridY - 1; NeighborY <= GridY + 1; NeighborY++)
			{
				if (NeighborX >= 0 && NeighborX < MapWidth && NeighborY >= 0 && NeighborY < MapHeight)
				{
					if (NeighborX != GridX || NeighborY != GridY)
					{
						WallCount += CaveMap2D[NeighborY * MapWidth + NeighborX];
					}
				}
				else
				{
					WallCount++; // Out of bounds counts as wall
				}
			}
		}
	}
	else
	{
		// 4-neighbor (Von Neumann neighborhood)
		const int32 DX[] = {0, 1, 0, -1};
		const int32 DY[] = {-1, 0, 1, 0};

		for (int32 i = 0; i < 4; i++)
		{
			int32 NeighborX = GridX + DX[i];
			int32 NeighborY = GridY + DY[i];

			if (NeighborX >= 0 && NeighborX < MapWidth && NeighborY >= 0 && NeighborY < MapHeight)
			{
				WallCount += CaveMap2D[NeighborY * MapWidth + NeighborX];
			}
			else
			{
				WallCount++; // Out of bounds counts as wall
			}
		}
	}

	return WallCount;
}

int32 UCellularAutomata::GetNeighborWallCount3D(int32 GridX, int32 GridY, int32 GridZ) const
{
	int32 WallCount = 0;

	// 26-neighbor cube
	for (int32 NeighborX = GridX - 1; NeighborX <= GridX + 1; NeighborX++)
	{
		for (int32 NeighborY = GridY - 1; NeighborY <= GridY + 1; NeighborY++)
		{
			for (int32 NeighborZ = GridZ - 1; NeighborZ <= GridZ + 1; NeighborZ++)
			{
				if (IsInMapRange(NeighborX, NeighborY, NeighborZ))
				{
					if (NeighborX != GridX || NeighborY != GridY || NeighborZ != GridZ)
					{
						WallCount += CaveMap3D[NeighborZ * (MapWidth * MapHeight) + NeighborY * MapWidth + NeighborX];
					}
				}
				else
				{
					WallCount++; // Out of bounds counts as wall
				}
			}
		}
	}

	return WallCount;
}

int32 UCellularAutomata::GetTile2D(int32 X, int32 Y) const
{
	if (X >= 0 && X < MapWidth && Y >= 0 && Y < MapHeight)
	{
		return CaveMap2D[Y * MapWidth + X];
	}
	return 1; // Wall for out of bounds
}

int32 UCellularAutomata::GetTile3D(int32 X, int32 Y, int32 Z) const
{
	if (IsInMapRange(X, Y, Z))
	{
		return CaveMap3D[Z * (MapWidth * MapHeight) + Y * MapWidth + X];
	}
	return 1; // Wall for out of bounds
}

void UCellularAutomata::SetTile2D(int32 X, int32 Y, int32 Value)
{
	if (X >= 0 && X < MapWidth && Y >= 0 && Y < MapHeight)
	{
		CaveMap2D[Y * MapWidth + X] = Value;
	}
}

void UCellularAutomata::SetTile3D(int32 X, int32 Y, int32 Z, int32 Value)
{
	if (IsInMapRange(X, Y, Z))
	{
		CaveMap3D[Z * (MapWidth * MapHeight) + Y * MapWidth + X] = Value;
	}
}

void UCellularAutomata::ProcessRooms2D()
{
	// Get all room regions (empty spaces)
	TArray<FRoom> RoomRegions = GetRegions2D(0);

	// Filter by size
	SurvivingRooms.Empty();
	for (FRoom& Room : RoomRegions)
	{
		if (Room.RoomSize >= MinRoomSize)
		{
			SurvivingRooms.Add(Room);
		}
		else
		{
			// Fill small rooms with walls
			for (const FCoord3D& Tile : Room.Tiles)
			{
				SetTile2D(Tile.X, Tile.Y, 1);
			}
		}
	}

	// Get wall regions and remove small ones
	TArray<FRoom> WallRegions = GetRegions2D(1);
	for (FRoom& WallRegion : WallRegions)
	{
		if (WallRegion.RoomSize < MinWallSize)
		{
			// Fill small wall regions with empty space
			for (const FCoord3D& Tile : WallRegion.Tiles)
			{
				SetTile2D(Tile.X, Tile.Y, 0);
			}
		}
	}

	// Connect rooms if enabled
	if (bConnectAllRooms && SurvivingRooms.Num() > 0)
	{
		SurvivingRooms.Sort([](const FRoom& A, const FRoom& B) {
			return A.RoomSize > B.RoomSize;
		});
		SurvivingRooms[0].bIsMainRoom = true;
		SurvivingRooms[0].bIsAccessibleFromMainRoom = true;

		ConnectClosestRooms(SurvivingRooms, false);
	}

	UE_LOG(LogTemp, Log, TEXT("Processed %d rooms in 2D cave"), SurvivingRooms.Num());
}

void UCellularAutomata::ProcessRooms3D()
{
	// Get all room regions (empty spaces)
	TArray<FRoom> RoomRegions = GetRegions3D(0);

	// Filter by size
	SurvivingRooms.Empty();
	for (FRoom& Room : RoomRegions)
	{
		if (Room.RoomSize >= MinRoomSize)
		{
			SurvivingRooms.Add(Room);
		}
		else
		{
			// Fill small rooms with walls
			for (const FCoord3D& Tile : Room.Tiles)
			{
				SetTile3D(Tile.X, Tile.Y, Tile.Z, 1);
			}
		}
	}

	// Connect rooms if enabled
	if (bConnectAllRooms && SurvivingRooms.Num() > 0)
	{
		SurvivingRooms.Sort([](const FRoom& A, const FRoom& B) {
			return A.RoomSize > B.RoomSize;
		});
		SurvivingRooms[0].bIsMainRoom = true;
		SurvivingRooms[0].bIsAccessibleFromMainRoom = true;

		ConnectClosestRooms(SurvivingRooms, false);
	}

	UE_LOG(LogTemp, Log, TEXT("Processed %d rooms in 3D cave"), SurvivingRooms.Num());
}

TArray<FCoord3D> UCellularAutomata::GetRegionTiles2D(int32 StartX, int32 StartY)
{
	TArray<FCoord3D> Tiles;
	TArray<int32> MapFlags;
	MapFlags.SetNumZeroed(MapWidth * MapHeight);

	int32 TileType = GetTile2D(StartX, StartY);

	TArray<FCoord3D> Queue;
	Queue.Add(FCoord3D(StartX, StartY, 0));
	MapFlags[StartY * MapWidth + StartX] = 1;

	while (Queue.Num() > 0)
	{
		FCoord3D Tile = Queue.Pop();
		Tiles.Add(Tile);

		// Check 4 neighbors
		const int32 DX[] = {0, 1, 0, -1};
		const int32 DY[] = {-1, 0, 1, 0};

		for (int32 i = 0; i < 4; i++)
		{
			int32 X = Tile.X + DX[i];
			int32 Y = Tile.Y + DY[i];

			if (IsInMapRange(X, Y) && MapFlags[Y * MapWidth + X] == 0 && GetTile2D(X, Y) == TileType)
			{
				MapFlags[Y * MapWidth + X] = 1;
				Queue.Add(FCoord3D(X, Y, 0));
			}
		}
	}

	return Tiles;
}

TArray<FCoord3D> UCellularAutomata::GetRegionTiles3D(int32 StartX, int32 StartY, int32 StartZ)
{
	TArray<FCoord3D> Tiles;
	TArray<int32> MapFlags;
	MapFlags.SetNumZeroed(MapWidth * MapHeight * MapDepth);

	int32 TileType = GetTile3D(StartX, StartY, StartZ);

	TArray<FCoord3D> Queue;
	Queue.Add(FCoord3D(StartX, StartY, StartZ));
	MapFlags[StartZ * (MapWidth * MapHeight) + StartY * MapWidth + StartX] = 1;

	while (Queue.Num() > 0)
	{
		FCoord3D Tile = Queue.Pop();
		Tiles.Add(Tile);

		// Check 6 neighbors (face-connected in 3D)
		const int32 DX[] = {1, -1, 0, 0, 0, 0};
		const int32 DY[] = {0, 0, 1, -1, 0, 0};
		const int32 DZ[] = {0, 0, 0, 0, 1, -1};

		for (int32 i = 0; i < 6; i++)
		{
			int32 X = Tile.X + DX[i];
			int32 Y = Tile.Y + DY[i];
			int32 Z = Tile.Z + DZ[i];

			if (IsInMapRange(X, Y, Z))
			{
				int32 Index = Z * (MapWidth * MapHeight) + Y * MapWidth + X;
				if (MapFlags[Index] == 0 && GetTile3D(X, Y, Z) == TileType)
				{
					MapFlags[Index] = 1;
					Queue.Add(FCoord3D(X, Y, Z));
				}
			}
		}
	}

	return Tiles;
}

TArray<FRoom> UCellularAutomata::GetRegions2D(int32 TileType)
{
	TArray<FRoom> Regions;
	TArray<int32> MapFlags;
	MapFlags.SetNumZeroed(MapWidth * MapHeight);

	for (int32 X = 0; X < MapWidth; X++)
	{
		for (int32 Y = 0; Y < MapHeight; Y++)
		{
			if (MapFlags[Y * MapWidth + X] == 0 && GetTile2D(X, Y) == TileType)
			{
				FRoom NewRoom;
				NewRoom.Tiles = GetRegionTiles2D(X, Y);
				NewRoom.RoomSize = NewRoom.Tiles.Num();

				// Mark as visited
				for (const FCoord3D& Tile : NewRoom.Tiles)
				{
					MapFlags[Tile.Y * MapWidth + Tile.X] = 1;
				}

				// Find edge tiles
				for (const FCoord3D& Tile : NewRoom.Tiles)
				{
					const int32 DX[] = {0, 1, 0, -1};
					const int32 DY[] = {-1, 0, 1, 0};

					for (int32 i = 0; i < 4; i++)
					{
						int32 NeighborX = Tile.X + DX[i];
						int32 NeighborY = Tile.Y + DY[i];

						if (IsInMapRange(NeighborX, NeighborY) && GetTile2D(NeighborX, NeighborY) != TileType)
						{
							NewRoom.EdgeTiles.Add(Tile);
							break;
						}
					}
				}

				Regions.Add(NewRoom);
			}
		}
	}

	return Regions;
}

TArray<FRoom> UCellularAutomata::GetRegions3D(int32 TileType)
{
	TArray<FRoom> Regions;
	TArray<int32> MapFlags;
	MapFlags.SetNumZeroed(MapWidth * MapHeight * MapDepth);

	for (int32 X = 0; X < MapWidth; X++)
	{
		for (int32 Y = 0; Y < MapHeight; Y++)
		{
			for (int32 Z = 0; Z < MapDepth; Z++)
			{
				int32 Index = Z * (MapWidth * MapHeight) + Y * MapWidth + X;
				if (MapFlags[Index] == 0 && GetTile3D(X, Y, Z) == TileType)
				{
					FRoom NewRoom;
					NewRoom.Tiles = GetRegionTiles3D(X, Y, Z);
					NewRoom.RoomSize = NewRoom.Tiles.Num();

					// Mark as visited
					for (const FCoord3D& Tile : NewRoom.Tiles)
					{
						MapFlags[Tile.Z * (MapWidth * MapHeight) + Tile.Y * MapWidth + Tile.X] = 1;
					}

					Regions.Add(NewRoom);
				}
			}
		}
	}

	return Regions;
}

void UCellularAutomata::ConnectClosestRooms(TArray<FRoom>& AllRooms, bool bForceAccessibilityFromMainRoom)
{
	TArray<FRoom> RoomListA;
	TArray<FRoom> RoomListB;

	if (bForceAccessibilityFromMainRoom)
	{
		for (FRoom& Room : AllRooms)
		{
			if (Room.bIsAccessibleFromMainRoom)
			{
				RoomListB.Add(Room);
			}
			else
			{
				RoomListA.Add(Room);
			}
		}
	}
	else
	{
		RoomListA = AllRooms;
		RoomListB = AllRooms;
	}

	int32 BestDistance = INT_MAX;
	FCoord3D BestTileA;
	FCoord3D BestTileB;
	FRoom* BestRoomA = nullptr;
	FRoom* BestRoomB = nullptr;
	bool bPossibleConnectionFound = false;

	for (FRoom& RoomA : RoomListA)
	{
		if (!bForceAccessibilityFromMainRoom)
		{
			bPossibleConnectionFound = false;
			if (RoomA.bIsAccessibleFromMainRoom)
			{
				continue;
			}
		}

		for (FRoom& RoomB : RoomListB)
		{
			if (&RoomA == &RoomB || RoomA.bIsAccessibleFromMainRoom == RoomB.bIsAccessibleFromMainRoom)
			{
				continue;
			}

			for (const FCoord3D& TileA : RoomA.EdgeTiles)
			{
				for (const FCoord3D& TileB : RoomB.EdgeTiles)
				{
					int32 DistanceX = TileA.X - TileB.X;
					int32 DistanceY = TileA.Y - TileB.Y;
					int32 DistanceZ = TileA.Z - TileB.Z;
					int32 DistanceSqr = DistanceX * DistanceX + DistanceY * DistanceY + DistanceZ * DistanceZ;

					if (DistanceSqr < BestDistance || !bPossibleConnectionFound)
					{
						BestDistance = DistanceSqr;
						bPossibleConnectionFound = true;
						BestTileA = TileA;
						BestTileB = TileB;
						BestRoomA = &RoomA;
						BestRoomB = &RoomB;
					}
				}
			}
		}

		if (bPossibleConnectionFound && !bForceAccessibilityFromMainRoom)
		{
			CreatePassage(*BestRoomA, *BestRoomB, BestTileA, BestTileB);
		}
	}

	if (bPossibleConnectionFound && bForceAccessibilityFromMainRoom)
	{
		CreatePassage(*BestRoomA, *BestRoomB, BestTileA, BestTileB);
		ConnectClosestRooms(AllRooms, true);
	}

	if (!bForceAccessibilityFromMainRoom)
	{
		ConnectClosestRooms(AllRooms, true);
	}
}

void UCellularAutomata::CreatePassage(FRoom& RoomA, FRoom& RoomB, FCoord3D TileA, FCoord3D TileB)
{
	RoomA.bIsAccessibleFromMainRoom = true;
	RoomB.bIsAccessibleFromMainRoom = true;

	TArray<FCoord3D> Line = GetLine(TileA, TileB);
	for (const FCoord3D& Coord : Line)
	{
		DrawCircle(Coord, PassageWidth);
	}
}

void UCellularAutomata::DrawCircle(FCoord3D Coord, int32 Radius)
{
	for (int32 X = -Radius; X <= Radius; X++)
	{
		for (int32 Y = -Radius; Y <= Radius; Y++)
		{
			if (X * X + Y * Y <= Radius * Radius)
			{
				int32 DrawX = Coord.X + X;
				int32 DrawY = Coord.Y + Y;

				if (MapDepth > 0 && CaveMap3D.Num() > 0)
				{
					SetTile3D(DrawX, DrawY, Coord.Z, 0);
				}
				else
				{
					SetTile2D(DrawX, DrawY, 0);
				}
			}
		}
	}
}

TArray<FCoord3D> UCellularAutomata::GetLine(FCoord3D From, FCoord3D To)
{
	TArray<FCoord3D> Line;

	int32 X = From.X;
	int32 Y = From.Y;

	int32 DX = To.X - From.X;
	int32 DY = To.Y - From.Y;

	bool bInverted = false;
	int32 Step = FMath::Sign(DX);
	int32 GradientStep = FMath::Sign(DY);

	int32 Longest = FMath::Abs(DX);
	int32 Shortest = FMath::Abs(DY);

	if (Longest < Shortest)
	{
		bInverted = true;
		Longest = FMath::Abs(DY);
		Shortest = FMath::Abs(DX);

		Step = FMath::Sign(DY);
		GradientStep = FMath::Sign(DX);
	}

	int32 GradientAccumulation = Longest / 2;
	for (int32 i = 0; i < Longest; i++)
	{
		Line.Add(FCoord3D(X, Y, From.Z));

		if (bInverted)
		{
			Y += Step;
		}
		else
		{
			X += Step;
		}

		GradientAccumulation += Shortest;
		if (GradientAccumulation >= Longest)
		{
			if (bInverted)
			{
				X += GradientStep;
			}
			else
			{
				Y += GradientStep;
			}
			GradientAccumulation -= Longest;
		}
	}

	return Line;
}

bool UCellularAutomata::IsInMapRange(int32 X, int32 Y, int32 Z) const
{
	if (Z == 0 && MapDepth == 0)
	{
		// 2D check
		return X >= 0 && X < MapWidth && Y >= 0 && Y < MapHeight;
	}
	else
	{
		// 3D check
		return X >= 0 && X < MapWidth && Y >= 0 && Y < MapHeight && Z >= 0 && Z < MapDepth;
	}
}
