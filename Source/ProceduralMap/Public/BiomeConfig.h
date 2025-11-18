// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BiomeConfig.generated.h"

/**
 * Enum for different biome types
 */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
	Desert		UMETA(DisplayName = "Desert"),
	Forest		UMETA(DisplayName = "Forest"),
	Snow		UMETA(DisplayName = "Snow"),
	Plains		UMETA(DisplayName = "Plains"),
	Mountains	UMETA(DisplayName = "Mountains"),
	Tundra		UMETA(DisplayName = "Tundra"),
	Swamp		UMETA(DisplayName = "Swamp"),
	Beach		UMETA(DisplayName = "Beach"),
	COUNT		UMETA(Hidden)
};

/**
 * Struct containing biome climate data
 */
USTRUCT(BlueprintType)
struct FBiomeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	EBiomeType BiomeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	float Temperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	float Humidity;

	FBiomeData()
		: BiomeType(EBiomeType::Plains)
		, Weight(0.0f)
		, Temperature(0.5f)
		, Humidity(0.5f)
	{
	}

	FBiomeData(EBiomeType InType, float InWeight, float InTemp, float InHumidity)
		: BiomeType(InType)
		, Weight(InWeight)
		, Temperature(InTemp)
		, Humidity(InHumidity)
	{
	}
};

/**
 * Foliage settings for each biome
 */
USTRUCT(BlueprintType)
struct FFoliageSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	float Density;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	float MinScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	float MaxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	TArray<UStaticMesh*> FoliageMeshes;

	FFoliageSettings()
		: Density(1.0f)
		, MinScale(0.8f)
		, MaxScale(1.2f)
	{
	}
};

/**
 * Complete settings for a biome
 */
USTRUCT(BlueprintType)
struct FBiomeSettings
{
	GENERATED_BODY()

	// Textures
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* BaseColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* NormalMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* RoughnessMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* DisplacementMap;

	// Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UMaterialInterface* BiomeMaterial;

	// Foliage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	FFoliageSettings FoliageSettings;

	// Generation Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MinTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MaxTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MinHumidity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MaxHumidity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MinElevation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MaxElevation;

	// Triplanar mapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	float TriplanarScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	float TriplanarSharpness;

	FBiomeSettings()
		: BaseColor(nullptr)
		, NormalMap(nullptr)
		, RoughnessMap(nullptr)
		, DisplacementMap(nullptr)
		, BiomeMaterial(nullptr)
		, MinTemperature(0.0f)
		, MaxTemperature(1.0f)
		, MinHumidity(0.0f)
		, MaxHumidity(1.0f)
		, MinElevation(0.0f)
		, MaxElevation(1.0f)
		, TriplanarScale(1.0f)
		, TriplanarSharpness(4.0f)
	{
	}
};

/**
 * Settings for biome transitions
 */
USTRUCT(BlueprintType)
struct FBiomeTransitionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float BlendDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float BlendSharpness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bUseHeightBasedBlending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float HeightBlendInfluence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float NoiseScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float NoiseStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	int32 NoiseOctaves;

	FBiomeTransitionSettings()
		: BlendDistance(100.0f)
		, BlendSharpness(2.0f)
		, bUseHeightBasedBlending(true)
		, HeightBlendInfluence(0.5f)
		, NoiseScale(0.01f)
		, NoiseStrength(0.2f)
		, NoiseOctaves(3)
	{
	}
};

/**
 * Data asset for biome configuration
 */
UCLASS(BlueprintType)
class PROCEDURALMAP_API UBiomeConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	TMap<EBiomeType, FBiomeSettings> BiomeSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
	FBiomeTransitionSettings TransitionSettings;

	UBiomeConfigAsset()
	{
		// Initialize default biome settings
		InitializeDefaultBiomes();
	}

	void InitializeDefaultBiomes()
	{
		// Desert
		FBiomeSettings Desert;
		Desert.MinTemperature = 0.7f;
		Desert.MaxTemperature = 1.0f;
		Desert.MinHumidity = 0.0f;
		Desert.MaxHumidity = 0.2f;
		BiomeSettings.Add(EBiomeType::Desert, Desert);

		// Forest
		FBiomeSettings Forest;
		Forest.MinTemperature = 0.4f;
		Forest.MaxTemperature = 0.7f;
		Forest.MinHumidity = 0.5f;
		Forest.MaxHumidity = 1.0f;
		BiomeSettings.Add(EBiomeType::Forest, Forest);

		// Snow
		FBiomeSettings Snow;
		Snow.MinTemperature = 0.0f;
		Snow.MaxTemperature = 0.2f;
		Snow.MinHumidity = 0.0f;
		Snow.MaxHumidity = 1.0f;
		BiomeSettings.Add(EBiomeType::Snow, Snow);

		// Plains
		FBiomeSettings Plains;
		Plains.MinTemperature = 0.4f;
		Plains.MaxTemperature = 0.7f;
		Plains.MinHumidity = 0.2f;
		Plains.MaxHumidity = 0.5f;
		BiomeSettings.Add(EBiomeType::Plains, Plains);

		// Mountains
		FBiomeSettings Mountains;
		Mountains.MinTemperature = 0.0f;
		Mountains.MaxTemperature = 0.5f;
		Mountains.MinElevation = 0.7f;
		Mountains.MaxElevation = 1.0f;
		BiomeSettings.Add(EBiomeType::Mountains, Mountains);

		// Tundra
		FBiomeSettings Tundra;
		Tundra.MinTemperature = 0.1f;
		Tundra.MaxTemperature = 0.3f;
		Tundra.MinHumidity = 0.2f;
		Tundra.MaxHumidity = 0.6f;
		BiomeSettings.Add(EBiomeType::Tundra, Tundra);

		// Swamp
		FBiomeSettings Swamp;
		Swamp.MinTemperature = 0.5f;
		Swamp.MaxTemperature = 0.8f;
		Swamp.MinHumidity = 0.8f;
		Swamp.MaxHumidity = 1.0f;
		BiomeSettings.Add(EBiomeType::Swamp, Swamp);

		// Beach
		FBiomeSettings Beach;
		Beach.MinTemperature = 0.5f;
		Beach.MaxTemperature = 0.9f;
		Beach.MinElevation = 0.0f;
		Beach.MaxElevation = 0.15f;
		BiomeSettings.Add(EBiomeType::Beach, Beach);
	}
};
