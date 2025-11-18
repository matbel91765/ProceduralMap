// Copyright Epic Games, Inc. All Rights Reserved.

#include "NormalComputeShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterUtils.h"
#include "GlobalShader.h"
#include "RHIStaticStates.h"
#include "PipelineStateCache.h"

// Implement shader classes
IMPLEMENT_GLOBAL_SHADER(FNormalCalculationCS, "/Plugin/ProceduralMap/Private/NormalComputeShader.usf", "CalculateNormalsCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FNormalSmoothingCS, "/Plugin/ProceduralMap/Private/NormalComputeShader.usf", "SmoothNormalsCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FEdgeDetectionCS, "/Plugin/ProceduralMap/Private/NormalComputeShader.usf", "DetectEdgesCS", SF_Compute);

namespace NormalComputeShaderUtils
{
	void CalculateNormals(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef NormalOutput,
		FRDGTextureSRVRef HeightmapInput,
		float HeightScale,
		float HorizontalScale,
		ENormalCalculationMethod Method,
		bool bEnableSmoothing,
		int32 SmoothKernelSize)
	{
		check(NormalOutput);
		check(HeightmapInput);

		// Get texture dimensions
		FRDGTextureDesc OutputDesc = NormalOutput->Desc.Texture->Desc;

		// Setup shader parameters
		FNormalCalculationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FNormalCalculationCS::FParameters>();
		PassParameters->HeightmapInput = HeightmapInput;
		PassParameters->NormalOutput = NormalOutput;
		PassParameters->HeightScale = HeightScale;
		PassParameters->HorizontalScale = HorizontalScale;
		PassParameters->CalculationMethod = static_cast<int32>(Method);
		PassParameters->EnableSmoothing = bEnableSmoothing ? 1 : 0;
		PassParameters->SmoothKernelSize = SmoothKernelSize;
		PassParameters->TextureSize = FVector2f(OutputDesc.Extent.X, OutputDesc.Extent.Y);

		// Get shader
		TShaderMapRef<FNormalCalculationCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// Calculate thread groups
		const uint32 ThreadGroupSizeX = 8;
		const uint32 ThreadGroupSizeY = 8;
		const uint32 ThreadGroupCountX = FMath::DivideAndRoundUp(OutputDesc.Extent.X, ThreadGroupSizeX);
		const uint32 ThreadGroupCountY = FMath::DivideAndRoundUp(OutputDesc.Extent.Y, ThreadGroupSizeY);

		// Add compute pass
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("CalculateNormals"),
			ComputeShader,
			PassParameters,
			FIntVector(ThreadGroupCountX, ThreadGroupCountY, 1)
		);
	}

	void SmoothNormals(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef NormalOutput,
		FRDGTextureSRVRef NormalInput,
		int32 KernelSize,
		float Sigma)
	{
		check(NormalOutput);
		check(NormalInput);

		// Get texture dimensions
		FRDGTextureDesc OutputDesc = NormalOutput->Desc.Texture->Desc;

		// Setup shader parameters
		FNormalSmoothingCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FNormalSmoothingCS::FParameters>();
		PassParameters->NormalInput = NormalInput;
		PassParameters->NormalOutput = NormalOutput;
		PassParameters->KernelSize = KernelSize;
		PassParameters->Sigma = Sigma;
		PassParameters->TextureSize = FVector2f(OutputDesc.Extent.X, OutputDesc.Extent.Y);

		// Get shader
		TShaderMapRef<FNormalSmoothingCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// Calculate thread groups
		const uint32 ThreadGroupSizeX = 8;
		const uint32 ThreadGroupSizeY = 8;
		const uint32 ThreadGroupCountX = FMath::DivideAndRoundUp(OutputDesc.Extent.X, ThreadGroupSizeX);
		const uint32 ThreadGroupCountY = FMath::DivideAndRoundUp(OutputDesc.Extent.Y, ThreadGroupSizeY);

		// Add compute pass
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("SmoothNormals"),
			ComputeShader,
			PassParameters,
			FIntVector(ThreadGroupCountX, ThreadGroupCountY, 1)
		);
	}

	void DetectEdges(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef EdgeOutput,
		FRDGTextureSRVRef HeightmapInput,
		float Threshold,
		float HeightScale)
	{
		check(EdgeOutput);
		check(HeightmapInput);

		// Get texture dimensions
		FRDGTextureDesc OutputDesc = EdgeOutput->Desc.Texture->Desc;

		// Setup shader parameters
		FEdgeDetectionCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FEdgeDetectionCS::FParameters>();
		PassParameters->HeightmapInput = HeightmapInput;
		PassParameters->EdgeOutput = EdgeOutput;
		PassParameters->Threshold = Threshold;
		PassParameters->HeightScale = HeightScale;
		PassParameters->TextureSize = FVector2f(OutputDesc.Extent.X, OutputDesc.Extent.Y);

		// Get shader
		TShaderMapRef<FEdgeDetectionCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// Calculate thread groups
		const uint32 ThreadGroupSizeX = 8;
		const uint32 ThreadGroupSizeY = 8;
		const uint32 ThreadGroupCountX = FMath::DivideAndRoundUp(OutputDesc.Extent.X, ThreadGroupSizeX);
		const uint32 ThreadGroupCountY = FMath::DivideAndRoundUp(OutputDesc.Extent.Y, ThreadGroupSizeY);

		// Add compute pass
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("DetectEdges"),
			ComputeShader,
			PassParameters,
			FIntVector(ThreadGroupCountX, ThreadGroupCountY, 1)
		);
	}

	void CalculateSmoothedNormals(
		FRDGBuilder& GraphBuilder,
		FRDGTextureUAVRef NormalOutput,
		FRDGTextureSRVRef HeightmapInput,
		float HeightScale,
		float HorizontalScale,
		int32 SmoothingPasses)
	{
		// Get texture dimensions
		FRDGTextureDesc OutputDesc = NormalOutput->Desc.Texture->Desc;

		// First, calculate normals
		CalculateNormals(
			GraphBuilder,
			NormalOutput,
			HeightmapInput,
			HeightScale,
			HorizontalScale,
			ENormalCalculationMethod::Sobel,
			false,
			3
		);

		// Apply smoothing passes if requested
		if (SmoothingPasses > 0)
		{
			// Create temporary texture for ping-pong smoothing
			FRDGTextureDesc TempDesc = FRDGTextureDesc::Create2D(
				OutputDesc.Extent,
				PF_A32B32G32R32F,
				FClearValueBinding::Black,
				TexCreate_ShaderResource | TexCreate_UAV
			);

			FRDGTextureRef TempTexture = GraphBuilder.CreateTexture(TempDesc, TEXT("TempNormalMap"));

			for (int32 i = 0; i < SmoothingPasses; i++)
			{
				// Determine input and output for ping-pong
				bool bEvenPass = (i % 2 == 0);

				if (bEvenPass)
				{
					// Read from NormalOutput, write to TempTexture
					FRDGTextureSRVRef InputSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(NormalOutput->Desc.Texture));
					FRDGTextureUAVRef TempUAV = GraphBuilder.CreateUAV(TempTexture);

					SmoothNormals(
						GraphBuilder,
						TempUAV,
						InputSRV,
						3,
						1.0f
					);
				}
				else
				{
					// Read from TempTexture, write to NormalOutput
					FRDGTextureSRVRef TempSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(TempTexture));

					SmoothNormals(
						GraphBuilder,
						NormalOutput,
						TempSRV,
						3,
						1.0f
					);
				}
			}

			// If we ended on an even pass, copy temp back to output
			if (SmoothingPasses % 2 == 1)
			{
				// The final result is in TempTexture, need to copy it
				FRDGTextureSRVRef TempSRV = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(TempTexture));

				SmoothNormals(
					GraphBuilder,
					NormalOutput,
					TempSRV,
					3,
					1.0f
				);
			}
		}
	}
}
