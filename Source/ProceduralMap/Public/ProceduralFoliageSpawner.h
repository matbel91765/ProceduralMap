// ProceduralFoliageSpawner.h - Professional Procedural Foliage Spawning System
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/DataAsset.h"
#include "ProceduralFoliageSpawner.generated.h"

class UFoliageDistribution;
class UVegetationLayer;

/**
 * Foliage type configuration structure
 */
USTRUCT(BlueprintType)
struct FFoliageTypeConfig
{
	GENERATED_BODY()

	/** Static mesh to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* StaticMesh = nullptr;

	/** Instances per square meter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Density = 1.0f;

	/** Minimum scale multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float MinScale = 0.8f;

	/** Maximum scale multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float MaxScale = 1.2f;

	/** Enable random rotation on Z axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bRandomRotation = true;

	/** Align mesh to terrain normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bAlignToNormal = true;

	/** Maximum alignment angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxAlignmentAngle = 45.0f;

	/** Biome mask - spawn only in these biomes (empty = all biomes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	TArray<FName> BiomeMask;

	/** Minimum spawn height (world Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height")
	float MinHeight = -100000.0f;

	/** Maximum spawn height (world Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height")
	float MaxHeight = 100000.0f;

	/** Minimum slope angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MinSlope = 0.0f;

	/** Maximum slope angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxSlope = 45.0f;

	/** Minimum distance from water (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float MinDistanceFromWater = 0.0f;

	/** Maximum distance from water (cm, 0 = ignore) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float MaxDistanceFromWater = 0.0f;

	/** Spawn weight for multi-species distribution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnWeight = 1.0f;

	/** Enable collision for spawned instances */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bEnableCollision = true;

	/** Cast shadows */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bCastShadow = true;

	/** Custom culling distance (0 = use default) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	float CullDistance = 0.0f;

	FFoliageTypeConfig()
	{
	}
};

/**
 * Spawn result structure
 */
USTRUCT(BlueprintType)
struct FFoliageSpawnResult
{
	GENERATED_BODY()

	/** Number of instances spawned */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 InstancesSpawned = 0;

	/** Number of spawn attempts */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 SpawnAttempts = 0;

	/** Time taken to spawn (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float SpawnTime = 0.0f;

	/** Success rate (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float SuccessRate = 0.0f;
};

/**
 * Professional Procedural Foliage Spawner Actor
 * Spawns and manages foliage using HISM for optimal performance
 */
UCLASS(Blueprintable, ClassGroup = "ProceduralMap")
class PROCEDURALMAP_API AProceduralFoliageSpawner : public AActor
{
	GENERATED_BODY()

public:
	AProceduralFoliageSpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ============================================
	// Configuration
	// ============================================

	/** Foliage types to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	TArray<FFoliageTypeConfig> FoliageTypes;

	/** Vegetation layer data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	UVegetationLayer* VegetationLayer;

	/** Foliage distribution algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Distribution")
	UFoliageDistribution* DistributionAlgorithm;

	/** Spawn on begin play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bSpawnOnBeginPlay = false;

	/** Auto-update when properties change in editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bAutoUpdate = false;

	/** Random seed for reproducible spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 RandomSeed = 12345;

	/** Spawn radius from actor location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "100.0"))
	float SpawnRadius = 10000.0f;

	/** Grid cell size for spatial optimization (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	float GridCellSize = 1000.0f;

	/** Maximum instances per HISM component (split to multiple if exceeded) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
	int32 MaxInstancesPerComponent = 1000;

	/** Enable distance-based culling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	bool bEnableDistanceCulling = true;

	/** Default culling distance (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
	float DefaultCullDistance = 20000.0f;

	/** Debug draw spawn points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugDraw = false;

	/** Debug draw duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugDrawDuration = 5.0f;

	// ============================================
	// Public Methods
	// ============================================

	/**
	 * Spawn foliage on terrain within spawn radius
	 * @param bClearExisting - Clear existing foliage before spawning
	 * @return Spawn result statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Foliage")
	FFoliageSpawnResult SpawnFoliageOnTerrain(bool bClearExisting = true);

	/**
	 * Spawn foliage in a specific radius around a point
	 * @param Center - World location center point
	 * @param Radius - Spawn radius in cm
	 * @param bClearExisting - Clear existing foliage before spawning
	 * @return Spawn result statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Foliage")
	FFoliageSpawnResult SpawnFoliageInRadius(FVector Center, float Radius, bool bClearExisting = true);

	/**
	 * Spawn foliage in a rectangular bounds
	 * @param Bounds - World space bounding box
	 * @param bClearExisting - Clear existing foliage before spawning
	 * @return Spawn result statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Foliage")
	FFoliageSpawnResult SpawnFoliageInBounds(FBox Bounds, bool bClearExisting = true);

	/**
	 * Clear all spawned foliage instances
	 */
	UFUNCTION(BlueprintCallable, Category = "Foliage")
	void ClearAllFoliage();

