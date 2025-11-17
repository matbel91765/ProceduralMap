// Ultimate Procedural Map Generation System - Cave Decorator
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CellularAutomata.h"
#include "CaveDecorator.generated.h"

/**
 * Prop placement rule
 */
USTRUCT(BlueprintType)
struct FPropPlacementRule
{
	GENERATED_BODY()

	/** Static mesh to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	UStaticMesh* PropMesh;

	/** Material to apply to the prop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	UMaterialInterface* PropMaterial;

	/** Spawn chance (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	float SpawnChance;

	/** Minimum distance between props of this type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	float MinSpacing;

	/** Scale range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	FVector2D ScaleRange;

	/** Random rotation on Z axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	bool bRandomRotation;

	/** Align to surface normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	bool bAlignToSurface;

	/** Only spawn on floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	bool bFloorOnly;

	/** Only spawn on ceiling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	bool bCeilingOnly;

	/** Only spawn on walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	bool bWallsOnly;

	FPropPlacementRule()
		: PropMesh(nullptr)
		, PropMaterial(nullptr)
		, SpawnChance(0.1f)
		, MinSpacing(100.0f)
		, ScaleRange(FVector2D(0.8f, 1.2f))
		, bRandomRotation(true)
		, bAlignToSurface(false)
		, bFloorOnly(false)
		, bCeilingOnly(false)
		, bWallsOnly(false)
	{
	}
};

/**
 * Lighting placement settings
 */
USTRUCT(BlueprintType)
struct FLightingPlacement
{
	GENERATED_BODY()

	/** Light class to spawn (PointLight, SpotLight, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	TSubclassOf<class AActor> LightClass;

	/** Spawn chance per room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	float SpawnChance;

	/** Minimum lights per room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	int32 MinLightsPerRoom;

	/** Maximum lights per room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	int32 MaxLightsPerRoom;

	/** Light intensity range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	FVector2D IntensityRange;

	/** Light color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	FLinearColor LightColor;

	/** Height offset from floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	float HeightOffset;

	FLightingPlacement()
		: LightClass(nullptr)
		, SpawnChance(0.5f)
		, MinLightsPerRoom(1)
		, MaxLightsPerRoom(3)
		, IntensityRange(FVector2D(1000.0f, 3000.0f))
		, LightColor(FLinearColor(1.0f, 0.8f, 0.6f))
		, HeightOffset(150.0f)
	{
	}
};

/**
 * Particle effect placement
 */
USTRUCT(BlueprintType)
struct FParticlePlacement
{
	GENERATED_BODY()

	/** Particle system to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	class UParticleSystem* ParticleSystem;

	/** Niagara system to spawn (UE5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	class UNiagaraSystem* NiagaraSystem;

	/** Spawn chance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	float SpawnChance;

	/** Minimum spacing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	float MinSpacing;

	FParticlePlacement()
		: ParticleSystem(nullptr)
		, NiagaraSystem(nullptr)
		, SpawnChance(0.05f)
		, MinSpacing(200.0f)
	{
	}
};

/**
 * Cave Decorator Component
 * Handles placement of decorations, props, lighting, and effects in caves
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROCEDURALMAP_API UCaveDecorator : public UActorComponent
{
	GENERATED_BODY()

public:
	UCaveDecorator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== Configuration =====

	/** Reference to cellular automata for cave data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	UCellularAutomata* CellularAutomata;

	/** Random seed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	int32 Seed;

	/** Voxel size for position calculations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	float VoxelSize;

	/** Wall height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator")
	float WallHeight;

	// ===== Stalactites/Stalagmites =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Speleothems")
	UStaticMesh* StalactiteMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Speleothems")
	UStaticMesh* StalagmiteMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Speleothems")
	UMaterialInterface* SpeleotemMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Speleothems")
	float StalactiteSpawnChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Speleothems")
	float StalagmiteSpawnChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Speleothems")
	FVector2D StalactiteScaleRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Speleothems")
	FVector2D StalagmiteScaleRange;

	// ===== Props =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Props")
	TArray<FPropPlacementRule> PropRules;

	// ===== Rocks and Debris =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Debris")
	TArray<UStaticMesh*> RockMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Debris")
	UMaterialInterface* RockMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Debris")
	float RockSpawnChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Debris")
	FVector2D RockScaleRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Debris")
	float MinRockSpacing;

	// ===== Lighting =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Lighting")
	TArray<FLightingPlacement> LightingRules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Lighting")
	bool bSpawnLights;

	// ===== Particles =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Particles")
	TArray<FParticlePlacement> ParticleRules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Particles")
	bool bSpawnParticles;

	// ===== Crystals =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Crystals")
	UStaticMesh* CrystalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Crystals")
	UMaterialInterface* CrystalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Crystals")
	float CrystalSpawnChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Crystals")
	FVector2D CrystalScaleRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Crystals")
	bool bCrystalsEmitLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Crystals")
	FLinearColor CrystalLightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Decorator|Crystals")
	float CrystalLightIntensity;

	// ===== Methods =====

	/**
	 * Decorate the entire cave
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	void DecorateCave();

	/**
	 * Place stalactites and stalagmites
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	void PlaceSpeleothems();

	/**
	 * Place props based on rules
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	void PlaceProps();

	/**
	 * Scatter rocks and debris
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	void ScatterDebris();

	/**
	 * Place lights in the cave
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	void PlaceLighting();

	/**
	 * Place particle effects
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	void PlaceParticles();

	/**
	 * Place crystal formations
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	void PlaceCrystals();

	/**
	 * Clear all decorations
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	void ClearDecorations();

	/**
	 * Get world position from grid coordinates
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	FVector GetWorldPosition(int32 X, int32 Y, int32 Z = 0) const;

	/**
	 * Check if surface is floor
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	bool IsFloorSurface(int32 X, int32 Y) const;

	/**
	 * Check if surface is ceiling
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	bool IsCeilingSurface(int32 X, int32 Y) const;

	/**
	 * Check if surface is wall
	 */
	UFUNCTION(BlueprintCallable, Category = "Cave Decorator")
	bool IsWallSurface(int32 X, int32 Y) const;

protected:
	// Random stream
	FRandomStream RandomStream;

	// Spawned decoration actors
	UPROPERTY()
	TArray<class UInstancedStaticMeshComponent*> InstancedMeshComponents;

	UPROPERTY()
	TArray<AActor*> SpawnedActors;

	/**
	 * Spawn a static mesh instance
	 */
	class UInstancedStaticMeshComponent* GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh, UMaterialInterface* Material);

	/**
	 * Check if location is valid for spawning
	 */
	bool IsValidSpawnLocation(FVector Location, float MinDistance) const;

	/**
	 * Get surface normal at location
	 */
	FVector GetSurfaceNormal(int32 X, int32 Y) const;
};
