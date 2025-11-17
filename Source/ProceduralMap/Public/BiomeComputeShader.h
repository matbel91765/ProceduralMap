// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"

/**
 * Biome types enumeration
 */
enum class EBiomeType : uint8
{
	Ocean = 0,
	Beach = 1,
	Desert = 2,
	Savanna = 3,
	TropicalRainforest = 4,
	Grassland = 5,
	TemperateForest = 6,
	Taiga = 7,
	Tundra = 8,
	Mountain = 9,
	Snow = 10,
	MAX
};

/**
 * Whittaker diagram parameters
 * Maps temperature and humidity to biome types
 */
struct FWhittakerParameters
{
	float TemperatureMin;
	float TemperatureMax;
	float HumidityMin;
	float HumidityMax;

	FWhittakerParameters()
		: TemperatureMin(-20.0f)
		, TemperatureMax(40.0f)
		, HumidityMin(0.0f)
		, HumidityMax(1.0f)
	{}
};

/**
 * Compute shader for GPU-accelerated biome calculation
 * Uses temperature and humidity maps to determine biome distribution
 */
class PROCEDURALMAP_API FBiomeCalculationCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FBiomeCalculationCS);
	SHADER_USE_PARAMETER_STRUCT(FBiomeCalculationCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// Input textures
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, HeightmapInput)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, TemperatureInput)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, HumidityInput)

		// Output textures
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<uint>, BiomeIDOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, BiomeWeightsOutput)

		// Parameters
		SHADER_PARAMETER(uint32, Seed)
		SHADER_PARAMETER(FVector2f, TerrainSize)
		SHADER_PARAMETER(float, SeaLevel)
		SHADER_PARAMETER(float, SnowLevel)
		SHADER_PARAMETER(float, MountainLevel)

		// Whittaker parameters
		SHADER_PARAMETER(float, TemperatureMin)
		SHADER_PARAMETER(float, TemperatureMax)
		SHADER_PARAMETER(float, HumidityMin)
		SHADER_PARAMETER(float, HumidityMax)

		// Blend parameters
		SHADER_PARAMETER(float, BlendDistance)
		SHADER_PARAMETER(int32, EnableSmoothing)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_X"), 8);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Y"), 8);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Z"), 1);
	}
};

/**
 * Compute shader for generating temperature map
 */
class PROCEDURALMAP_API FTemperatureMapCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTemperatureMapCS);
	SHADER_USE_PARAMETER_STRUCT(FTemperatureMapCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, TemperatureOutput)
		SHADER_PARAMETER(uint32, Seed)
		SHADER_PARAMETER(FVector2f, TerrainSize)
		SHADER_PARAMETER(float, LatitudeEffect)
		SHADER_PARAMETER(float, AltitudeEffect)
		SHADER_PARAMETER(float, NoiseScale)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, HeightmapInput)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_X"), 8);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Y"), 8);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Z"), 1);
	}
};

/**
 * Compute shader for generating humidity map
 */
class PROCEDURALMAP_API FHumidityMapCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FHumidityMapCS);
	SHADER_USE_PARAMETER_STRUCT(FHumidityMapCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, HumidityOutput)
		SHADER_PARAMETER(uint32, Seed)
		SHADER_PARAMETER(FVector2f, TerrainSize)
		SHADER_PARAMETER(float, OceanProximityEffect)
		SHADER_PARAMETER(float, NoiseScale)
		SHADER_PARAMETER(float, SeaLevel)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, HeightmapInput)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_X"), 8);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Y"), 8);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Z"), 1);
	}
};

/**
 * Namespace for biome compute shader utilities
 */
namespace BiomeComputeShaderUtils
{
	/**
	 * Generate temperature map based on latitude and altitude
	 */
	PROCEDURALMAP_API void GenerateTemperatureMap(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef TemperatureOutput,
		FRDGTextureSRVRef HeightmapInput,
		uint32 Seed,
		FVector2D TerrainSize,
		float LatitudeEffect = 1.0f,
		float AltitudeEffect = 0.006f,
		float NoiseScale = 0.5f
	);

	/**
	 * Generate humidity map based on ocean proximity and noise
	 */
	PROCEDURALMAP_API void GenerateHumidityMap(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef HumidityOutput,
		FRDGTextureSRVRef HeightmapInput,
		uint32 Seed,
		FVector2D TerrainSize,
		float SeaLevel = 0.0f,
		float OceanProximityEffect = 1.0f,
		float NoiseScale = 0.5f
	);

	/**
	 * Calculate biome distribution using Whittaker diagram
	 */
	PROCEDURALMAP_API void CalculateBiomes(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef BiomeIDOutput,
		FRDGTextureUAVRef BiomeWeightsOutput,
		FRDGTextureSRVRef HeightmapInput,
		FRDGTextureSRVRef TemperatureInput,
		FRDGTextureSRVRef HumidityInput,
		uint32 Seed,
		FVector2D TerrainSize,
		const FWhittakerParameters& WhittakerParams,
		float SeaLevel = 0.0f,
		float SnowLevel = 1000.0f,
		float MountainLevel = 800.0f
	);

	/**
	 * Complete biome generation pipeline
	 * Generates temperature, humidity, and biome maps in one call
	 */
	PROCEDURALMAP_API void GenerateCompleteBiomeData(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef BiomeIDOutput,
		FRDGTextureUAVRef BiomeWeightsOutput,
		FRDGTextureSRVRef HeightmapInput,
		uint32 Seed,
		FVector2D TerrainSize
	);
}
