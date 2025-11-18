// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainGPUManager.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TextureResource.h"
#include "RenderTargetPool.h"
#include "RendererInterface.h"
#include "GlobalShader.h"
#include "Misc/ScopedSlowTask.h"

//=============================================================================
// UTerrainGPUManager Implementation
//=============================================================================

UTerrainGPUManager::UTerrainGPUManager()
	: LastGenerationTimeMs(0.0f)
	, bIsGenerating(false)
{
}

FTerrainGenerationResult UTerrainGPUManager::GenerateTerrainComplete(const FTerrainGenerationConfig& Config)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_TerrainGPUManager_GenerateComplete);

	FTerrainGenerationResult Result;
	Result.bSuccess = false;

	if (bIsGenerating)
	{
		UE_LOG(LogTemp, Warning, TEXT("Terrain generation already in progress"));
		return Result;
	}

	bIsGenerating = true;
	const double StartTime = FPlatformTime::Seconds();

	// Execute on render thread
	ENQUEUE_RENDER_COMMAND(GenerateTerrainCommand)(
		[this, Config, &Result](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			FRDGTextureRef HeightmapTexture;
			FRDGTextureRef BiomeIDTexture;
			FRDGTextureRef BiomeWeightsTexture;
			FRDGTextureRef NormalMapTexture;

			// Execute complete pipeline
			FTerrainGPUUtils::ExecuteTerrainPipeline(
				GraphBuilder,
				Config.HeightmapResolution,
				Config,
				HeightmapTexture,
				BiomeIDTexture,
				BiomeWeightsTexture,
				NormalMapTexture
			);

			// Readback textures if not in GPU-only mode
			if (!Config.bGPUOnlyMode)
			{
				ReadbackTextures(
					GraphBuilder,
					HeightmapTexture,
					BiomeIDTexture,
					BiomeWeightsTexture,
					NormalMapTexture,
					Result
				);
			}

			GraphBuilder.Execute();
		}
	);

	// Wait for render thread
	FlushRenderingCommands();

	const double EndTime = FPlatformTime::Seconds();
	LastGenerationTimeMs = (EndTime - StartTime) * 1000.0f;
	Result.GenerationTimeMs = LastGenerationTimeMs;
	Result.bSuccess = true;

	bIsGenerating = false;

	UE_LOG(LogTemp, Log, TEXT("Terrain generation completed in %.2f ms"), LastGenerationTimeMs);

	return Result;
}

UTexture2D* UTerrainGPUManager::GenerateHeightmap(const FTerrainGenerationConfig& Config)
{
	UTexture2D* HeightmapTexture = nullptr;

	ENQUEUE_RENDER_COMMAND(GenerateHeightmapCommand)(
		[this, Config, &HeightmapTexture](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			// Create heightmap texture
			FRDGTextureDesc HeightmapDesc = FRDGTextureDesc::Create2D(
				FIntPoint(Config.HeightmapResolution, Config.HeightmapResolution),
				PF_R32_FLOAT,
				FClearValueBinding::Black,
				TexCreate_ShaderResource | TexCreate_UAV
			);

			FRDGTextureRef Heightmap = GraphBuilder.CreateTexture(HeightmapDesc, TEXT("GeneratedHeightmap"));

			// Generate heightmap
			FTerrainGPUUtils::GenerateHeightmapGPU(GraphBuilder, Heightmap, Config);

			// Convert to UTexture2D
			HeightmapTexture = ConvertRDGTextureToTexture2D(GraphBuilder, Heightmap, TEXT("TerrainHeightmap"));

			GraphBuilder.Execute();
		}
	);

	FlushRenderingCommands();

	return HeightmapTexture;
}

