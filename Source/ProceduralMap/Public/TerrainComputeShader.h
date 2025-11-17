// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"

/**
 * Enumeration for terrain generation pass types
 */
enum class ETerrainGenerationPass : uint8
{
	Continental,	// Large-scale continental shapes
	Mountains,		// Mountain ranges and hills
	Details			// Fine details and noise
};

/**
 * Parameters for terrain generation
 */
struct FTerrainNoiseParameters
{
	float Frequency;
	int32 Octaves;
	float Lacunarity;
	float Gain;
	float Amplitude;

	FTerrainNoiseParameters()
		: Frequency(1.0f)
		, Octaves(6)
		, Lacunarity(2.0f)
		, Gain(0.5f)
		, Amplitude(1.0f)
	{}
};

/**
 * Compute shader for GPU-accelerated terrain generation
 * Uses Render Dependency Graph (RDG) for modern rendering pipeline
 */
class PROCEDURALMAP_API FTerrainGenerationCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTerrainGenerationCS);
	SHADER_USE_PARAMETER_STRUCT(FTerrainGenerationCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// Output heightmap texture
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutputHeightmap)

		// Noise parameters
		SHADER_PARAMETER(float, Frequency)
		SHADER_PARAMETER(int32, Octaves)
		SHADER_PARAMETER(float, Lacunarity)
		SHADER_PARAMETER(float, Gain)
		SHADER_PARAMETER(float, Amplitude)

		// Terrain parameters
		SHADER_PARAMETER(uint32, Seed)
		SHADER_PARAMETER(FVector2f, TerrainSize)
		SHADER_PARAMETER(FVector2f, Offset)
		SHADER_PARAMETER(int32, PassType)

		// Biome weights for blending
		SHADER_PARAMETER(FVector4f, BiomeWeights)

		// Advanced parameters
		SHADER_PARAMETER(float, Persistence)
		SHADER_PARAMETER(float, Scale)
		SHADER_PARAMETER(int32, EnableRidgedNoise)
		SHADER_PARAMETER(float, RidgedSharpness)
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
 * Namespace for terrain compute shader utilities
 */
namespace TerrainComputeShaderUtils
{
	/**
	 * Dispatch terrain generation compute shader
	 * @param GraphBuilder - RDG graph builder
	 * @param OutputTexture - Output heightmap texture (RDG)
	 * @param NoiseParams - Noise generation parameters
	 * @param Seed - Random seed for generation
	 * @param TerrainSize - Size of terrain in world units
	 * @param Offset - Offset for tiling/chunking
	 * @param PassType - Type of generation pass
	 * @param BiomeWeights - Weights for biome blending
	 */
	PROCEDURALMAP_API void DispatchTerrainGeneration(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef OutputTexture,
		const FTerrainNoiseParameters& NoiseParams,
		uint32 Seed,
		FVector2D TerrainSize,
		FVector2D Offset,
		ETerrainGenerationPass PassType,
		FVector4 BiomeWeights = FVector4(1, 0, 0, 0)
	);

	/**
	 * Multi-pass terrain generation
	 * Generates continental shapes, mountains, and details in separate passes
	 */
	PROCEDURALMAP_API void DispatchMultiPassTerrainGeneration(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef OutputTexture,
		uint32 Seed,
		FVector2D TerrainSize,
		FVector2D Offset
	);
}
