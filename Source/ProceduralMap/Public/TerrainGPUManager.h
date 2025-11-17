// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RenderGraphFwd.h"
#include "TerrainComputeShader.h"
#include "BiomeComputeShader.h"
#include "NormalComputeShader.h"
#include "TerrainGPUManager.generated.h"

class UTexture2D;
class UTextureRenderTarget2D;

/**
 * Configuration for terrain generation pipeline
 */
USTRUCT(BlueprintType)
struct FTerrainGenerationConfig
{
	GENERATED_BODY()

	// Heightmap resolution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution")
	int32 HeightmapResolution = 1024;

	// Terrain size in world units
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
	FVector2D TerrainSize = FVector2D(10000, 10000);

	// Random seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 Seed = 12345;

	// Noise parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	FTerrainNoiseParameters NoiseParameters;

	// Generate biome data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	bool bGenerateBiomes = true;

	// Generate normal map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normals")
	bool bGenerateNormals = true;

	// Normal calculation method
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normals")
	ENormalCalculationMethod NormalMethod = ENormalCalculationMethod::Sobel;

	// Height scale for normal calculation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normals")
	float NormalHeightScale = 100.0f;

	// Enable multi-pass generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bUseMultiPass = true;

	// Enable GPU-only mode (no readback)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bGPUOnlyMode = false;
};

/**
 * Result data from GPU terrain generation
 */
USTRUCT(BlueprintType)
struct FTerrainGenerationResult
{
	GENERATED_BODY()

	// Generated heightmap texture
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TObjectPtr<UTexture2D> HeightmapTexture;

	// Generated biome ID map
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TObjectPtr<UTexture2D> BiomeIDTexture;

	// Generated biome weights map
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TObjectPtr<UTexture2D> BiomeWeightsTexture;

	// Generated normal map
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TObjectPtr<UTexture2D> NormalMapTexture;

	// Generation time in milliseconds
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float GenerationTimeMs = 0.0f;

	// Was generation successful
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;
};

/**
 * GPU manager for orchestrating terrain generation compute shaders
 * Handles the complete pipeline: heightmap -> biomes -> normals
 */
UCLASS(BlueprintType)
class PROCEDURALMAP_API UTerrainGPUManager : public UObject
{
	GENERATED_BODY()

public:
	UTerrainGPUManager();

	/**
	 * Generate complete terrain data on GPU
	 * @param Config - Generation configuration
	 * @return Generation result with all textures
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	FTerrainGenerationResult GenerateTerrainComplete(const FTerrainGenerationConfig& Config);

	/**
	 * Generate heightmap only
	 * @param Config - Generation configuration
	 * @return Heightmap texture
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	UTexture2D* GenerateHeightmap(const FTerrainGenerationConfig& Config);

	/**
	 * Generate biome data from heightmap
	 * @param HeightmapTexture - Input heightmap
	 * @param Seed - Random seed
	 * @param OutBiomeID - Output biome ID texture
	 * @param OutBiomeWeights - Output biome weights texture
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void GenerateBiomes(
		UTexture2D* HeightmapTexture,
		int32 Seed,
		UTexture2D*& OutBiomeID,
		UTexture2D*& OutBiomeWeights
	);

	/**
	 * Generate normal map from heightmap
	 * @param HeightmapTexture - Input heightmap
	 * @param HeightScale - Scale factor for height
	 * @param Method - Normal calculation method
	 * @return Normal map texture
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	UTexture2D* GenerateNormalMap(
		UTexture2D* HeightmapTexture,
		float HeightScale = 100.0f,
		ENormalCalculationMethod Method = ENormalCalculationMethod::Sobel
	);

	/**
	 * Async generation with callback
	 * Generates terrain on render thread and calls callback when done
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void GenerateTerrainAsync(
		const FTerrainGenerationConfig& Config,
		const FOnTerrainGenerationComplete& OnComplete
	);

	/**
	 * Get last generation time
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain Generation")
	float GetLastGenerationTime() const { return LastGenerationTimeMs; }

	/**
	 * Check if generation is currently in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain Generation")
	bool IsGenerating() const { return bIsGenerating; }

protected:
	/**
	 * Internal method to execute terrain generation on render thread
	 */
	void ExecuteTerrainGeneration_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FTerrainGenerationConfig& Config,
		FRDGTextureRef& OutHeightmap,
		FRDGTextureRef& OutBiomeID,
		FRDGTextureRef& OutBiomeWeights,
		FRDGTextureRef& OutNormalMap
	);

	/**
	 * Create RDG textures for terrain generation
	 */
	void CreateTerrainTextures_RenderThread(
		FRDGBuilder& GraphBuilder,
		int32 Resolution,
		FRDGTextureRef& OutHeightmap,
		FRDGTextureRef& OutBiomeID,
		FRDGTextureRef& OutBiomeWeights,
		FRDGTextureRef& OutNormalMap
	);

	/**
	 * Readback GPU textures to CPU
	 */
	void ReadbackTextures(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef HeightmapTexture,
		FRDGTextureRef BiomeIDTexture,
		FRDGTextureRef BiomeWeightsTexture,
		FRDGTextureRef NormalMapTexture,
		FTerrainGenerationResult& OutResult
	);

	/**
	 * Convert RDG texture to UTexture2D
	 */
	UTexture2D* ConvertRDGTextureToTexture2D(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef RDGTexture,
		const FString& TextureName
	);

private:
	// Last generation time in milliseconds
	UPROPERTY(Transient)
	float LastGenerationTimeMs;

	// Is generation currently in progress
	UPROPERTY(Transient)
	bool bIsGenerating;

	// Cached render targets for GPU-only mode
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CachedHeightmapRT;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CachedBiomeIDRT;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CachedBiomeWeightsRT;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CachedNormalMapRT;
};

/**
 * Delegate for async terrain generation completion
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTerrainGenerationComplete, const FTerrainGenerationResult&, Result);

/**
 * Static utility class for terrain GPU operations
 */
class PROCEDURALMAP_API FTerrainGPUUtils
{
public:
	/**
	 * Execute complete terrain generation pipeline in RDG
	 * This is the main entry point for GPU terrain generation
	 */
	static void ExecuteTerrainPipeline(
		FRDGBuilder& GraphBuilder,
		int32 Resolution,
		const FTerrainGenerationConfig& Config,
		FRDGTextureRef& OutHeightmap,
		FRDGTextureRef& OutBiomeID,
		FRDGTextureRef& OutBiomeWeights,
		FRDGTextureRef& OutNormalMap
	);

	/**
	 * Generate heightmap using compute shader
	 */
	static void GenerateHeightmapGPU(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef HeightmapTexture,
		const FTerrainGenerationConfig& Config
	);

	/**
	 * Generate biome data using compute shader
	 */
	static void GenerateBiomesGPU(
		FRDGBuilder& GraphBuilder,
		FRDGTextureSRVRef HeightmapSRV,
		FRDGTextureRef BiomeIDTexture,
		FRDGTextureRef BiomeWeightsTexture,
		uint32 Seed,
		FVector2D TerrainSize
	);

	/**
	 * Generate normal map using compute shader
	 */
	static void GenerateNormalMapGPU(
		FRDGBuilder& GraphBuilder,
		FRDGTextureSRVRef HeightmapSRV,
		FRDGTextureRef NormalMapTexture,
		float HeightScale,
		ENormalCalculationMethod Method
	);
};
