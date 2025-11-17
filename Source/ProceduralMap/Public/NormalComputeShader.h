// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"

/**
 * Normal calculation method enumeration
 */
enum class ENormalCalculationMethod : uint8
{
	Sobel,			// Sobel operator for edge detection
	CentralDifference,	// Simple central difference
	SobelScharr		// Enhanced Sobel-Scharr operator
};

/**
 * Compute shader for GPU-accelerated normal map calculation
 * Calculates surface normals from heightmap data
 */
class PROCEDURALMAP_API FNormalCalculationCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FNormalCalculationCS);
	SHADER_USE_PARAMETER_STRUCT(FNormalCalculationCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// Input heightmap
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, HeightmapInput)

		// Output normal map (RGB = XYZ normal)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, NormalOutput)

		// Parameters
		SHADER_PARAMETER(float, HeightScale)
		SHADER_PARAMETER(float, HorizontalScale)
		SHADER_PARAMETER(int32, CalculationMethod)
		SHADER_PARAMETER(int32, EnableSmoothing)
		SHADER_PARAMETER(int32, SmoothKernelSize)
		SHADER_PARAMETER(FVector2f, TextureSize)
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
 * Compute shader for smoothing normal maps
 * Applies Gaussian blur or box filter to reduce noise
 */
class PROCEDURALMAP_API FNormalSmoothingCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FNormalSmoothingCS);
	SHADER_USE_PARAMETER_STRUCT(FNormalSmoothingCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float4>, NormalInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, NormalOutput)
		SHADER_PARAMETER(int32, KernelSize)
		SHADER_PARAMETER(float, Sigma)
		SHADER_PARAMETER(FVector2f, TextureSize)
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
 * Compute shader for edge detection on terrain
 * Useful for feature extraction and LOD calculations
 */
class PROCEDURALMAP_API FEdgeDetectionCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FEdgeDetectionCS);
	SHADER_USE_PARAMETER_STRUCT(FEdgeDetectionCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, HeightmapInput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, EdgeOutput)
		SHADER_PARAMETER(float, Threshold)
		SHADER_PARAMETER(float, HeightScale)
		SHADER_PARAMETER(FVector2f, TextureSize)
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
 * Namespace for normal computation utilities
 */
namespace NormalComputeShaderUtils
{
	/**
	 * Calculate normals from heightmap
	 * @param GraphBuilder - RDG graph builder
	 * @param NormalOutput - Output normal map texture (RDG UAV)
	 * @param HeightmapInput - Input heightmap texture (RDG SRV)
	 * @param HeightScale - Scale factor for height values
	 * @param HorizontalScale - Scale factor for horizontal distances
	 * @param Method - Calculation method to use
	 * @param bEnableSmoothing - Whether to apply smoothing
	 * @param SmoothKernelSize - Size of smoothing kernel (3, 5, or 7)
	 */
	PROCEDURALMAP_API void CalculateNormals(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef NormalOutput,
		FRDGTextureSRVRef HeightmapInput,
		float HeightScale = 1.0f,
		float HorizontalScale = 1.0f,
		ENormalCalculationMethod Method = ENormalCalculationMethod::Sobel,
		bool bEnableSmoothing = false,
		int32 SmoothKernelSize = 3
	);

	/**
	 * Smooth an existing normal map
	 * @param GraphBuilder - RDG graph builder
	 * @param NormalOutput - Output smoothed normal map
	 * @param NormalInput - Input normal map to smooth
	 * @param KernelSize - Size of smoothing kernel
	 * @param Sigma - Gaussian sigma for smoothing
	 */
	PROCEDURALMAP_API void SmoothNormals(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef NormalOutput,
		FRDGTextureSRVRef NormalInput,
		int32 KernelSize = 3,
		float Sigma = 1.0f
	);

	/**
	 * Detect edges in heightmap
	 * @param GraphBuilder - RDG graph builder
	 * @param EdgeOutput - Output edge detection map
	 * @param HeightmapInput - Input heightmap
	 * @param Threshold - Edge detection threshold
	 * @param HeightScale - Scale factor for height values
	 */
	PROCEDURALMAP_API void DetectEdges(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef EdgeOutput,
		FRDGTextureSRVRef HeightmapInput,
		float Threshold = 0.1f,
		float HeightScale = 1.0f
	);

	/**
	 * Calculate normals with automatic smoothing
	 * Combines normal calculation and smoothing in one call
	 */
	PROCEDURALMAP_API void CalculateSmoothedNormals(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef NormalOutput,
		FRDGTextureSRVRef HeightmapInput,
		float HeightScale = 1.0f,
		float HorizontalScale = 1.0f,
		int32 SmoothingPasses = 1
	);
}
