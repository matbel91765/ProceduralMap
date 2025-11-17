// Ultimate Procedural Map Generation System - Heightmap Analyzer
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeightmapAnalyzer.generated.h"

/**
 * Heightmap statistics structure
 */
USTRUCT(BlueprintType)
struct FHeightmapStatistics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float MinHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float MaxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float AverageHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float MedianHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float StandardDeviation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float Range = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalSamples = 0;
};

/**
 * Slope statistics structure
 */
USTRUCT(BlueprintType)
struct FSlopeStatistics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	float MinSlope = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	float MaxSlope = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	float AverageSlope = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	float MedianSlope = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	int32 FlatRegions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	int32 SteepRegions = 0;
};

/**
 * Histogram data structure
 */
USTRUCT(BlueprintType)
struct FHeightmapHistogram
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Histogram")
	TArray<int32> Bins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Histogram")
	float BinWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Histogram")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Histogram")
	float MaxValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Histogram")
	int32 NumBins = 0;
};

/**
 * Quality metrics structure
 */
USTRUCT(BlueprintType)
struct FHeightmapQualityMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	float Smoothness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	float Roughness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	float Contrast = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	float NoiseLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	bool bIsValidForUnreal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	FString ValidationMessage;
};

/**
 * Heightmap Analyzer - Analyzes heightmap data
 * Provides statistics, slope analysis, histograms, and quality metrics
 */
