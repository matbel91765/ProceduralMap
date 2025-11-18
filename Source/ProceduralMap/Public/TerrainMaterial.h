// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Engine/Texture2D.h"
#include "TerrainMaterial.generated.h"

/**
 * Texture layer configuration for terrain materials
 */
USTRUCT(BlueprintType)
struct FTerrainTextureLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* BaseColorTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* NormalMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* RoughnessMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* DisplacementMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* HeightMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float TilingScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float TriplanarBlendSharpness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FLinearColor TintColor;

	FTerrainTextureLayer()
		: BaseColorTexture(nullptr)
		, NormalMap(nullptr)
		, RoughnessMap(nullptr)
		, DisplacementMap(nullptr)
		, HeightMap(nullptr)
		, TilingScale(1.0f)
		, TriplanarBlendSharpness(8.0f)
		, TintColor(FLinearColor::White)
	{
	}
};

/**
 * Biome-specific material configuration
 */
USTRUCT(BlueprintType)
struct FBiomeMaterialConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	FString BiomeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
	TArray<FTerrainTextureLayer> TextureLayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float GlobalTriplanarBlendSharpness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	bool bUseWorldAlignedTexturing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	bool bEnableDistanceFading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float FadeStartDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float FadeEndDistance;

	FBiomeMaterialConfig()
		: BiomeName(TEXT("Default"))
		, GlobalTriplanarBlendSharpness(8.0f)
		, bUseWorldAlignedTexturing(true)
		, bEnableDistanceFading(false)
		, FadeStartDistance(5000.0f)
		, FadeEndDistance(10000.0f)
	{
	}
};

/**
 * Terrain Material Generator
 * Handles dynamic material instance creation and configuration for terrain with triplanar mapping
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAP_API UTerrainMaterialGenerator : public UObject
{
	GENERATED_BODY()

public:
	UTerrainMaterialGenerator();

	/**
	 * Generate a dynamic material instance with configured texture layers
	 * @param BaseMaterial - The base material to create instance from
	 * @param BiomeConfig - Biome-specific material configuration
	 * @return The generated material instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Material")
	UMaterialInstanceDynamic* GenerateMaterialInstance(UMaterial* BaseMaterial, const FBiomeMaterialConfig& BiomeConfig);

	/**
	 * Update an existing material instance with new texture layers
	 * @param MaterialInstance - Material instance to update
	 * @param LayerIndex - Index of the layer to update
	 * @param Layer - New layer configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Material")
	void UpdateTextureLayer(UMaterialInstanceDynamic* MaterialInstance, int32 LayerIndex, const FTerrainTextureLayer& Layer);

	/**
	 * Configure triplanar mapping parameters for a material instance
	 * @param MaterialInstance - Material instance to configure
	 * @param BlendSharpness - Blend sharpness for triplanar projection
	 * @param bWorldAligned - Whether to use world-aligned or object-aligned texturing
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Material")
	void ConfigureTriplanarMapping(UMaterialInstanceDynamic* MaterialInstance, float BlendSharpness, bool bWorldAligned);

	/**
	 * Set up distance-based detail fading
	 * @param MaterialInstance - Material instance to configure
	 * @param StartDistance - Distance where fading starts
	 * @param EndDistance - Distance where fading ends
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Material")
	void SetupDistanceFading(UMaterialInstanceDynamic* MaterialInstance, float StartDistance, float EndDistance);

	/**
	 * Apply biome configuration to multiple texture layers
	 * @param MaterialInstance - Material instance to configure
	 * @param BiomeConfig - Biome configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Material")
	void ApplyBiomeConfiguration(UMaterialInstanceDynamic* MaterialInstance, const FBiomeMaterialConfig& BiomeConfig);

	/**
	 * Get the maximum number of supported texture layers
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain Material")
	int32 GetMaxTextureLayers() const { return MaxTextureLayers; }

protected:
	/** Maximum number of texture layers supported */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	int32 MaxTextureLayers;

	/** Default triplanar blend sharpness */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float DefaultBlendSharpness;

	/** Default tiling scale */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float DefaultTilingScale;

private:
	/**
	 * Set texture parameter on material instance
	 */
	void SetTextureParameter(UMaterialInstanceDynamic* MaterialInstance, const FName& ParameterName, UTexture2D* Texture);

	/**
	 * Set scalar parameter on material instance
	 */
	void SetScalarParameter(UMaterialInstanceDynamic* MaterialInstance, const FName& ParameterName, float Value);

	/**
	 * Set vector parameter on material instance
	 */
	void SetVectorParameter(UMaterialInstanceDynamic* MaterialInstance, const FName& ParameterName, const FLinearColor& Value);

	/**
	 * Generate parameter names for a specific layer
	 */
	FName GetLayerParameterName(const FString& BaseParameterName, int32 LayerIndex) const;
};
