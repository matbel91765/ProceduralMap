// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainMaterial.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Engine/Texture2D.h"

UTerrainMaterialGenerator::UTerrainMaterialGenerator()
	: MaxTextureLayers(8)
	, DefaultBlendSharpness(8.0f)
	, DefaultTilingScale(1.0f)
{
}

UMaterialInstanceDynamic* UTerrainMaterialGenerator::GenerateMaterialInstance(
	UMaterial* BaseMaterial,
	const FBiomeMaterialConfig& BiomeConfig)
{
	if (!BaseMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("UTerrainMaterialGenerator::GenerateMaterialInstance - BaseMaterial is null"));
		return nullptr;
	}

	// Create dynamic material instance
	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!MaterialInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UTerrainMaterialGenerator::GenerateMaterialInstance - Failed to create MaterialInstanceDynamic"));
		return nullptr;
	}

	// Apply biome configuration
	ApplyBiomeConfiguration(MaterialInstance, BiomeConfig);

	return MaterialInstance;
}

void UTerrainMaterialGenerator::UpdateTextureLayer(
	UMaterialInstanceDynamic* MaterialInstance,
	int32 LayerIndex,
	const FTerrainTextureLayer& Layer)
{
	if (!MaterialInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UTerrainMaterialGenerator::UpdateTextureLayer - MaterialInstance is null"));
		return;
	}

	if (LayerIndex < 0 || LayerIndex >= MaxTextureLayers)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTerrainMaterialGenerator::UpdateTextureLayer - LayerIndex %d out of range [0, %d)"), LayerIndex, MaxTextureLayers);
		return;
	}

	// Set base color texture
	if (Layer.BaseColorTexture)
	{
		FName ParamName = GetLayerParameterName(TEXT("BaseColor"), LayerIndex);
		SetTextureParameter(MaterialInstance, ParamName, Layer.BaseColorTexture);
	}

	// Set normal map
	if (Layer.NormalMap)
	{
		FName ParamName = GetLayerParameterName(TEXT("Normal"), LayerIndex);
		SetTextureParameter(MaterialInstance, ParamName, Layer.NormalMap);
	}

	// Set roughness map
	if (Layer.RoughnessMap)
	{
		FName ParamName = GetLayerParameterName(TEXT("Roughness"), LayerIndex);
		SetTextureParameter(MaterialInstance, ParamName, Layer.RoughnessMap);
	}

	// Set displacement/height map
	if (Layer.DisplacementMap)
	{
		FName ParamName = GetLayerParameterName(TEXT("Displacement"), LayerIndex);
		SetTextureParameter(MaterialInstance, ParamName, Layer.DisplacementMap);
	}

	if (Layer.HeightMap)
	{
		FName ParamName = GetLayerParameterName(TEXT("Height"), LayerIndex);
		SetTextureParameter(MaterialInstance, ParamName, Layer.HeightMap);
	}

	// Set tiling scale
	{
		FName ParamName = GetLayerParameterName(TEXT("TilingScale"), LayerIndex);
		SetScalarParameter(MaterialInstance, ParamName, Layer.TilingScale);
	}

	// Set triplanar blend sharpness
	{
		FName ParamName = GetLayerParameterName(TEXT("TriplanarBlendSharpness"), LayerIndex);
		SetScalarParameter(MaterialInstance, ParamName, Layer.TriplanarBlendSharpness);
	}

	// Set tint color
	{
		FName ParamName = GetLayerParameterName(TEXT("TintColor"), LayerIndex);
		SetVectorParameter(MaterialInstance, ParamName, Layer.TintColor);
	}
}

void UTerrainMaterialGenerator::ConfigureTriplanarMapping(
	UMaterialInstanceDynamic* MaterialInstance,
	float BlendSharpness,
	bool bWorldAligned)
{
	if (!MaterialInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UTerrainMaterialGenerator::ConfigureTriplanarMapping - MaterialInstance is null"));
		return;
	}

	// Set global triplanar blend sharpness
	SetScalarParameter(MaterialInstance, TEXT("GlobalTriplanarBlendSharpness"), BlendSharpness);

	// Set world-aligned vs object-aligned
	SetScalarParameter(MaterialInstance, TEXT("UseWorldAlignedTexturing"), bWorldAligned ? 1.0f : 0.0f);
}

