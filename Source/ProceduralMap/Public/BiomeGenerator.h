// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeConfig.h"
#include "BiomeGenerator.generated.h"

/**
 * Noise parameters for multi-octave noise generation
 */
USTRUCT(BlueprintType)
struct FNoiseParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Frequency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 Octaves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Lacunarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Persistence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 Seed;

	FNoiseParameters()
		: Frequency(0.01f)
		, Octaves(4)
		, Lacunarity(2.0f)
		, Persistence(0.5f)
		, Seed(1337)
	{
	}
};

/**
 * Main biome generator actor
 * Handles climate calculation and biome determination using Whittaker diagram
 */
UCLASS()
class PROCEDURALMAP_API ABiomeGenerator : public AActor
{
	GENERATED_BODY()

public:
	ABiomeGenerator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
	UBiomeConfigAsset* BiomeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
	FNoiseParameters TemperatureNoise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
	FNoiseParameters HumidityNoise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
	float TemperatureScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
	float HumidityScale;

	// World parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
	float WorldSizeX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
	float WorldSizeY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
	float MaxElevation;

	// Climate calculation methods

	/**
	 * Calculate temperature based on latitude and elevation
	 * @param Position World position
	 * @param Elevation Height at this position (0-1 normalized)
	 * @return Temperature value (0-1, where 0 is cold, 1 is hot)
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Climate")
	float CalculateTemperature(FVector Position, float Elevation);

	/**
	 * Calculate humidity based on distance to water and noise
	 * @param Position World position
	 * @param DistanceToWater Distance to nearest water body
	 * @return Humidity value (0-1, where 0 is dry, 1 is wet)
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Climate")
	float CalculateHumidity(FVector Position, float DistanceToWater);

	/**
	 * Determine biome type from temperature and humidity using Whittaker diagram
	 * @param Temperature Temperature value (0-1)
	 * @param Humidity Humidity value (0-1)
	 * @param Elevation Elevation value (0-1)
	 * @return Biome type
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Generation")
	EBiomeType GetBiomeFromClimate(float Temperature, float Humidity, float Elevation);

	/**
	 * Calculate weighted biome blending for smooth transitions
	 * @param Position World position
	 * @param Elevation Height at this position
	 * @param DistanceToWater Distance to nearest water
	 * @return Array of biome data with weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Generation")
	TArray<FBiomeData> CalculateBiomeWeights(FVector Position, float Elevation, float DistanceToWater);

	/**
	 * Multi-octave noise generation
	 * @param X X coordinate
	 * @param Y Y coordinate
	 * @param Params Noise parameters
	 * @return Noise value (-1 to 1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Noise")
	float GenerateNoise(float X, float Y, const FNoiseParameters& Params);

	/**
	 * Get biome settings for a specific biome type
	 * @param BiomeType Type of biome
	 * @return Biome settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome|Config")
	FBiomeSettings GetBiomeSettings(EBiomeType BiomeType);

private:
	/**
	 * Calculate latitude factor (0-1) from world position
	 */
	float GetLatitudeFactor(FVector Position) const;

	/**
	 * Calculate base noise value using FastNoiseLite
	 */
	float GetNoiseValue(float X, float Y, int32 Seed) const;

	/**
	 * Smooth interpolation for climate values
	 */
	float SmoothStep(float Edge0, float Edge1, float X) const;

	/**
	 * Calculate biome match score based on climate parameters
	 */
	float CalculateBiomeMatchScore(float Temperature, float Humidity, float Elevation, const FBiomeSettings& Settings) const;

	/**
	 * Normalize weights array so they sum to 1
	 */
	void NormalizeWeights(TArray<FBiomeData>& BiomeData) const;
};
