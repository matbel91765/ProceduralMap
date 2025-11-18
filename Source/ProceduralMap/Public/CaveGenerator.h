// Ultimate Procedural Map Generation System - Cave Generator Actor
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "CellularAutomata.h"
#include "CaveGenerator.generated.h"

/**
 * Mesh generation mode
 */
UENUM(BlueprintType)
enum class ECaveMeshMode : uint8
{
	MarchingSquares2D UMETA(DisplayName = "Marching Squares (2D)"),
	MarchingCubes3D UMETA(DisplayName = "Marching Cubes (3D)"),
	SimpleBlocks UMETA(DisplayName = "Simple Voxel Blocks")
};

/**
 * Resource placement information
 */
USTRUCT(BlueprintType)
struct FResourcePlacement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Resources")
	TSubclassOf<AActor> ResourceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Resources")
	float SpawnChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Resources")
	int32 MinPerRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Resources")
	int32 MaxPerRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Resources")
	float MinDistanceBetween;

	FResourcePlacement()
		: ResourceClass(nullptr)
		, SpawnChance(0.5f)
		, MinPerRoom(1)
		, MaxPerRoom(5)
		, MinDistanceBetween(200.0f)
	{
	}
};

/**
 * Cave Generator Actor
 * Generates 3D cave meshes from cellular automata data
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAP_API ACaveGenerator : public AActor
{
	GENERATED_BODY()

public:
	ACaveGenerator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ===== Components =====

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Generator")
	UProceduralMeshComponent* CaveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Generator")
	USceneComponent* RootSceneComponent;

	// ===== Cellular Automata =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
	UCellularAutomata* CellularAutomata;

	// ===== Generation Settings =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
	ECaveMeshMode MeshMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
	bool bGenerateOnBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
	float VoxelSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
	float WallHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
	bool bGenerateCollision;

	// ===== Materials =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Materials")
	UMaterialInterface* CaveMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Materials")
	UMaterialInterface* FloorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Materials")
	UMaterialInterface* CeilingMaterial;

	// ===== Stalactites/Stalagmites =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
	bool bGenerateStalactites;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
	float StalactiteChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
	float StalactiteMinHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
	float StalactiteMaxHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
	bool bGenerateStalagmites;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
	float StalagmiteChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
	float StalagmiteMinHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
	float StalagmiteMaxHeight;

	// ===== Resources =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Resources")
	TArray<FResourcePlacement> ResourceTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Resources")
	bool bSpawnResources;

	// ===== Generation Methods =====

	/**
	 * Generate the complete cave system
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	void GenerateCave();

	/**
	 * Generate cave mesh from cellular automata data
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	void GenerateCaveMesh();

	/**
	 * Generate 2D cave mesh using Marching Squares
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	void GenerateMarchingSquares2D();

	/**
	 * Generate 3D cave mesh using Marching Cubes
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	void GenerateMarchingCubes3D();

	/**
	 * Generate simple voxel block mesh
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	void GenerateSimpleBlockMesh();

	/**
	 * Generate stalactites and stalagmites
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	void GenerateSpeleothems();

	/**
	 * Spawn resources in the cave
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	void SpawnResources();

	/**
	 * Clear all generated geometry
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	void ClearCave();

	/**
	 * Get world position from grid coordinates
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	FVector GetWorldPosition(int32 X, int32 Y, int32 Z = 0) const;

	/**
	 * Get grid coordinates from world position
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Generator")
	FIntVector GetGridPosition(FVector WorldPosition) const;

protected:
	// Random stream for reproducible generation
	FRandomStream RandomStream;

	// Spawned decoration actors
	UPROPERTY()
	TArray<AActor*> SpawnedDecorations;

	/**
	 * Create stalactite mesh
	 */
	void CreateStalactite(FVector Position, float Height);

	/**
	 * Create stalagmite mesh
	 */
	void CreateStalagmite(FVector Position, float Height);

	/**
	 * Check if position is valid for resource spawn
	 */
	bool IsValidSpawnLocation(FVector Location, float MinDistance) const;

	/**
	 * Calculate smooth normals for mesh
	 */
	void CalculateSmoothNormals(TArray<FVector>& Vertices, const TArray<int32>& Triangles, TArray<FVector>& Normals);

	/**
	 * Generate UVs for mesh
	 */
	void GenerateUVs(const TArray<FVector>& Vertices, TArray<FVector2D>& UVs);
};