void UTerrainMaterialGenerator::SetupDistanceFading(
	UMaterialInstanceDynamic* MaterialInstance,
	float StartDistance,
	float EndDistance)
{
	if (!MaterialInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UTerrainMaterialGenerator::SetupDistanceFading - MaterialInstance is null"));
		return;
	}

	// Enable distance fading
	SetScalarParameter(MaterialInstance, TEXT("EnableDistanceFading"), 1.0f);

	// Set fade distances
	SetScalarParameter(MaterialInstance, TEXT("FadeStartDistance"), StartDistance);
	SetScalarParameter(MaterialInstance, TEXT("FadeEndDistance"), EndDistance);
}

void UTerrainMaterialGenerator::ApplyBiomeConfiguration(
	UMaterialInstanceDynamic* MaterialInstance,
	const FBiomeMaterialConfig& BiomeConfig)
{
	if (!MaterialInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UTerrainMaterialGenerator::ApplyBiomeConfiguration - MaterialInstance is null"));
		return;
	}

	// Configure global triplanar mapping
	ConfigureTriplanarMapping(
		MaterialInstance,
		BiomeConfig.GlobalTriplanarBlendSharpness,
		BiomeConfig.bUseWorldAlignedTexturing
	);

	// Set up distance fading if enabled
	if (BiomeConfig.bEnableDistanceFading)
	{
		SetupDistanceFading(
			MaterialInstance,
			BiomeConfig.FadeStartDistance,
			BiomeConfig.FadeEndDistance
		);
	}
	else
	{
		SetScalarParameter(MaterialInstance, TEXT("EnableDistanceFading"), 0.0f);
	}

	// Apply all texture layers
	int32 LayerCount = FMath::Min(BiomeConfig.TextureLayers.Num(), MaxTextureLayers);
	for (int32 i = 0; i < LayerCount; ++i)
	{
		UpdateTextureLayer(MaterialInstance, i, BiomeConfig.TextureLayers[i]);
	}

	// Set the number of active layers
	SetScalarParameter(MaterialInstance, TEXT("NumActiveLayers"), static_cast<float>(LayerCount));

	UE_LOG(LogTemp, Log, TEXT("UTerrainMaterialGenerator::ApplyBiomeConfiguration - Applied biome '%s' with %d layers"),
		*BiomeConfig.BiomeName, LayerCount);
}

void UTerrainMaterialGenerator::SetTextureParameter(
	UMaterialInstanceDynamic* MaterialInstance,
	const FName& ParameterName,
	UTexture2D* Texture)
{
	if (MaterialInstance && Texture)
	{
		MaterialInstance->SetTextureParameterValue(ParameterName, Texture);
	}
}

void UTerrainMaterialGenerator::SetScalarParameter(
	UMaterialInstanceDynamic* MaterialInstance,
	const FName& ParameterName,
	float Value)
{
	if (MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue(ParameterName, Value);
	}
}

void UTerrainMaterialGenerator::SetVectorParameter(
	UMaterialInstanceDynamic* MaterialInstance,
	const FName& ParameterName,
	const FLinearColor& Value)
{
	if (MaterialInstance)
	{
		MaterialInstance->SetVectorParameterValue(ParameterName, Value);
	}
}

FName UTerrainMaterialGenerator::GetLayerParameterName(const FString& BaseParameterName, int32 LayerIndex) const
{
	// Generate parameter name like "BaseColor_Layer0", "Normal_Layer1", etc.
	FString ParameterNameString = FString::Printf(TEXT("%s_Layer%d"), *BaseParameterName, LayerIndex);
	return FName(*ParameterNameString);
}
