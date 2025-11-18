// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeConfig.h"
#include "BiomeBlender.generated.h"

/**
 * Result of biome blending operation
 */
USTRUCT(BlueprintType)
struct FBlendedBiomeResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Biome")
	TMap<EBiomeType, float> BiomeWeights;

	UPROPERTY(BlueprintReadWrite, Category = "Biome")
	FLinearColor BlendedColor;

	UPROPERTY(BlueprintReadWrite, Category = "Biome")
	float BlendedRoughness;

	UPROPERTY(BlueprintReadWrite, Category = "Biome")
	FVector BlendedNormal;

	FBlendedBiomeResult()
		: BlendedColor(FLinearColor::White)
		, BlendedRoughness(0.5f)
		, BlendedNormal(FVector::UpVector)
	{
	}
};

/**
 * Triplanar mapping data
 */
USTRUCT(BlueprintType)
struct FTriplanarWeights
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Triplanar")
	float WeightX;

	UPROPERTY(BlueprintReadWrite, Category = "Triplanar")
	float WeightY;

	UPROPERTY(BlueprintReadWrite, Category = "Triplanar")
	float WeightZ;

	FTriplanarWeights()
		: WeightX(0.0f)
		, WeightY(1.0f)
		, WeightZ(0.0f)
	{
	}
};

/**
 * Biome blending actor
 * Handles weighted blending between multiple biomes with smooth transitions
 */
UCLASS()
class PROCEDURALMAP_API ABiomeBlender : public AActor
{
	GENERATED_BODY()

public:
	ABiomeBlender();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend|Config")
	UBiomeConfigAsset* BiomeConfig;

	/**
	 * Blend multiple biomes using weighted sum
	 * @param BiomeData Array of biomes with their weights
	 * @param Position World position for sampling
	 * @param Normal Surface normal at this position
	 * @return Blended biome result
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Blending")
	FBlendedBiomeResult BlendBiomes(const TArray<FBiomeData>& BiomeData, FVector Position, FVector Normal);

	/**
	 * Perform height-based blending for natural transitions
	 * @param BiomeData Array of biomes with weights
	 * @param Height Current height (0-1 normalized)
	 * @param Slope Terrain slope (0-1)
	 * @return Modified biome weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Blending")
	TArray<FBiomeData> HeightBasedBlending(const TArray<FBiomeData>& BiomeData, float Height, float Slope);

	/**
	 * Calculate transition mask between two biomes
	 * @param BiomeA First biome type
	 * @param BiomeB Second biome type
	 * @param Position World position
	 * @param BlendFactor Base blend factor (0-1)
	 * @return Refined blend mask
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Blending")
	float CalculateTransitionMask(EBiomeType BiomeA, EBiomeType BiomeB, FVector Position, float BlendFactor);

	/**
	 * Calculate triplanar mapping weights from surface normal
	 * @param Normal Surface normal
	 * @param Sharpness Blending sharpness
	 * @return Triplanar weights for X, Y, Z axes
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Triplanar")
	FTriplanarWeights CalculateTriplanarWeights(FVector Normal, float Sharpness);

	/**
	 * Sample texture using triplanar projection
	 * @param Texture Texture to sample
	 * @param Position World position
	 * @param Normal Surface normal
	 * @param Scale Texture scale
	 * @param Sharpness Blend sharpness
	 * @return Sampled color
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Triplanar")
	FLinearColor SampleTriplanar(UTexture2D* Texture, FVector Position, FVector Normal, float Scale, float Sharpness);

	/**
	 * Blend colors using weighted average
	 * @param Colors Array of colors
	 * @param Weights Array of weights
	 * @return Blended color
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Blending")
	FLinearColor BlendColors(const TArray<FLinearColor>& Colors, const TArray<float>& Weights);

	/**
	 * Blend normals using weighted average and normalization
	 * @param Normals Array of normal vectors
	 * @param Weights Array of weights
	 * @return Blended normal vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Blending")
	FVector BlendNormals(const TArray<FVector>& Normals, const TArray<float>& Weights);

	/**
	 * Apply noise to blend weights for more organic transitions
	 * @param Weights Original weights
	 * @param Position World position
	 * @param NoiseStrength Strength of noise effect
	 * @return Modified weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Blending")
	TArray<float> ApplyBlendNoise(const TArray<float>& Weights, FVector Position, float NoiseStrength);

	/**
	 * Calculate slope from normal vector
	 * @param Normal Surface normal
	 * @return Slope value (0 = flat, 1 = vertical)
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Utility")
	float CalculateSlope(FVector Normal) const;

private:
	/**
	 * Smooth interpolation using smoothstep function
	 */
	float SmoothStep(float Edge0, float Edge1, float X) const;

	/**
	 * Power function for weight curves
	 */
	float PowerCurve(float Value, float Power) const;

	/**
	 * Get noise value for blend mask calculation
	 */
	float GetBlendNoise(FVector Position, float Scale) const;

	/**
	 * Sample texture at UV coordinates (simplified for demonstration)
	 */
	FLinearColor SampleTexture(UTexture2D* Texture, FVector2D UV) const;

	/**
	 * Normalize a weight array to sum to 1.0
	 */
	void NormalizeWeights(TArray<float>& Weights) const;
};
