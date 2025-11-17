// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

/**
 * FQuadTreeNode - Structure representing a node in the QuadTree
 * Each node represents a square region of terrain with a specific LOD level
 */
struct FQuadTreeNode
{
public:
	// Center position of this node in 2D space
	FVector2D Center;

	// Size of this node (width/height of the square)
	float Size;

	// LOD level (0 = highest detail, higher = lower detail)
	int32 LODLevel;

	// Pointers to four children (NW, NE, SW, SE)
	TSharedPtr<FQuadTreeNode> Children[4];

	// Is this a leaf node (no children)?
	bool bIsLeaf;

	// Procedural mesh component for rendering this node
	UProceduralMeshComponent* MeshComponent;

	// Parent node reference (weak pointer to avoid circular references)
	TWeakPtr<FQuadTreeNode> Parent;

	// Bounds for frustum culling
	FBox2D Bounds;

	// Flag to check if mesh needs regeneration
	bool bNeedsUpdate;

	// Last known distance to camera (for hysteresis)
	float LastCameraDistance;

	// Timestamp of last split/merge operation (for stability)
	float LastUpdateTime;

	// Constructor
	FQuadTreeNode()
		: Center(FVector2D::ZeroVector)
		, Size(0.0f)
		, LODLevel(0)
		, bIsLeaf(true)
		, MeshComponent(nullptr)
		, bNeedsUpdate(true)
		, LastCameraDistance(0.0f)
		, LastUpdateTime(0.0f)
	{
		for (int32 i = 0; i < 4; ++i)
		{
			Children[i] = nullptr;
		}
		Bounds = FBox2D(ForceInit);
	}

	// Constructor with parameters
	FQuadTreeNode(const FVector2D& InCenter, float InSize, int32 InLODLevel)
		: Center(InCenter)
		, Size(InSize)
		, LODLevel(InLODLevel)
		, bIsLeaf(true)
		, MeshComponent(nullptr)
		, bNeedsUpdate(true)
		, LastCameraDistance(0.0f)
		, LastUpdateTime(0.0f)
	{
		for (int32 i = 0; i < 4; ++i)
		{
			Children[i] = nullptr;
		}

		// Calculate bounds
		FVector2D HalfSize(Size * 0.5f, Size * 0.5f);
		Bounds = FBox2D(Center - HalfSize, Center + HalfSize);
	}

	// Destructor
	~FQuadTreeNode()
	{
		// Clean up mesh component if it exists
		if (MeshComponent && MeshComponent->IsValidLowLevel())
		{
			MeshComponent->ClearAllMeshSections();
			MeshComponent->DestroyComponent();
			MeshComponent = nullptr;
		}

		// Clear children
		for (int32 i = 0; i < 4; ++i)
		{
			Children[i].Reset();
		}
	}

	/**
	 * Check if this node has any children
	 */
	bool HasChildren() const
	{
		return !bIsLeaf && Children[0].IsValid();
	}

	/**
	 * Get the quadrant index for a given position
	 * 0 = NW, 1 = NE, 2 = SW, 3 = SE
	 */
	int32 GetQuadrant(const FVector2D& Position) const
	{
		bool bEast = Position.X >= Center.X;
		bool bNorth = Position.Y >= Center.Y;

		if (bNorth)
		{
			return bEast ? 1 : 0; // NE : NW
		}
		else
		{
			return bEast ? 3 : 2; // SE : SW
		}
	}

	/**
	 * Calculate center position for a child quadrant
	 */
	FVector2D GetChildCenter(int32 QuadrantIndex) const
	{
		float QuarterSize = Size * 0.25f;

		switch (QuadrantIndex)
		{
		case 0: // NW
			return Center + FVector2D(-QuarterSize, QuarterSize);
		case 1: // NE
			return Center + FVector2D(QuarterSize, QuarterSize);
		case 2: // SW
			return Center + FVector2D(-QuarterSize, -QuarterSize);
		case 3: // SE
			return Center + FVector2D(QuarterSize, -QuarterSize);
		default:
			return Center;
		}
	}

	/**
	 * Get memory usage of this node and its children (in bytes)
	 */
	SIZE_T GetMemoryUsage() const
	{
		SIZE_T TotalSize = sizeof(FQuadTreeNode);

		if (HasChildren())
		{
			for (int32 i = 0; i < 4; ++i)
			{
				if (Children[i].IsValid())
				{
					TotalSize += Children[i]->GetMemoryUsage();
				}
			}
		}

		return TotalSize;
	}

	/**
	 * Count total number of nodes (including children)
	 */
	int32 CountNodes() const
	{
		int32 Count = 1; // This node

		if (HasChildren())
		{
			for (int32 i = 0; i < 4; ++i)
			{
				if (Children[i].IsValid())
				{
					Count += Children[i]->CountNodes();
				}
			}
		}

		return Count;
	}

	/**
	 * Count leaf nodes only
	 */
	int32 CountLeafNodes() const
	{
		if (bIsLeaf)
		{
			return 1;
		}

		int32 Count = 0;
		if (HasChildren())
		{
			for (int32 i = 0; i < 4; ++i)
			{
				if (Children[i].IsValid())
				{
					Count += Children[i]->CountLeafNodes();
				}
			}
		}

		return Count;
	}
};