void UTerrainGPUManager::GenerateBiomes(
	UTexture2D* HeightmapTexture,
	int32 Seed,
	UTexture2D*& OutBiomeID,
	UTexture2D*& OutBiomeWeights)
{
	if (!HeightmapTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid heightmap texture for biome generation"));
		return;
	}

	ENQUEUE_RENDER_COMMAND(GenerateBiomesCommand)(
		[this, HeightmapTexture, Seed, &OutBiomeID, &OutBiomeWeights](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			// Import heightmap texture
			FRDGTextureRef ImportedHeightmap = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(HeightmapTexture->GetResource()->TextureRHI, TEXT("ImportedHeightmap"))
			);

			FRDGTextureSRVRef HeightmapSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(ImportedHeightmap));

			// Create biome textures
			int32 Resolution = HeightmapTexture->GetSizeX();

			FRDGTextureDesc BiomeIDDesc = FRDGTextureDesc::Create2D(
				FIntPoint(Resolution, Resolution),
				PF_R32_UINT,
				FClearValueBinding::Black,
				TexCreate_ShaderResource | TexCreate_UAV
			);

			FRDGTextureDesc BiomeWeightsDesc = FRDGTextureDesc::Create2D(
				FIntPoint(Resolution, Resolution),
				PF_A32B32G32R32F,
				FClearValueBinding::Black,
				TexCreate_ShaderResource | TexCreate_UAV
			);

			FRDGTextureRef BiomeIDTex = GraphBuilder.CreateTexture(BiomeIDDesc, TEXT("BiomeID"));
			FRDGTextureRef BiomeWeightsTex = GraphBuilder.CreateTexture(BiomeWeightsDesc, TEXT("BiomeWeights"));

			// Generate biomes
			FVector2D TerrainSize(10000, 10000); // Default terrain size
			FTerrainGPUUtils::GenerateBiomesGPU(
				GraphBuilder,
				HeightmapSRV,
				BiomeIDTex,
				BiomeWeightsTex,
				Seed,
				TerrainSize
			);

			// Convert to textures
			OutBiomeID = ConvertRDGTextureToTexture2D(GraphBuilder, BiomeIDTex, TEXT("BiomeID"));
			OutBiomeWeights = ConvertRDGTextureToTexture2D(GraphBuilder, BiomeWeightsTex, TEXT("BiomeWeights"));

			GraphBuilder.Execute();
		}
	);

	FlushRenderingCommands();
}

UTexture2D* UTerrainGPUManager::GenerateNormalMap(
	UTexture2D* HeightmapTexture,
	float HeightScale,
	ENormalCalculationMethod Method)
{
	if (!HeightmapTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid heightmap texture for normal map generation"));
		return nullptr;
	}

	UTexture2D* NormalMapTexture = nullptr;

	ENQUEUE_RENDER_COMMAND(GenerateNormalMapCommand)(
		[this, HeightmapTexture, HeightScale, Method, &NormalMapTexture](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			// Import heightmap
			FRDGTextureRef ImportedHeightmap = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(HeightmapTexture->GetResource()->TextureRHI, TEXT("ImportedHeightmap"))
			);

			FRDGTextureSRVRef HeightmapSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(ImportedHeightmap));

			// Create normal map texture
			int32 Resolution = HeightmapTexture->GetSizeX();

			FRDGTextureDesc NormalDesc = FRDGTextureDesc::Create2D(
				FIntPoint(Resolution, Resolution),
				PF_A32B32G32R32F,
				FClearValueBinding::Black,
				TexCreate_ShaderResource | TexCreate_UAV
			);

			FRDGTextureRef NormalTex = GraphBuilder.CreateTexture(NormalDesc, TEXT("NormalMap"));

			// Generate normal map
			FTerrainGPUUtils::GenerateNormalMapGPU(
				GraphBuilder,
				HeightmapSRV,
				NormalTex,
				HeightScale,
				Method
			);

			// Convert to texture
			NormalMapTexture = ConvertRDGTextureToTexture2D(GraphBuilder, NormalTex, TEXT("TerrainNormalMap"));

			GraphBuilder.Execute();
		}
	);

	FlushRenderingCommands();

	return NormalMapTexture;
}