	/**
	 * Calculate foliage density at a specific location based on biome and terrain
	 * @param Location - World location
	 * @param FoliageType - Foliage type configuration
	 * @return Density multiplier (0-1)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Foliage")
	float CalculateFoliageDensity(const FVector& Location, const FFoliageTypeConfig& FoliageType) const;

	/**
	 * Check if spawn conditions are met at a location
	 * @param Location - World location
	 * @param Normal - Surface normal
	 * @param FoliageType - Foliage type configuration
	 * @return true if spawn conditions are valid
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Foliage")
	bool CheckSpawnConditions(const FVector& Location, const FVector& Normal, const FFoliageTypeConfig& FoliageType) const;

	/**
	 * Get total instance count across all HISM components
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Foliage")
	int32 GetTotalInstanceCount() const;

	/**
	 * Get instance count for a specific foliage type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Foliage")
	int32 GetInstanceCountForType(int32 FoliageTypeIndex) const;

	/**
	 * Update culling distances for all HISM components
	 */
	UFUNCTION(BlueprintCallable, Category = "Optimization")
	void UpdateCullingDistances();

protected:
	// ============================================
	// Internal Methods
	// ============================================

	/** Create HISM components for foliage types */
	void CreateHISMComponents();

	/** Get or create HISM component for foliage type */
	UHierarchicalInstancedStaticMeshComponent* GetOrCreateHISMComponent(int32 FoliageTypeIndex, int32 ComponentIndex = 0);

	/** Trace to find ground at location */
	bool TraceGround(const FVector& Location, FVector& OutHitLocation, FVector& OutHitNormal) const;

	/** Check height constraints */
	bool CheckHeightConstraints(const FVector& Location, const FFoliageTypeConfig& FoliageType) const;

	/** Check slope constraints */
	bool CheckSlopeConstraints(const FVector& Normal, const FFoliageTypeConfig& FoliageType) const;

	/** Check biome constraints */
	bool CheckBiomeConstraints(const FVector& Location, const FFoliageTypeConfig& FoliageType) const;

	/** Check water distance constraints */
	bool CheckWaterDistanceConstraints(const FVector& Location, const FFoliageTypeConfig& FoliageType) const;

	/** Generate random scale */
	FVector GenerateRandomScale(const FFoliageTypeConfig& FoliageType) const;

	/** Generate rotation from normal */
	FRotator GenerateRotation(const FVector& Normal, const FFoliageTypeConfig& FoliageType) const;

	/** Add instance to HISM component */
	void AddFoliageInstance(int32 FoliageTypeIndex, const FTransform& Transform);

	/** Debug draw spawn point */
	void DebugDrawSpawnPoint(const FVector& Location, const FVector& Normal, bool bSuccess) const;

protected:
	// ============================================
	// Internal Data
	// ============================================

	/** HISM components for each foliage type (can have multiple per type) */
	UPROPERTY()
	TMap<int32, TArray<UHierarchicalInstancedStaticMeshComponent*>> HISMComponents;

	/** Random stream for reproducible spawning */
	FRandomStream RandomStream;

	/** Last spawn result */
	UPROPERTY()
	FFoliageSpawnResult LastSpawnResult;

	/** Cached water locations for distance checks */
	TArray<FVector> CachedWaterLocations;
};
