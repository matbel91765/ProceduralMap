// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "Landscape.h"
#include "SplatMapGenerator.generated.h"

/**
 * Splat layer generation rule
 */
UENUM(BlueprintType)
enum class ESplatGenerationRule : uint8
{
	Height UMETA(DisplayName = "Height Based"),
	Slope UMETA(DisplayName = "Slope Based"),
	Biome UMETA(DisplayName = "Biome Based"),
	Humidity UMETA(DisplayName = "Humidity Based"),
	CustomMask UMETA(DisplayName = "Custom Mask"),
	Combined UMETA(DisplayName = "Combined Rules")
};

/**
 * Splat layer configuration
 */
USTRUCT(BlueprintType)
struct FSplatLayerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splat Layer")
	FString LayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splat Layer")
	ESplatGenerationRule GenerationRule;

	// Height-based parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height")
	float MinHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height")
	float MaxHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height")
	float HeightBlendRange;

	// Slope-based parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	float MinSlope;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	float MaxSlope;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	float SlopeBlendRange;

	// Biome/Humidity parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	int32 BiomeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Humidity")
	float MinHumidity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Humidity")
	float MaxHumidity;

	// Custom mask
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
	UTexture2D* CustomMask;

	// Height-based blending
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
	bool bUseHeightBasedBlending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
	UTexture2D* HeightTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
	float HeightBlendStrength;

	// Layer weight multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
	float LayerWeight;

	FSplatLayerConfig()
		: LayerName(TEXT("Layer"))
		, GenerationRule(ESplatGenerationRule::Height)
		, MinHeight(0.0f)
		, MaxHeight(100.0f)
		, HeightBlendRange(10.0f)
		, MinSlope(0.0f)
		, MaxSlope(90.0f)
		, SlopeBlendRange(5.0f)
		, BiomeID(0)
		, MinHumidity(0.0f)
		, MaxHumidity(1.0f)
		, CustomMask(nullptr)
		, bUseHeightBasedBlending(false)
		, HeightTexture(nullptr)
		, HeightBlendStrength(1.0f)
		, LayerWeight(1.0f)
	{
	}
};

/**
 * Splat map generation parameters
 */
USTRUCT(BlueprintType)
struct FSplatMapGenerationParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 TextureWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 TextureHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	TArray<FSplatLayerConfig> Layers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UTexture2D* HeightMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UTexture2D* BiomeMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UTexture2D* HumidityMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normalization")
	bool bNormalizeLayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normalization")
	bool bUseHeightBasedBlending;

	FSplatMapGenerationParams()
		: TextureWidth(1024)
		, TextureHeight(1024)
		, HeightMap(nullptr)
		, BiomeMap(nullptr)
		, HumidityMap(nullptr)
		, bNormalizeLayers(true)
		, bUseHeightBasedBlending(false)
	{
	}
};