void UTerrainGPUManager::GenerateTerrainAsync(
	const FTerrainGenerationConfig& Config,
	const FOnTerrainGenerationComplete& OnComplete)
{
	// For now, just call synchronous version
	// TODO: Implement proper async with callbacks
	FTerrainGenerationResult Result = GenerateTerrainComplete(Config);
	OnComplete.ExecuteIfBound(Result);
}

void UTerrainGPUManager::ReadbackTextures(
	FRDGBuilder& GraphBuilder,
	FRDGTextureRef HeightmapTexture,
	FRDGTextureRef BiomeIDTexture,
	FRDGTextureRef BiomeWeightsTexture,
	FRDGTextureRef NormalMapTexture,
	FTerrainGenerationResult& OutResult)
{
	// Convert RDG textures to UTexture2D
	if (HeightmapTexture)
	{
		OutResult.HeightmapTexture = ConvertRDGTextureToTexture2D(GraphBuilder, HeightmapTexture, TEXT("Heightmap"));
	}

	if (BiomeIDTexture)
	{
		OutResult.BiomeIDTexture = ConvertRDGTextureToTexture2D(GraphBuilder, BiomeIDTexture, TEXT("BiomeID"));
	}

	if (BiomeWeightsTexture)
	{
		OutResult.BiomeWeightsTexture = ConvertRDGTextureToTexture2D(GraphBuilder, BiomeWeightsTexture, TEXT("BiomeWeights"));
	}

	if (NormalMapTexture)
	{
		OutResult.NormalMapTexture = ConvertRDGTextureToTexture2D(GraphBuilder, NormalMapTexture, TEXT("NormalMap"));
	}
}

UTexture2D* UTerrainGPUManager::ConvertRDGTextureToTexture2D(
	FRDGBuilder& GraphBuilder,
	FRDGTextureRef RDGTexture,
	const FString& TextureName)
{
	// This is a simplified version - in production, you'd want proper texture creation
	// For now, return nullptr and log a warning
	UE_LOG(LogTemp, Warning, TEXT("ConvertRDGTextureToTexture2D not fully implemented - requires texture readback"));
	return nullptr;
}

//=============================================================================
// FTerrainGPUUtils Implementation
//=============================================================================

void FTerrainGPUUtils::ExecuteTerrainPipeline(
	FRDGBuilder& GraphBuilder,
	int32 Resolution,
	const FTerrainGenerationConfig& Config,
	FRDGTextureRef& OutHeightmap,
	FRDGTextureRef& OutBiomeID,
	FRDGTextureRef& OutBiomeWeights,
	FRDGTextureRef& OutNormalMap)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_TerrainGPU_ExecutePipeline);

	// Create heightmap texture
	FRDGTextureDesc HeightmapDesc = FRDGTextureDesc::Create2D(
		FIntPoint(Resolution, Resolution),
		PF_R32_FLOAT,
		FClearValueBinding::Black,
		TexCreate_ShaderResource | TexCreate_UAV
	);

	OutHeightmap = GraphBuilder.CreateTexture(HeightmapDesc, TEXT("TerrainHeightmap"));

	// Generate heightmap
	GenerateHeightmapGPU(GraphBuilder, OutHeightmap, Config);

	// Create SRV for heightmap
	FRDGTextureSRVRef HeightmapSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(OutHeightmap));

	// Generate biomes if requested
	if (Config.bGenerateBiomes)
	{
		FRDGTextureDesc BiomeIDDesc = FRDGTextureDesc::Create2D(
			FIntPoint(Resolution, Resolution),
			PF_R32_UINT,
			FClearValueBinding::Black,
			TexCreate_ShaderResource | TexCreate_UAV
		);

		FRDGTextureDesc BiomeWeightsDesc = FRDGTextureDesc::Create2D(
			FIntPoint(Resolution, Resolution),
			PF_A32B32G32R32F,
			FClearValueBinding::Black,
			TexCreate_ShaderResource | TexCreate_UAV
		);

		OutBiomeID = GraphBuilder.CreateTexture(BiomeIDDesc, TEXT("BiomeID"));
		OutBiomeWeights = GraphBuilder.CreateTexture(BiomeWeightsDesc, TEXT("BiomeWeights"));

		GenerateBiomesGPU(
			GraphBuilder,
			HeightmapSRV,
			OutBiomeID,
			OutBiomeWeights,
			Config.Seed,
			Config.TerrainSize
		);
	}

	// Generate normal map if requested
	if (Config.bGenerateNormals)
	{
		FRDGTextureDesc NormalDesc = FRDGTextureDesc::Create2D(
			FIntPoint(Resolution, Resolution),
			PF_A32B32G32R32F,
			FClearValueBinding::Black,
			TexCreate_ShaderResource | TexCreate_UAV
		);

		OutNormalMap = GraphBuilder.CreateTexture(NormalDesc, TEXT("NormalMap"));

		GenerateNormalMapGPU(
			GraphBuilder,
			HeightmapSRV,
			OutNormalMap,
			Config.NormalHeightScale,
			Config.NormalMethod
		);
	}
}

