// Ultimate Procedural Map Generation System - Heightmap Converter
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeightmapConverter.generated.h"

/**
 * Resampling filter types
 */
UENUM(BlueprintType)
enum class EResamplingFilter : uint8
{
	Nearest UMETA(DisplayName = "Nearest Neighbor"),
	Bilinear UMETA(DisplayName = "Bilinear"),
	Bicubic UMETA(DisplayName = "Bicubic"),
	Lanczos UMETA(DisplayName = "Lanczos")
};

/**
 * Rotation angles
 */
UENUM(BlueprintType)
enum class ERotationAngle : uint8
{
	Rotate90 UMETA(DisplayName = "90 Degrees"),
	Rotate180 UMETA(DisplayName = "180 Degrees"),
	Rotate270 UMETA(DisplayName = "270 Degrees")
};

/**
 * Conversion parameters structure
 */
USTRUCT(BlueprintType)
struct FHeightmapConversionParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
	int32 TargetWidth = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
	int32 TargetHeight = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
	EResamplingFilter Filter = EResamplingFilter::Bilinear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
	bool bNormalize = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
	bool bClamp = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion|Range")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion|Range")
	float MaxValue = 1.0f;
};

/**
 * Heightmap Converter - Converts and processes heightmap data
 * Provides format conversion, resizing, normalization, and transformations
 */