UCLASS()
class PROCEDURALMAP_API UHeightmapAnalyzer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// STATISTICS
	// ============================================================================

	/**
	 * Calculate comprehensive statistics for heightmap
	 * @param HeightData - Height data to analyze
	 * @param OutStatistics - Output statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static void CalculateStatistics(
		const TArray<float>& HeightData,
		FHeightmapStatistics& OutStatistics
	);

	/**
	 * Get minimum height value
	 * @param HeightData - Height data to analyze
	 * @return Minimum height
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Analyzer")
	static float GetMinHeight(const TArray<float>& HeightData);

	/**
	 * Get maximum height value
	 * @param HeightData - Height data to analyze
	 * @return Maximum height
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Analyzer")
	static float GetMaxHeight(const TArray<float>& HeightData);

	/**
	 * Get average height value
	 * @param HeightData - Height data to analyze
	 * @return Average height
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Analyzer")
	static float GetAverageHeight(const TArray<float>& HeightData);

	/**
	 * Get median height value
	 * @param HeightData - Height data to analyze
	 * @return Median height
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static float GetMedianHeight(const TArray<float>& HeightData);

	/**
	 * Get standard deviation of height values
	 * @param HeightData - Height data to analyze
	 * @param Mean - Mean value (optional, will be calculated if <= 0)
	 * @return Standard deviation
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static float GetStandardDeviation(const TArray<float>& HeightData, float Mean = 0.0f);

	// ============================================================================
	// SLOPE ANALYSIS
	// ============================================================================

	/**
	 * Calculate slope statistics
	 * @param HeightData - Height data to analyze
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param HorizontalScale - Horizontal scale (world units per pixel)
	 * @param VerticalScale - Vertical scale (world units)
	 * @param OutSlope - Output slope statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static void CalculateSlopeStatistics(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		float HorizontalScale,
		float VerticalScale,
		FSlopeStatistics& OutSlope
	);

	/**
	 * Generate slope map from heightmap
	 * @param HeightData - Height data to analyze
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param HorizontalScale - Horizontal scale
	 * @param OutSlopeMap - Output slope map (in degrees)
	 * @return True if generation succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static bool GenerateSlopeMap(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		float HorizontalScale,
		TArray<float>& OutSlopeMap
	);

	/**
	 * Calculate slope at specific position
	 * @param HeightData - Height data
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param X - X coordinate
	 * @param Y - Y coordinate
	 * @param HorizontalScale - Horizontal scale
	 * @return Slope in degrees
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Analyzer")
	static float GetSlopeAtPosition(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		int32 X,
		int32 Y,
		float HorizontalScale = 1.0f
	);

	// ============================================================================
	// HISTOGRAM
	// ============================================================================

	/**
	 * Generate histogram of height values
	 * @param HeightData - Height data to analyze
	 * @param NumBins - Number of histogram bins
	 * @param OutHistogram - Output histogram data
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static void GenerateHistogram(
		const TArray<float>& HeightData,
		int32 NumBins,
		FHeightmapHistogram& OutHistogram
	);

	/**
	 * Get cumulative distribution function
	 * @param HeightData - Height data to analyze
	 * @param NumSamples - Number of CDF samples
	 * @param OutCDF - Output CDF values
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static void GetCumulativeDistribution(
		const TArray<float>& HeightData,
		int32 NumSamples,
		TArray<float>& OutCDF
	);

	// ============================================================================
	// QUALITY METRICS
	// ============================================================================

	/**
	 * Calculate quality metrics for heightmap
	 * @param HeightData - Height data to analyze
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param OutMetrics - Output quality metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static void CalculateQualityMetrics(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		FHeightmapQualityMetrics& OutMetrics
	);

	/**
	 * Calculate smoothness metric (0 = rough, 1 = smooth)
	 * @param HeightData - Height data to analyze
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @return Smoothness value
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static float CalculateSmoothness(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height
	);

	/**
	 * Calculate roughness metric (0 = smooth, 1 = rough)
	 * @param HeightData - Height data to analyze
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @return Roughness value
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static float CalculateRoughness(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height
	);

	// ============================================================================
	// VALIDATION
	// ============================================================================

	/**
	 * Validate heightmap for Unreal Engine requirements
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param OutMessage - Output validation message
	 * @return True if valid for Unreal
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Analyzer")
	static bool ValidateForUnreal(
		int32 Width,
		int32 Height,
		FString& OutMessage
	);

	/**
	 * Check if resolution is power of two plus one (Unreal requirement)
	 * @param Value - Value to check
	 * @return True if valid
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Analyzer")
	static bool IsPowerOfTwoPlusOne(int32 Value);

	/**
	 * Get nearest valid Unreal resolution
	 * @param Value - Input value
	 * @return Nearest valid resolution
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Analyzer")
	static int32 GetNearestValidResolution(int32 Value);

	/**
	 * Get list of recommended Unreal landscape sizes
	 * @return Array of recommended sizes (e.g., 1009, 2017, 4033, 8129)
	 */
	UFUNCTION(BlueprintPure, Category = "Heightmap|Analyzer")
	static TArray<int32> GetRecommendedLandscapeSizes();

	// ============================================================================
	// ADVANCED ANALYSIS
	// ============================================================================

	/**
	 * Detect features in heightmap (peaks, valleys, plateaus)
	 * @param HeightData - Height data to analyze
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param OutPeakPositions - Output peak positions
	 * @param OutValleyPositions - Output valley positions
	 * @param Threshold - Feature detection threshold
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static void DetectFeatures(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		TArray<FVector2D>& OutPeakPositions,
		TArray<FVector2D>& OutValleyPositions,
		float Threshold = 0.1f
	);

	/**
	 * Calculate fractal dimension (measure of terrain complexity)
	 * @param HeightData - Height data to analyze
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @return Fractal dimension (typically 2.0-2.5 for natural terrain)
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Analyzer")
	static float CalculateFractalDimension(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height
	);

private:
	// Helper functions
	static float GetHeightSafe(const TArray<float>& Data, int32 Width, int32 Height, int32 X, int32 Y);
	static void CalculateGradient(const TArray<float>& Data, int32 Width, int32 Height, int32 X, int32 Y, float& OutDx, float& OutDy);
	static bool IsPowerOfTwo(int32 Value);
	static int32 GetNearestPowerOfTwo(int32 Value);
};