void FTerrainGPUUtils::GenerateHeightmapGPU(
	FRDGBuilder& GraphBuilder,
	FRDGTextureRef HeightmapTexture,
	const FTerrainGenerationConfig& Config)
{
	FRDGTextureUAVRef HeightmapUAV = GraphBuilder.CreateUAV(HeightmapTexture);

	if (Config.bUseMultiPass)
	{
		// Multi-pass generation (continental, mountains, details)
		TerrainComputeShaderUtils::DispatchMultiPassTerrainGeneration(
			GraphBuilder,
			HeightmapUAV,
			Config.Seed,
			Config.TerrainSize,
			FVector2D(0, 0) // Offset
		);
	}
	else
	{
		// Single-pass generation
		TerrainComputeShaderUtils::DispatchTerrainGeneration(
			GraphBuilder,
			HeightmapUAV,
			Config.NoiseParameters,
			Config.Seed,
			Config.TerrainSize,
			FVector2D(0, 0),
			ETerrainGenerationPass::Continental,
			FVector4(1, 0, 0, 0)
		);
	}
}

void FTerrainGPUUtils::GenerateBiomesGPU(
	FRDGBuilder& GraphBuilder,
	FRDGTextureSRVRef HeightmapSRV,
	FRDGTextureRef BiomeIDTexture,
	FRDGTextureRef BiomeWeightsTexture,
	uint32 Seed,
	FVector2D TerrainSize)
{
	FRDGTextureUAVRef BiomeIDUAV = GraphBuilder.CreateUAV(BiomeIDTexture);
	FRDGTextureUAVRef BiomeWeightsUAV = GraphBuilder.CreateUAV(BiomeWeightsTexture);

	// Generate complete biome data (temperature, humidity, biomes)
	BiomeComputeShaderUtils::GenerateCompleteBiomeData(
		GraphBuilder,
		BiomeIDUAV,
		BiomeWeightsUAV,
		HeightmapSRV,
		Seed,
		TerrainSize
	);
}

void FTerrainGPUUtils::GenerateNormalMapGPU(
	FRDGBuilder& GraphBuilder,
	FRDGTextureSRVRef HeightmapSRV,
	FRDGTextureRef NormalMapTexture,
	float HeightScale,
	ENormalCalculationMethod Method)
{
	FRDGTextureUAVRef NormalMapUAV = GraphBuilder.CreateUAV(NormalMapTexture);

	// Calculate normals with smoothing
	NormalComputeShaderUtils::CalculateSmoothedNormals(
		GraphBuilder,
		NormalMapUAV,
		HeightmapSRV,
		HeightScale,
		1.0f, // HorizontalScale
		1     // SmoothingPasses
	);
}
