// Ultimate Procedural Map Generation System - Cave Mesh Generator Utilities
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CaveMeshGenerator.generated.h"

/**
 * Marching Squares configuration for a cell
 */
struct FMarchingSquaresCell
{
	int32 Config;
	FVector ControlNodes[4];
	FVector CenterTop;
	FVector CenterRight;
	FVector CenterBottom;
	FVector CenterLeft;

	FMarchingSquaresCell(int32 InConfig, FVector* Nodes, float SquareSize)
		: Config(InConfig)
	{
		for (int32 i = 0; i < 4; i++)
		{
			ControlNodes[i] = Nodes[i];
		}

		CenterTop = (ControlNodes[0] + ControlNodes[1]) * 0.5f;
		CenterRight = (ControlNodes[1] + ControlNodes[2]) * 0.5f;
		CenterBottom = (ControlNodes[2] + ControlNodes[3]) * 0.5f;
		CenterLeft = (ControlNodes[3] + ControlNodes[0]) * 0.5f;
	}
};

/**
 * Marching Cubes edge table and triangle table
 * Based on Paul Bourke's algorithm
 */
class FMarchingCubesTable
{
public:
	static const int32 EdgeTable[256];
	static const int32 TriTable[256][16];
};

/**
 * Cave Mesh Generator Utilities
 * Contains static methods for generating cave meshes
 */
UCLASS()
class PROCEDURALMAP_API UCaveMeshGenerator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Generate mesh using Marching Squares algorithm (2D)
	 */
	static void GenerateMarchingSquaresMesh(
		const TArray<int32>& Map,
		int32 Width,
		int32 Height,
		float SquareSize,
		float WallHeight,
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles,
		TArray<FVector>& OutNormals,
		TArray<FVector2D>& OutUVs
	);

	/**
	 * Generate mesh using Marching Cubes algorithm (3D)
	 */
	static void GenerateMarchingCubesMesh(
		const TArray<int32>& Map,
		int32 Width,
		int32 Height,
		int32 Depth,
		float VoxelSize,
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles,
		TArray<FVector>& OutNormals,
		TArray<FVector2D>& OutUVs
	);

	/**
	 * Add a cube to the mesh
	 */
	static void AddCube(
		FVector Center,
		float Size,
		float Height,
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles,
		TArray<FVector>& OutNormals,
		TArray<FVector2D>& OutUVs
	);

	/**
	 * Create a cone mesh (for stalactites/stalagmites)
	 */
	static void CreateCone(
		FVector BasePosition,
		FVector Direction,
		float Height,
		float BaseRadius,
		int32 Segments,
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles,
		TArray<FVector>& OutNormals,
		TArray<FVector2D>& OutUVs
	);

	/**
	 * Triangulate a marching squares cell
	 */
	static void TriangulateMarchingSquaresCell(
		const FMarchingSquaresCell& Cell,
		float WallHeight,
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles
	);

	/**
	 * Get interpolated vertex position for marching cubes
	 */
	static FVector GetVertexForEdge(int32 Edge, const FVector Corners[8], const int32 Values[8]);

	/**
	 * Get grid value at position
	 */
	static int32 GetGridValue(const TArray<int32>& Map, int32 X, int32 Y, int32 Z, int32 Width, int32 Height, int32 Depth);

	/**
	 * Calculate vertex normal based on gradient
	 */
	static FVector CalculateNormal(const TArray<int32>& Map, int32 X, int32 Y, int32 Z, int32 Width, int32 Height, int32 Depth);

	/**
	 * Merge duplicate vertices
	 */
	static void MergeVertices(
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UVs,
		float Threshold = 0.01f
	);

	/**
	 * Generate planar UVs for vertices
	 */
	static void GeneratePlanarUVs(
		const TArray<FVector>& Vertices,
		TArray<FVector2D>& OutUVs,
		float UVScale = 1.0f
	);

	/**
	 * Optimize mesh by removing degenerate triangles
	 */
	static void OptimizeMesh(
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UVs
	);

protected:
	/**
	 * Add triangle with vertices
	 */
	static void AddTriangle(
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles
	);

	/**
	 * Add quad (two triangles)
	 */
	static void AddQuad(
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		const FVector& V3,
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles
	);
};
