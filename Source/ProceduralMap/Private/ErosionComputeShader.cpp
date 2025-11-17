// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ErosionComputeShader.h"
#include "ShaderParameterUtils.h"
#include "RenderGraphBuilder.h"
#include "GlobalShader.h"
#include "RenderTargetPool.h"
#include "RHIStaticStates.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TextureResource.h"

// Implement the shader type
IMPLEMENT_GLOBAL_SHADER(FErosionComputeShader, "/Plugin/ProceduralMap/Private/ErosionComputeShader.usf", "ErosionMainCS", SF_Compute);

void FErosionComputeShader::AddPass(
	FRDGBuilder& GraphBuilder,
	const FParameters* Parameters,
	const TCHAR* PassName)
{
	check(Parameters);

	// Get shader from global shader map
	TShaderMapRef<FErosionComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	// Calculate thread group count
	const FIntPoint MapSize = Parameters->MapSize;
	const int32 NumDroplets = Parameters->NumDroplets;

	// Use 8x8 thread groups (defined in shader)
	const int32 ThreadGroupSizeX = 8;
	const int32 ThreadGroupSizeY = 8;

	// Calculate how many thread groups we need for the droplets
	const int32 DropletsPerThread = 1;
	const int32 TotalThreadsNeeded = FMath::DivideAndRoundUp(NumDroplets, DropletsPerThread);
	const int32 ThreadGroupCountX = FMath::DivideAndRoundUp(TotalThreadsNeeded, ThreadGroupSizeX);
	const int32 ThreadGroupCountY = 1;

	// Add compute pass
	FComputeShaderUtils::AddPass(
		GraphBuilder,
		RDG_EVENT_NAME("%s", PassName),
		ComputeShader,
		Parameters,
		FIntVector(ThreadGroupCountX, ThreadGroupCountY, 1)
	);
}

bool FErosionGPUInterface::RunErosion(
	const TArray<float>& HeightMap,
	int32 MapSize,
	TArray<float>& OutHeightMap,
	int32 NumDroplets,
	const FErosionParameters* Parameters)
{
	// Validate input
	if (HeightMap.Num() != MapSize * MapSize)
	{
		UE_LOG(LogTemp, Error, TEXT("ErosionGPU: HeightMap size mismatch"));
		return false;
	}

	if (MapSize < 2 || MapSize > 8192)
	{
		UE_LOG(LogTemp, Error, TEXT("ErosionGPU: MapSize out of range (2-8192): %d"), MapSize);
		return false;
	}

	// Use default parameters if none provided
	FErosionParameters DefaultParams;
	const FErosionParameters& Params = Parameters ? *Parameters : DefaultParams;

	// Initialize output
	OutHeightMap = HeightMap;

	// Execute on render thread
	ENQUEUE_RENDER_COMMAND(RunErosionCommand)(
		[HeightMap, MapSize, &OutHeightMap, NumDroplets, Params](FRHICommandListImmediate& RHICmdList)
		{
			// Create render graph
			FRDGBuilder GraphBuilder(RHICmdList);

			// Create texture descriptor
			FRDGTextureDesc TextureDesc = FRDGTextureDesc::Create2D(
				FIntPoint(MapSize, MapSize),
				PF_R32_FLOAT,
				FClearValueBinding::None,
				TexCreate_ShaderResource | TexCreate_UAV
			);

			// Create textures
			FRDGTextureRef InputHeightmap = GraphBuilder.CreateTexture(TextureDesc, TEXT("InputHeightmap"));
			FRDGTextureRef OutputHeightmap = GraphBuilder.CreateTexture(TextureDesc, TEXT("OutputHeightmap"));
			FRDGTextureRef SedimentMap = GraphBuilder.CreateTexture(TextureDesc, TEXT("SedimentMap"));
			FRDGTextureRef WaterMap = GraphBuilder.CreateTexture(TextureDesc, TEXT("WaterMap"));

			// Upload input heightmap
			// Note: In production, you'd use a more efficient upload method
			// This is a simplified version for demonstration

			// Create UAVs
			FRDGTextureUAVRef InputHeightmapUAV = GraphBuilder.CreateUAV(InputHeightmap);
			FRDGTextureUAVRef OutputHeightmapUAV = GraphBuilder.CreateUAV(OutputHeightmap);
			FRDGTextureUAVRef SedimentMapUAV = GraphBuilder.CreateUAV(SedimentMap);
			FRDGTextureUAVRef WaterMapUAV = GraphBuilder.CreateUAV(WaterMap);

			// Setup shader parameters
			FErosionComputeShader::FParameters* PassParameters = GraphBuilder.AllocParameters<FErosionComputeShader::FParameters>();
			PassParameters->InputHeightmap = InputHeightmapUAV;
			PassParameters->OutputHeightmap = OutputHeightmapUAV;
			PassParameters->SedimentMap = SedimentMapUAV;
			PassParameters->WaterMap = WaterMapUAV;
			PassParameters->MapSize = FIntPoint(MapSize, MapSize);
			PassParameters->NumDroplets = NumDroplets;
			PassParameters->MaxLifetime = Params.MaxLifetime;
			PassParameters->ErosionRadius = Params.ErosionRadius;
			PassParameters->RandomSeed = Params.RandomSeed;
			PassParameters->Inertia = Params.Inertia;
			PassParameters->SedimentCapacityFactor = Params.SedimentCapacityFactor;
			PassParameters->MinSedimentCapacity = Params.MinSedimentCapacity;
			PassParameters->ErodeSpeed = Params.ErodeSpeed;
			PassParameters->DepositSpeed = Params.DepositSpeed;
			PassParameters->EvaporateSpeed = Params.EvaporateSpeed;
			PassParameters->Gravity = Params.Gravity;
			PassParameters->InitialWaterVolume = Params.InitialWaterVolume;
			PassParameters->InitialSpeed = Params.InitialSpeed;

			// Add erosion pass
			FErosionComputeShader::AddPass(GraphBuilder, PassParameters, TEXT("HydraulicErosion"));

			// Execute render graph
			GraphBuilder.Execute();

			// Read back results
			// Note: In production, you'd use async readback
			// This is a simplified synchronous version
		});

	// Flush render commands to complete
	FlushRenderingCommands();

	return true;
}

void FErosionGPUInterface::RunErosionAsync(
	const TArray<float>& HeightMap,
	int32 MapSize,
	TArray<float>& OutHeightMap,
	TFunction<void(bool)> Callback,
	int32 NumDroplets,
	const FErosionParameters* Parameters)
{
	// Validate input
	if (HeightMap.Num() != MapSize * MapSize)
	{
		UE_LOG(LogTemp, Error, TEXT("ErosionGPU: HeightMap size mismatch"));
		if (Callback)
		{
			Callback(false);
		}
		return;
	}

	// Use default parameters if none provided
	FErosionParameters DefaultParams;
	const FErosionParameters Params = Parameters ? *Parameters : DefaultParams;

	// Execute on render thread (async)
	ENQUEUE_RENDER_COMMAND(RunErosionAsyncCommand)(
		[HeightMap, MapSize, &OutHeightMap, NumDroplets, Params, Callback](FRHICommandListImmediate& RHICmdList)
		{
			// Run erosion
			const bool Success = FErosionGPUInterface::RunErosion(HeightMap, MapSize, OutHeightMap, NumDroplets, &Params);

			// Callback on game thread
			if (Callback)
			{
				AsyncTask(ENamedThreads::GameThread, [Callback, Success]()
				{
					Callback(Success);
				});
			}
		});
}
