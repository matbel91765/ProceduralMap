// VegetationLayer.h - Vegetation Layer Data Asset
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VegetationLayer.generated.h"

/**
 * Vegetation layer type
 */
UENUM(BlueprintType)
enum class EVegetationLayerType : uint8
{
	/** Ground cover (grass, flowers, small plants) */
	GroundCover UMETA(DisplayName = "Ground Cover"),

	/** Bushes and shrubs */
	Bushes UMETA(DisplayName = "Bushes"),

	/** Trees */
	Trees UMETA(DisplayName = "Trees"),

	/** Rocks and boulders */
	Rocks UMETA(DisplayName = "Rocks"),

	/** Custom vegetation */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * LOD configuration for vegetation
 */
USTRUCT(BlueprintType)
struct FVegetationLODConfig
{
	GENERATED_BODY()

	/** LOD 0 distance (highest quality) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float LOD0Distance = 2000.0f;

	/** LOD 1 distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float LOD1Distance = 5000.0f;

	/** LOD 2 distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float LOD2Distance = 10000.0f;

	/** Maximum draw distance (culled beyond this) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0"))
	float MaxDrawDistance = 20000.0f;

	/** Enable LOD dithering for smooth transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bEnableLODDithering = true;

	FVegetationLODConfig()
	{
	}
};

/**
 * Vegetation type configuration
 */
USTRUCT(BlueprintType)
struct FVegetationTypeConfig
{
	GENERATED_BODY()

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName TypeName = NAME_None;

	/** Static mesh to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* StaticMesh = nullptr;

	/** Alternative meshes for variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TArray<UStaticMesh*> VariationMeshes;

	/** Spawn weight (higher = more common) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
	float SpawnWeight = 1.0f;

	/** Base density (instances per square meter) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BaseDensity = 1.0f;

	/** Density variation (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DensityVariation = 0.3f;

	/** Minimum spacing between instances (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "1.0"))
	float MinimumSpacing = 100.0f;

	/** Clustering factor (0=uniform, 1=highly clustered) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClusteringFactor = 0.5f;

	/** Minimum scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.1"))
	float MinScale = 0.8f;

	/** Maximum scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.1"))
	float MaxScale = 1.2f;

	/** Enable uniform scaling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale")
	bool bUniformScaling = true;

	/** Scale distribution curve (0=min, 1=max) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale")
	UCurveFloat* ScaleDistributionCurve = nullptr;

	/** Random rotation on Z axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bRandomRotation = true;

	/** Align to surface normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bAlignToNormal = true;

	/** Maximum alignment angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxAlignmentAngle = 45.0f;

	/** Random rotation variation (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float RotationVariation = 0.0f;

	/** Enable collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bEnableCollision = true;

	/** Collision complexity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryOnly;

	/** Cast shadows */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bCastShadow = true;

	/** Cast dynamic shadows */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bCastDynamicShadow = true;

	/** Receive decals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bReceiveDecals = true;

	/** Affect distance field lighting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bAffectDistanceFieldLighting = true;

	/** Override materials */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	TArray<UMaterialInterface*> OverrideMaterials;

	/** LOD configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	FVegetationLODConfig LODConfig;

	/** Biome restrictions (empty = all biomes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions")
	TArray<FName> AllowedBiomes;

	/** Minimum height (world Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions")
	float MinHeight = -100000.0f;

	/** Maximum height (world Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions")
	float MaxHeight = 100000.0f;

	/** Minimum slope (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MinSlope = 0.0f;

	/** Maximum slope (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxSlope = 45.0f;

	/** Prefer certain slope ranges (optional curve) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions")
	UCurveFloat* SlopePreferenceCurve = nullptr;

	/** Minimum distance from water (cm, 0=ignore) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (ClampMin = "0.0"))
	float MinDistanceFromWater = 0.0f;

	/** Maximum distance from water (cm, 0=ignore) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (ClampMin = "0.0"))
	float MaxDistanceFromWater = 0.0f;

	/** Prefer proximity to water (0-1 curve based on distance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	UCurveFloat* WaterProximityCurve = nullptr;

	FVegetationTypeConfig()
	{
	}

	/** Get random mesh (base or variation) */
	UStaticMesh* GetRandomMesh(FRandomStream& RandomStream) const
	{
		if (VariationMeshes.Num() > 0 && RandomStream.FRand() > 0.5f)
		{
			int32 Index = RandomStream.RandRange(0, VariationMeshes.Num() - 1);
			if (VariationMeshes[Index])
			{
				return VariationMeshes[Index];
			}
		}
		return StaticMesh;
	}

