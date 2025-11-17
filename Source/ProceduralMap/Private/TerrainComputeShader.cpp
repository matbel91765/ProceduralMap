// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainComputeShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterUtils.h"
#include "GlobalShader.h"
#include "RHIStaticStates.h"
#include "PipelineStateCache.h"

// Implement the shader class
IMPLEMENT_GLOBAL_SHADER(FTerrainGenerationCS, "/Plugin/ProceduralMap/Private/TerrainComputeShader.usf", "GenerateTerrainCS", SF_Compute);

namespace TerrainComputeShaderUtils
{
	void DispatchTerrainGeneration(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef OutputTexture,
		const FTerrainNoiseParameters& NoiseParams,
		uint32 Seed,
		FVector2D TerrainSize,
		FVector2D Offset,
		ETerrainGenerationPass PassType,
		FVector4 BiomeWeights)
	{
		check(OutputTexture);

		// Get the output texture description
		FRDGTextureDesc OutputDesc = OutputTexture->Desc.Texture->Desc;

		// Setup shader parameters
		FTerrainGenerationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FTerrainGenerationCS::FParameters>();
		PassParameters->OutputHeightmap = OutputTexture;
		PassParameters->Frequency = NoiseParams.Frequency;
		PassParameters->Octaves = NoiseParams.Octaves;
		PassParameters->Lacunarity = NoiseParams.Lacunarity;
		PassParameters->Gain = NoiseParams.Gain;
		PassParameters->Amplitude = NoiseParams.Amplitude;
		PassParameters->Seed = Seed;
		PassParameters->TerrainSize = FVector2f(TerrainSize);
		PassParameters->Offset = FVector2f(Offset);
		PassParameters->PassType = static_cast<int32>(PassType);
		PassParameters->BiomeWeights = FVector4f(BiomeWeights);

		// Advanced parameters based on pass type
		switch (PassType)
		{
		case ETerrainGenerationPass::Continental:
			PassParameters->Persistence = 0.5f;
			PassParameters->Scale = 1000.0f;
			PassParameters->EnableRidgedNoise = 0;
			PassParameters->RidgedSharpness = 1.0f;
			break;

		case ETerrainGenerationPass::Mountains:
			PassParameters->Persistence = 0.6f;
			PassParameters->Scale = 500.0f;
			PassParameters->EnableRidgedNoise = 1;
			PassParameters->RidgedSharpness = 2.0f;
			break;

		case ETerrainGenerationPass::Details:
			PassParameters->Persistence = 0.4f;
			PassParameters->Scale = 100.0f;
			PassParameters->EnableRidgedNoise = 0;
			PassParameters->RidgedSharpness = 1.0f;
			break;
		}

		// Get shader from global shader map
		TShaderMapRef<FTerrainGenerationCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// Calculate thread group counts
		const uint32 ThreadGroupSizeX = 8;
		const uint32 ThreadGroupSizeY = 8;
		const uint32 ThreadGroupCountX = FMath::DivideAndRoundUp(OutputDesc.Extent.X, ThreadGroupSizeX);
		const uint32 ThreadGroupCountY = FMath::DivideAndRoundUp(OutputDesc.Extent.Y, ThreadGroupSizeY);

		// Add compute pass to render graph
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("TerrainGeneration_%s",
				PassType == ETerrainGenerationPass::Continental ? TEXT("Continental") :
				PassType == ETerrainGenerationPass::Mountains ? TEXT("Mountains") : TEXT("Details")),
			ComputeShader,
			PassParameters,
			FIntVector(ThreadGroupCountX, ThreadGroupCountY, 1)
		);
	}

	void DispatchMultiPassTerrainGeneration(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef OutputTexture,
		uint32 Seed,
		FVector2D TerrainSize,
		FVector2D Offset)
	{
		// Pass 1: Continental shapes (large scale, low frequency)
		{
			FTerrainNoiseParameters ContinentalParams;
			ContinentalParams.Frequency = 0.0005f;
			ContinentalParams.Octaves = 4;
			ContinentalParams.Lacunarity = 2.0f;
			ContinentalParams.Gain = 0.5f;
			ContinentalParams.Amplitude = 1000.0f;

			DispatchTerrainGeneration(
				GraphBuilder,
				OutputTexture,
				ContinentalParams,
				Seed,
				TerrainSize,
				Offset,
				ETerrainGenerationPass::Continental,
				FVector4(1, 0, 0, 0)
			);
		}

		// Pass 2: Mountain ranges (medium scale, medium frequency)
		{
			FTerrainNoiseParameters MountainParams;
			MountainParams.Frequency = 0.002f;
			MountainParams.Octaves = 6;
			MountainParams.Lacunarity = 2.2f;
			MountainParams.Gain = 0.6f;
			MountainParams.Amplitude = 500.0f;

			DispatchTerrainGeneration(
				GraphBuilder,
				OutputTexture,
				MountainParams,
				Seed + 1000,
				TerrainSize,
				Offset,
				ETerrainGenerationPass::Mountains,
				FVector4(0, 1, 0, 0)
			);
		}

		// Pass 3: Fine details (small scale, high frequency)
		{
			FTerrainNoiseParameters DetailParams;
			DetailParams.Frequency = 0.01f;
			DetailParams.Octaves = 8;
			DetailParams.Lacunarity = 2.5f;
			DetailParams.Gain = 0.4f;
			DetailParams.Amplitude = 50.0f;

			DispatchTerrainGeneration(
				GraphBuilder,
				OutputTexture,
				DetailParams,
				Seed + 2000,
				TerrainSize,
				Offset,
				ETerrainGenerationPass::Details,
				FVector4(0, 0, 1, 0)
			);
		}
	}
}