UCLASS()
class PROCEDURALMAP_API UHeightmapConverter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// RESIZING AND RESAMPLING
	// ============================================================================

	/**
	 * Resize heightmap with specified filter
	 * @param HeightData - Input height data
	 * @param SourceWidth - Source width
	 * @param SourceHeight - Source height
	 * @param TargetWidth - Target width
	 * @param TargetHeight - Target height
	 * @param Filter - Resampling filter to use
	 * @param OutResizedData - Output resized data
	 * @return True if resize succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static bool ResizeHeightmap(
		const TArray<float>& HeightData,
		int32 SourceWidth,
		int32 SourceHeight,
		int32 TargetWidth,
		int32 TargetHeight,
		EResamplingFilter Filter,
		TArray<float>& OutResizedData
	);

	/**
	 * Resize heightmap using nearest neighbor (fast)
	 * @param HeightData - Input height data
	 * @param SourceWidth - Source width
	 * @param SourceHeight - Source height
	 * @param TargetWidth - Target width
	 * @param TargetHeight - Target height
	 * @param OutResizedData - Output resized data
	 * @return True if resize succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static bool ResizeNearestNeighbor(
		const TArray<float>& HeightData,
		int32 SourceWidth,
		int32 SourceHeight,
		int32 TargetWidth,
		int32 TargetHeight,
		TArray<float>& OutResizedData
	);

	/**
	 * Resize heightmap using bilinear interpolation (balanced)
	 * @param HeightData - Input height data
	 * @param SourceWidth - Source width
	 * @param SourceHeight - Source height
	 * @param TargetWidth - Target width
	 * @param TargetHeight - Target height
	 * @param OutResizedData - Output resized data
	 * @return True if resize succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static bool ResizeBilinear(
		const TArray<float>& HeightData,
		int32 SourceWidth,
		int32 SourceHeight,
		int32 TargetWidth,
		int32 TargetHeight,
		TArray<float>& OutResizedData
	);

	/**
	 * Resize heightmap using bicubic interpolation (high quality)
	 * @param HeightData - Input height data
	 * @param SourceWidth - Source width
	 * @param SourceHeight - Source height
	 * @param TargetWidth - Target width
	 * @param TargetHeight - Target height
	 * @param OutResizedData - Output resized data
	 * @return True if resize succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static bool ResizeBicubic(
		const TArray<float>& HeightData,
		int32 SourceWidth,
		int32 SourceHeight,
		int32 TargetWidth,
		int32 TargetHeight,
		TArray<float>& OutResizedData
	);

	// ============================================================================
	// NORMALIZATION AND CLAMPING
	// ============================================================================

	/**
	 * Normalize heightmap to 0-1 range
	 * @param HeightData - Height data to normalize (modified in place)
	 * @param OutMin - Output original minimum value
	 * @param OutMax - Output original maximum value
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static void NormalizeHeightmap(
		UPARAM(ref) TArray<float>& HeightData,
		float& OutMin,
		float& OutMax
	);

	/**
	 * Normalize heightmap to custom range
	 * @param HeightData - Height data to normalize (modified in place)
	 * @param TargetMin - Target minimum value
	 * @param TargetMax - Target maximum value
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static void NormalizeToRange(
		UPARAM(ref) TArray<float>& HeightData,
		float TargetMin,
		float TargetMax
	);

	/**
	 * Clamp heightmap values to specified range
	 * @param HeightData - Height data to clamp (modified in place)
	 * @param MinValue - Minimum value
	 * @param MaxValue - Maximum value
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static void ClampHeightmap(
		UPARAM(ref) TArray<float>& HeightData,
		float MinValue,
		float MaxValue
	);

	// ============================================================================
	// TRANSFORMATIONS
	// ============================================================================

	/**
	 * Flip heightmap horizontally
	 * @param HeightData - Height data to flip (modified in place)
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static void FlipHorizontal(
		UPARAM(ref) TArray<float>& HeightData,
		int32 Width,
		int32 Height
	);

	/**
	 * Flip heightmap vertically
	 * @param HeightData - Height data to flip (modified in place)
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static void FlipVertical(
		UPARAM(ref) TArray<float>& HeightData,
		int32 Width,
		int32 Height
	);

	/**
	 * Rotate heightmap
	 * @param HeightData - Height data to rotate
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param Angle - Rotation angle
	 * @param OutRotatedData - Output rotated data
	 * @param OutNewWidth - Output new width
	 * @param OutNewHeight - Output new height
	 * @return True if rotation succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static bool RotateHeightmap(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		ERotationAngle Angle,
		TArray<float>& OutRotatedData,
		int32& OutNewWidth,
		int32& OutNewHeight
	);

	// ============================================================================
	// TILING
	// ============================================================================

	/**
	 * Make heightmap tileable (seamless)
	 * @param HeightData - Height data to make tileable (modified in place)
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param BlendWidth - Width of blend region
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static void MakeTileable(
		UPARAM(ref) TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		int32 BlendWidth = 32
	);

	/**
	 * Extract region from heightmap
	 * @param HeightData - Source height data
	 * @param SourceWidth - Source width
	 * @param SourceHeight - Source height
	 * @param StartX - Start X coordinate
	 * @param StartY - Start Y coordinate
	 * @param RegionWidth - Region width
	 * @param RegionHeight - Region height
	 * @param OutRegionData - Output region data
	 * @return True if extraction succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static bool ExtractRegion(
		const TArray<float>& HeightData,
		int32 SourceWidth,
		int32 SourceHeight,
		int32 StartX,
		int32 StartY,
		int32 RegionWidth,
		int32 RegionHeight,
		TArray<float>& OutRegionData
	);

	/**
	 * Merge multiple heightmap tiles
	 * @param TileData - Array of tile data arrays
	 * @param TileWidth - Width of each tile
	 * @param TileHeight - Height of each tile
	 * @param TilesX - Number of tiles in X
	 * @param TilesY - Number of tiles in Y
	 * @param OutMergedData - Output merged data
	 * @return True if merge succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static bool MergeTiles(
		const TArray<TArray<float>>& TileData,
		int32 TileWidth,
		int32 TileHeight,
		int32 TilesX,
		int32 TilesY,
		TArray<float>& OutMergedData
	);

	// ============================================================================
	// UTILITY
	// ============================================================================

	/**
	 * Apply full conversion with parameters
	 * @param HeightData - Input height data
	 * @param SourceWidth - Source width
	 * @param SourceHeight - Source height
	 * @param Params - Conversion parameters
	 * @param OutConvertedData - Output converted data
	 * @return True if conversion succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Converter")
	static bool ConvertHeightmap(
		const TArray<float>& HeightData,
		int32 SourceWidth,
		int32 SourceHeight,
		const FHeightmapConversionParams& Params,
		TArray<float>& OutConvertedData
	);

private:
	// Interpolation helpers
	static float SampleBilinear(const TArray<float>& Data, int32 Width, int32 Height, float X, float Y);
	static float SampleBicubic(const TArray<float>& Data, int32 Width, int32 Height, float X, float Y);
	static float CubicInterpolate(float p0, float p1, float p2, float p3, float t);
	static float LanczosKernel(float x, int32 a = 3);
	static float SampleLanczos(const TArray<float>& Data, int32 Width, int32 Height, float X, float Y);

	// Utility helpers
	static float GetHeightSafe(const TArray<float>& Data, int32 Width, int32 Height, int32 X, int32 Y);
	static void FindMinMax(const TArray<float>& Data, float& OutMin, float& OutMax);
};