	/** Get random scale */
	FVector GetRandomScale(FRandomStream& RandomStream) const
	{
		float Scale = RandomStream.FRandRange(MinScale, MaxScale);

		if (ScaleDistributionCurve)
		{
			float RandomValue = RandomStream.FRand();
			Scale = FMath::Lerp(MinScale, MaxScale, ScaleDistributionCurve->GetFloatValue(RandomValue));
		}

		if (bUniformScaling)
		{
			return FVector(Scale);
		}
		else
		{
			return FVector(
				RandomStream.FRandRange(MinScale, MaxScale),
				RandomStream.FRandRange(MinScale, MaxScale),
				RandomStream.FRandRange(MinScale, MaxScale)
			);
		}
	}
};

/**
 * Vegetation Layer Data Asset
 * Defines layers of vegetation types (ground cover, bushes, trees, rocks)
 */
UCLASS(Blueprintable, ClassGroup = "ProceduralMap")
class PROCEDURALMAP_API UVegetationLayer : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UVegetationLayer();

	// ============================================
	// Configuration
	// ============================================

	/** Layer name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName LayerName = TEXT("VegetationLayer");

	/** Layer type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	EVegetationLayerType LayerType = EVegetationLayerType::Trees;

	/** Layer description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic", meta = (MultiLine = true))
	FString Description;

	/** Vegetation types in this layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	TArray<FVegetationTypeConfig> VegetationTypes;

	/** Overall layer density multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float LayerDensityMultiplier = 1.0f;

	/** Enable this layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
	bool bEnabled = true;

	/** Layer priority (higher = rendered last) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
	int32 LayerPriority = 0;

	/** Minimum distance between different species (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Competition", meta = (ClampMin = "0.0"))
	float MinInterSpeciesDistance = 100.0f;

	/** Enable species competition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Competition")
	bool bEnableSpeciesCompetition = false;

	/** Dominant species indices (get spawned first) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Competition")
	TArray<int32> DominantSpeciesIndices;

	// ============================================
	// Methods
	// ============================================

	/**
	 * Get random vegetation type based on spawn weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Vegetation")
	int32 GetRandomVegetationType(FRandomStream& RandomStream) const;

	/**
	 * Get total spawn weight
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation")
	float GetTotalSpawnWeight() const;

	/**
	 * Get vegetation type by name
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation")
	int32 GetVegetationTypeByName(FName TypeName) const;

	/**
	 * Calculate effective density at location (considering all modifiers)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vegetation")
	float CalculateEffectiveDensity(int32 TypeIndex, const FVector& Location, float BaseDensity) const;

	/**
	 * Check if vegetation type can spawn at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Vegetation")
	bool CanSpawnAtLocation(int32 TypeIndex, const FVector& Location, const FVector& Normal) const;

	/**
	 * Get all enabled vegetation types
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vegetation")
	TArray<int32> GetEnabledVegetationTypes() const;

	/**
	 * Get vegetation types for specific biome
	 */
	UFUNCTION(BlueprintCallable, Category = "Vegetation")
	TArray<int32> GetVegetationTypesForBiome(FName BiomeName) const;

	/**
	 * Validate vegetation layer configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateConfiguration(TArray<FString>& OutErrors) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	/** Cached total spawn weight */
	UPROPERTY()
	mutable float CachedTotalSpawnWeight = -1.0f;

	/** Recalculate cached values */
	void RecalculateCache() const;
};
