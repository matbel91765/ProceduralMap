// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeComputeShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterUtils.h"
#include "GlobalShader.h"
#include "RHIStaticStates.h"
#include "PipelineStateCache.h"

// Implement shader classes
IMPLEMENT_GLOBAL_SHADER(FBiomeCalculationCS, "/Plugin/ProceduralMap/Private/BiomeComputeShader.usf", "CalculateBiomesCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FTemperatureMapCS, "/Plugin/ProceduralMap/Private/BiomeComputeShader.usf", "GenerateTemperatureCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FHumidityMapCS, "/Plugin/ProceduralMap/Private/BiomeComputeShader.usf", "GenerateHumidityCS", SF_Compute);

namespace BiomeComputeShaderUtils
{
	void GenerateTemperatureMap(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef TemperatureOutput,
		FRDGTextureSRVRef HeightmapInput,
		uint32 Seed,
		FVector2D TerrainSize,
		float LatitudeEffect,
		float AltitudeEffect,
		float NoiseScale)
	{
		check(TemperatureOutput);
		check(HeightmapInput);

		// Get texture dimensions
		FRDGTextureDesc OutputDesc = TemperatureOutput->Desc.Texture->Desc;

		// Setup shader parameters
		FTemperatureMapCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FTemperatureMapCS::FParameters>();
		PassParameters->TemperatureOutput = TemperatureOutput;
		PassParameters->HeightmapInput = HeightmapInput;
		PassParameters->Seed = Seed;
		PassParameters->TerrainSize = FVector2f(TerrainSize);
		PassParameters->LatitudeEffect = LatitudeEffect;
		PassParameters->AltitudeEffect = AltitudeEffect;
		PassParameters->NoiseScale = NoiseScale;

		// Get shader
		TShaderMapRef<FTemperatureMapCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// Calculate thread groups
		const uint32 ThreadGroupSizeX = 8;
		const uint32 ThreadGroupSizeY = 8;
		const uint32 ThreadGroupCountX = FMath::DivideAndRoundUp(OutputDesc.Extent.X, ThreadGroupSizeX);
		const uint32 ThreadGroupCountY = FMath::DivideAndRoundUp(OutputDesc.Extent.Y, ThreadGroupSizeY);

		// Add compute pass
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("GenerateTemperatureMap"),
			ComputeShader,
			PassParameters,
			FIntVector(ThreadGroupCountX, ThreadGroupCountY, 1)
		);
	}

	void GenerateHumidityMap(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef HumidityOutput,
		FRDGTextureSRVRef HeightmapInput,
		uint32 Seed,
		FVector2D TerrainSize,
		float SeaLevel,
		float OceanProximityEffect,
		float NoiseScale)
	{
		check(HumidityOutput);
		check(HeightmapInput);

		// Get texture dimensions
		FRDGTextureDesc OutputDesc = HumidityOutput->Desc.Texture->Desc;

		// Setup shader parameters
		FHumidityMapCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FHumidityMapCS::FParameters>();
		PassParameters->HumidityOutput = HumidityOutput;
		PassParameters->HeightmapInput = HeightmapInput;
		PassParameters->Seed = Seed;
		PassParameters->TerrainSize = FVector2f(TerrainSize);
		PassParameters->OceanProximityEffect = OceanProximityEffect;
		PassParameters->NoiseScale = NoiseScale;
		PassParameters->SeaLevel = SeaLevel;

		// Get shader
		TShaderMapRef<FHumidityMapCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// Calculate thread groups
		const uint32 ThreadGroupSizeX = 8;
		const uint32 ThreadGroupSizeY = 8;
		const uint32 ThreadGroupCountX = FMath::DivideAndRoundUp(OutputDesc.Extent.X, ThreadGroupSizeX);
		const uint32 ThreadGroupCountY = FMath::DivideAndRoundUp(OutputDesc.Extent.Y, ThreadGroupSizeY);

		// Add compute pass
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("GenerateHumidityMap"),
			ComputeShader,
			PassParameters,
			FIntVector(ThreadGroupCountX, ThreadGroupCountY, 1)
		);
	}

	void CalculateBiomes(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef BiomeIDOutput,
		FRDGTextureUAVRef BiomeWeightsOutput,
		FRDGTextureSRVRef HeightmapInput,
		FRDGTextureSRVRef TemperatureInput,
		FRDGTextureSRVRef HumidityInput,
		uint32 Seed,
		FVector2D TerrainSize,
		const FWhittakerParameters& WhittakerParams,
		float SeaLevel,
		float SnowLevel,
		float MountainLevel)
	{
		check(BiomeIDOutput);
		check(BiomeWeightsOutput);
		check(HeightmapInput);
		check(TemperatureInput);
		check(HumidityInput);

		// Get texture dimensions
		FRDGTextureDesc OutputDesc = BiomeIDOutput->Desc.Texture->Desc;

		// Setup shader parameters
		FBiomeCalculationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBiomeCalculationCS::FParameters>();
		PassParameters->HeightmapInput = HeightmapInput;
		PassParameters->TemperatureInput = TemperatureInput;
		PassParameters->HumidityInput = HumidityInput;
		PassParameters->BiomeIDOutput = BiomeIDOutput;
		PassParameters->BiomeWeightsOutput = BiomeWeightsOutput;
		PassParameters->Seed = Seed;
		PassParameters->TerrainSize = FVector2f(TerrainSize);
		PassParameters->SeaLevel = SeaLevel;
		PassParameters->SnowLevel = SnowLevel;
		PassParameters->MountainLevel = MountainLevel;
		PassParameters->TemperatureMin = WhittakerParams.TemperatureMin;
		PassParameters->TemperatureMax = WhittakerParams.TemperatureMax;
		PassParameters->HumidityMin = WhittakerParams.HumidityMin;
		PassParameters->HumidityMax = WhittakerParams.HumidityMax;
		PassParameters->BlendDistance = 0.1f;
		PassParameters->EnableSmoothing = 1;

		// Get shader
		TShaderMapRef<FBiomeCalculationCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// Calculate thread groups
		const uint32 ThreadGroupSizeX = 8;
		const uint32 ThreadGroupSizeY = 8;
		const uint32 ThreadGroupCountX = FMath::DivideAndRoundUp(OutputDesc.Extent.X, ThreadGroupSizeX);
		const uint32 ThreadGroupCountY = FMath::DivideAndRoundUp(OutputDesc.Extent.Y, ThreadGroupSizeY);

		// Add compute pass
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("CalculateBiomes"),
			ComputeShader,
			PassParameters,
			FIntVector(ThreadGroupCountX, ThreadGroupCountY, 1)
		);
	}

	void GenerateCompleteBiomeData(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef BiomeIDOutput,
		FRDGTextureUAVRef BiomeWeightsOutput,
		FRDGTextureSRVRef HeightmapInput,
		uint32 Seed,
		FVector2D TerrainSize)
	{
		// Get texture dimensions from output
		FRDGTextureDesc BiomeDesc = BiomeIDOutput->Desc.Texture->Desc;

		// Create temporary textures for temperature and humidity
		FRDGTextureDesc TempDesc = FRDGTextureDesc::Create2D(
			BiomeDesc.Extent,
			PF_R32_FLOAT,
			FClearValueBinding::Black,
			TexCreate_ShaderResource | TexCreate_UAV
		);

		FRDGTextureRef TemperatureTexture = GraphBuilder.CreateTexture(TempDesc, TEXT("TemperatureMap"));
		FRDGTextureRef HumidityTexture = GraphBuilder.CreateTexture(TempDesc, TEXT("HumidityMap"));

		FRDGTextureUAVRef TemperatureUAV = GraphBuilder.CreateUAV(TemperatureTexture);
		FRDGTextureUAVRef HumidityUAV = GraphBuilder.CreateUAV(HumidityTexture);

		// Generate temperature map
		GenerateTemperatureMap(
			GraphBuilder,
			TemperatureUAV,
			HeightmapInput,
			Seed,
			TerrainSize,
			1.0f,   // LatitudeEffect
			0.006f, // AltitudeEffect (6°C per 1000m)
			0.3f    // NoiseScale
		);

		// Generate humidity map
		GenerateHumidityMap(
			GraphBuilder,
			HumidityUAV,
			HeightmapInput,
			Seed + 1000,
			TerrainSize,
			0.0f,  // SeaLevel
			1.0f,  // OceanProximityEffect
			0.5f   // NoiseScale
		);

		// Create SRVs for the temperature and humidity textures
		FRDGTextureSRVRef TemperatureSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(TemperatureTexture));
		FRDGTextureSRVRef HumiditySRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(HumidityTexture));

		// Calculate biomes using Whittaker diagram
		FWhittakerParameters WhittakerParams;
		WhittakerParams.TemperatureMin = -20.0f;
		WhittakerParams.TemperatureMax = 40.0f;
		WhittakerParams.HumidityMin = 0.0f;
		WhittakerParams.HumidityMax = 1.0f;

		CalculateBiomes(
			GraphBuilder,
			BiomeIDOutput,
			BiomeWeightsOutput,
			HeightmapInput,
			TemperatureSRV,
			HumiditySRV,
			Seed,
			TerrainSize,
			WhittakerParams,
			0.0f,    // SeaLevel
			1000.0f, // SnowLevel
			800.0f   // MountainLevel
		);
	}
}
