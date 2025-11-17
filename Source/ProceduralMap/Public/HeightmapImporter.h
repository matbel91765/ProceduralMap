// Ultimate Procedural Map Generation System - Heightmap Import System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Landscape.h"
#include "HeightmapImporter.generated.h"

/**
 * Import format types for heightmap data
 */
UENUM(BlueprintType)
enum class EHeightmapImportFormat : uint8
{
	RAW_16Bit UMETA(DisplayName = "RAW 16-bit"),
	PNG UMETA(DisplayName = "PNG (8 or 16-bit)"),
	EXR UMETA(DisplayName = "EXR 32-bit Float"),
	TGA UMETA(DisplayName = "TGA"),
	Auto UMETA(DisplayName = "Auto-Detect")
};

/**
 * Import parameters structure
 */
USTRUCT(BlueprintType)
struct FHeightmapImportParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
	EHeightmapImportFormat Format = EHeightmapImportFormat::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
	FString FilePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Resolution")
	int32 ExpectedWidth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Resolution")
	int32 ExpectedHeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Options")
	bool bAutoNormalize = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Options")
	bool bValidatePowerOfTwo = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Options")
	bool bFlipY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Range")
	float MinHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Range")
	float MaxHeight = 1000.0f;
};

/**
 * Heightmap Importer - Imports heightmap data from various formats
 * Supports RAW, PNG, EXR, and TGA formats with validation
 */
UCLASS()
class PROCEDURALMAP_API UHeightmapImporter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// MAIN IMPORT FUNCTIONS
	// ============================================================================

	/**
	 * Import heightmap from RAW format (16-bit grayscale)
	 * @param FilePath - Path to RAW file
	 * @param Width - Expected width of heightmap
	 * @param Height - Expected height of heightmap
	 * @param OutHeightData - Output height data array
	 * @param MinHeight - Minimum height value for denormalization
	 * @param MaxHeight - Maximum height value for denormalization
	 * @return True if import succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Import")
	static bool ImportHeightmapFromRAW(
		const FString& FilePath,
		int32 Width,
		int32 Height,
		TArray<float>& OutHeightData,
		float MinHeight = 0.0f,
		float MaxHeight = 1000.0f
	);

	/**
	 * Import heightmap from PNG format (8-bit or 16-bit)
	 * @param FilePath - Path to PNG file
	 * @param OutHeightData - Output height data array
	 * @param OutWidth - Output width
	 * @param OutHeight - Output height
	 * @param MinHeight - Minimum height value for denormalization
	 * @param MaxHeight - Maximum height value for denormalization
	 * @return True if import succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Import")
	static bool ImportHeightmapFromPNG(
		const FString& FilePath,
		TArray<float>& OutHeightData,
		int32& OutWidth,
		int32& OutHeight,
		float MinHeight = 0.0f,
		float MaxHeight = 1000.0f
	);

	/**
	 * Import heightmap from EXR format (32-bit float, high precision)
	 * @param FilePath - Path to EXR file
	 * @param OutHeightData - Output height data array
	 * @param OutWidth - Output width
	 * @param OutHeight - Output height
	 * @return True if import succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Import")
	static bool ImportHeightmapFromEXR(
		const FString& FilePath,
		TArray<float>& OutHeightData,
		int32& OutWidth,
		int32& OutHeight
	);

	/**
	 * Import heightmap from TGA format
	 * @param FilePath - Path to TGA file
	 * @param OutHeightData - Output height data array
	 * @param OutWidth - Output width
	 * @param OutHeight - Output height
	 * @param MinHeight - Minimum height value for denormalization
	 * @param MaxHeight - Maximum height value for denormalization
	 * @return True if import succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Import")
	static bool ImportHeightmapFromTGA(
		const FString& FilePath,
		TArray<float>& OutHeightData,
		int32& OutWidth,
		int32& OutHeight,
		float MinHeight = 0.0f,
		float MaxHeight = 1000.0f
	);

	/**
	 * Import heightmap with auto-format detection
	 * @param Params - Import parameters
	 * @param OutHeightData - Output height data array
	 * @param OutWidth - Output width
	 * @param OutHeight - Output height
	 * @return True if import succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Import")
	static bool ImportHeightmap(
		const FHeightmapImportParams& Params,
		TArray<float>& OutHeightData,
		int32& OutWidth,
		int32& OutHeight
	);

	// ============================================================================
	// LANDSCAPE APPLICATION
	// ============================================================================

	/**
	 * Apply heightmap data to Unreal Landscape
	 * @param Landscape - Landscape actor to modify
	 * @param HeightData - Height data to apply
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @return True if application succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Apply")
	static bool ApplyToLandscape(
		ALandscape* Landscape,
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height
	);

	/**
	 * Import and apply heightmap directly to Landscape
	 * @param Landscape - Landscape actor to modify
	 * @param Params - Import parameters
	 * @return True if import and application succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Apply")
	static bool ImportAndApplyToLandscape(
		ALandscape* Landscape,
		const FHeightmapImportParams& Params
	);

	/**
	 * Apply heightmap to procedural terrain system
	 * @param HeightData - Height data to apply
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param OutProceduralData - Output procedural data
	 * @return True if application succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Apply")
	static bool ApplyToProceduralTerrain(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		TArray<float>& OutProceduralData
	);

	// ============================================================================
	// VALIDATION
	// ============================================================================

	/**
	 * Validate heightmap resolution (power-of-two check for Unreal)
	 * @param Width - Width to validate
	 * @param Height - Height to validate
	 * @return True if valid for Unreal Engine
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Validation")
	static bool ValidateResolution(int32 Width, int32 Height);

	/**
	 * Get recommended resolution for Unreal Landscape
	 * @param Width - Input width
	 * @param Height - Input height
	 * @param OutRecommendedWidth - Output recommended width
	 * @param OutRecommendedHeight - Output recommended height
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Validation")
	static void GetRecommendedResolution(
		int32 Width,
		int32 Height,
		int32& OutRecommendedWidth,
		int32& OutRecommendedHeight
	);

	/**
	 * Detect format from file extension
	 * @param FilePath - Path to file
	 * @return Detected format
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Utility")
	static EHeightmapImportFormat DetectFormat(const FString& FilePath);

private:
	// Helper functions
	static bool ReadRAWFile(const FString& FilePath, int32 Width, int32 Height, TArray<uint16>& OutData);
	static bool ReadImageFile(const FString& FilePath, TArray<uint8>& OutData, int32& OutWidth, int32& OutHeight, int32& OutBitDepth);

	static void Convert16BitToFloat(const TArray<uint16>& InData, TArray<float>& OutData, float MinHeight, float MaxHeight);
	static void Convert8BitToFloat(const TArray<uint8>& InData, TArray<float>& OutData, float MinHeight, float MaxHeight);
	static void ConvertRGBAToFloat(const TArray<uint8>& InData, TArray<float>& OutData, int32 Width, int32 Height, float MinHeight, float MaxHeight);

	static void NormalizeHeightData(TArray<float>& Data);
	static void FlipVertical(TArray<float>& Data, int32 Width, int32 Height);

	static bool IsPowerOfTwo(int32 Value);
	static int32 GetNearestPowerOfTwo(int32 Value);
	static int32 GetNearestUnrealResolution(int32 Value);
};