/**
 * Splat Map Generator
 * Generates texture splat maps for terrain based on various rules
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAP_API USplatMapGenerator : public UObject
{
	GENERATED_BODY()

public:
	USplatMapGenerator();

	/**
	 * Generate a splat map texture based on parameters
	 * @param Params - Generation parameters including layers and rules
	 * @return Generated splat map texture (RGBA for 4 layers per texture)
	 */
	UFUNCTION(BlueprintCallable, Category = "Splat Map")
	UTexture2D* GenerateSplatMap(const FSplatMapGenerationParams& Params);

	/**
	 * Generate multiple splat map textures for more than 4 layers
	 * @param Params - Generation parameters
	 * @return Array of splat map textures (each supports 4 layers via RGBA)
	 */
	UFUNCTION(BlueprintCallable, Category = "Splat Map")
	TArray<UTexture2D*> GenerateMultipleSplatMaps(const FSplatMapGenerationParams& Params);

	/**
	 * Calculate layer weight at a specific position
	 * @param X - X coordinate (0-1 normalized)
	 * @param Y - Y coordinate (0-1 normalized)
	 * @param LayerConfig - Layer configuration
	 * @param Params - Generation parameters
	 * @return Layer weight (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Splat Map")
	float CalculateLayerWeight(
		float X,
		float Y,
		const FSplatLayerConfig& LayerConfig,
		const FSplatMapGenerationParams& Params
	);

	/**
	 * Blend splat layers using height-based blending
	 * @param LayerWeights - Array of layer weights
	 * @param HeightValues - Array of height values for each layer
	 * @param BlendStrength - Strength of height-based blending
	 * @return Blended layer weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Splat Map")
	TArray<float> BlendSplatLayers(
		const TArray<float>& LayerWeights,
		const TArray<float>& HeightValues,
		float BlendStrength
	);

	/**
	 * Apply splat map to a landscape actor
	 * @param Landscape - Target landscape
	 * @param SplatMaps - Array of splat map textures
	 * @param LayerNames - Names of landscape layers to apply to
	 * @return Success status
	 */
	UFUNCTION(BlueprintCallable, Category = "Splat Map")
	bool ApplyToLandscape(
		ALandscape* Landscape,
		const TArray<UTexture2D*>& SplatMaps,
		const TArray<FString>& LayerNames
	);

	/**
	 * Calculate slope at a specific position from height map
	 * @param HeightMap - Input height map
	 * @param X - X coordinate
	 * @param Y - Y coordinate
	 * @param PixelWidth - Width of a pixel in world units
	 * @return Slope in degrees
	 */
	UFUNCTION(BlueprintPure, Category = "Splat Map")
	static float CalculateSlope(UTexture2D* HeightMap, int32 X, int32 Y, float PixelWidth = 1.0f);

	/**
	 * Sample texture at normalized coordinates (0-1)
	 * @param Texture - Texture to sample
	 * @param U - U coordinate (0-1)
	 * @param V - V coordinate (0-1)
	 * @return Sampled color
	 */
	UFUNCTION(BlueprintPure, Category = "Splat Map")
	static FLinearColor SampleTexture(UTexture2D* Texture, float U, float V);

	/**
	 * Normalize layer weights to sum to 1.0
	 * @param Weights - Input weights
	 * @return Normalized weights
	 */
	UFUNCTION(BlueprintPure, Category = "Splat Map")
	static TArray<float> NormalizeWeights(const TArray<float>& Weights);

	/**
	 * Create a texture from raw pixel data
	 * @param Width - Texture width
	 * @param Height - Texture height
	 * @param PixelData - Raw RGBA pixel data
	 * @param TextureName - Name for the texture
	 * @return Created texture
	 */
	UFUNCTION(BlueprintCallable, Category = "Splat Map")
	UTexture2D* CreateTextureFromData(
		int32 Width,
		int32 Height,
		const TArray<FColor>& PixelData,
		const FString& TextureName
	);

protected:
	/**
	 * Calculate height-based weight
	 */
	float CalculateHeightWeight(float Height, const FSplatLayerConfig& Config);

	/**
	 * Calculate slope-based weight
	 */
	float CalculateSlopeWeight(float Slope, const FSplatLayerConfig& Config);

	/**
	 * Calculate biome-based weight
	 */
	float CalculateBiomeWeight(int32 BiomeID, const FSplatLayerConfig& Config);

	/**
	 * Calculate humidity-based weight
	 */
	float CalculateHumidityWeight(float Humidity, const FSplatLayerConfig& Config);

	/**
	 * Smooth blend function (smoothstep)
	 */
	static float SmoothBlend(float Edge0, float Edge1, float Value);

	/**
	 * Get pixel from texture data
	 */
	FColor GetPixel(UTexture2D* Texture, int32 X, int32 Y);

	/**
	 * Get normalized height value from height map
	 */
	float GetHeightValue(UTexture2D* HeightMap, float U, float V);

private:
	/** Cache for texture mip data */
	TMap<UTexture2D*, TArray<FColor>> TextureDataCache;

	/**
	 * Get or cache texture data
	 */
	const TArray<FColor>& GetTextureData(UTexture2D* Texture);

	/**
	 * Clear texture data cache
	 */
	void ClearCache();
};
