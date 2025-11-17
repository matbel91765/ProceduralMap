// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TerrainGenerationTask.h"
#include "BiomeConfig.h"
#include "TerrainPreset.generated.h"

/**
 * UTerrainPreset
 * Data asset for storing terrain generation presets
 * Can be saved/loaded from Content Browser
 */
UCLASS(BlueprintType)
class PROCEDURALMAPEDITOR_API UTerrainPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	UTerrainPreset();

	// ========================================================================
	// Preset Metadata
	// ========================================================================

	/** Preset display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FString PresetName;

	/** Preset description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset", meta = (MultiLine = true))
	FString Description;

	/** Preset author */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FString Author;

	/** Preset version */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	int32 Version;

	/** Preset tags for categorization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	TArray<FString> Tags;

	/** Preview thumbnail (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	UTexture2D* PreviewThumbnail;

	// ========================================================================
	// Generation Parameters
	// ========================================================================

	/** Terrain generation parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FTerrainGenerationParams GenerationParams;

	/** Primary biome type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	EBiomeType BiomeType;

	/** Terrain size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "1000", ClampMax = "1000000"))
	float TerrainSize;

	/** Max LOD levels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxLODLevels;

	/** Mesh resolution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "8", ClampMax = "256"))
	int32 MeshResolution;

	// ========================================================================
	// Erosion Settings
	// ========================================================================

	/** Enable erosion simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
	bool bEnableErosion;

	/** Erosion iterations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0", ClampMax = "100000", EditCondition = "bEnableErosion"))
	int32 ErosionIterations;

	/** Erosion strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "10.0", EditCondition = "bEnableErosion"))
	float ErosionStrength;

	/** Enable thermal erosion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
	bool bEnableThermalErosion;

	/** Thermal erosion iterations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0", ClampMax = "10000", EditCondition = "bEnableThermalErosion"))
	int32 ThermalErosionIterations;

	// ========================================================================
	// Biome Settings
	// ========================================================================

	/** Enable multi-biome blending */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	bool bEnableBiomeBlending;

	/** Biome blend distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (ClampMin = "0.0", ClampMax = "1000.0", EditCondition = "bEnableBiomeBlending"))
	float BiomeBlendDistance;

	/** Secondary biomes to blend */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	TArray<EBiomeType> SecondaryBiomes;

	// ========================================================================
	// Foliage Settings
	// ========================================================================

	/** Enable foliage spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	bool bEnableFoliage;

	/** Foliage density multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage", meta = (ClampMin = "0.0", ClampMax = "10.0", EditCondition = "bEnableFoliage"))
	float FoliageDensityMultiplier;

	// ========================================================================
	// Water Settings
	// ========================================================================

	/** Enable water generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	bool bEnableWater;

	/** Water level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (ClampMin = "-1000.0", ClampMax = "1000.0", EditCondition = "bEnableWater"))
	float WaterLevel;

	/** Enable rivers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	bool bEnableRivers;

	/** Number of rivers to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (ClampMin = "0", ClampMax = "100", EditCondition = "bEnableRivers"))
	int32 NumRivers;

	// ========================================================================
	// Advanced Settings
	// ========================================================================

	/** Use GPU acceleration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bUseGPUAcceleration;

	/** Enable async generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bEnableAsyncGeneration;

	/** Custom material override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	UMaterialInterface* CustomMaterial;

	// ========================================================================
	// Public API
	// ========================================================================

	/**
	 * Apply this preset to a terrain component
	 */
	UFUNCTION(BlueprintCallable, Category = "Preset")
	void ApplyToTerrain(class UQuadTreeTerrain* Terrain);

	/**
	 * Create a copy of this preset
	 */
	UFUNCTION(BlueprintCallable, Category = "Preset")
	UTerrainPreset* DuplicatePreset();

	/**
	 * Validate preset parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Preset")
	bool ValidatePreset(FString& OutErrorMessage) const;

	/**
	 * Get preset summary string
	 */
	UFUNCTION(BlueprintCallable, Category = "Preset")
	FString GetPresetSummary() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
