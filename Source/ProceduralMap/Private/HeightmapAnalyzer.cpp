// Ultimate Procedural Map Generation System - Heightmap Analyzer
// Copyright (C) 2025. All Rights Reserved.

#include "HeightmapAnalyzer.h"
#include "Algo/Sort.h"

// ============================================================================
// STATISTICS
// ============================================================================

void UHeightmapAnalyzer::CalculateStatistics(
	const TArray<float>& HeightData,
	FHeightmapStatistics& OutStatistics)
{
	if (HeightData.Num() == 0)
	{
		OutStatistics = FHeightmapStatistics();
		return;
	}

	OutStatistics.TotalSamples = HeightData.Num();
	OutStatistics.MinHeight = GetMinHeight(HeightData);
	OutStatistics.MaxHeight = GetMaxHeight(HeightData);
	OutStatistics.AverageHeight = GetAverageHeight(HeightData);
	OutStatistics.MedianHeight = GetMedianHeight(HeightData);
	OutStatistics.StandardDeviation = GetStandardDeviation(HeightData, OutStatistics.AverageHeight);
	OutStatistics.Range = OutStatistics.MaxHeight - OutStatistics.MinHeight;

	UE_LOG(LogTemp, Log, TEXT("HeightmapAnalyzer: Statistics calculated for %d samples"), OutStatistics.TotalSamples);
	UE_LOG(LogTemp, Log, TEXT("  Min: %.2f, Max: %.2f, Range: %.2f"),
		OutStatistics.MinHeight, OutStatistics.MaxHeight, OutStatistics.Range);
	UE_LOG(LogTemp, Log, TEXT("  Average: %.2f, Median: %.2f, StdDev: %.2f"),
		OutStatistics.AverageHeight, OutStatistics.MedianHeight, OutStatistics.StandardDeviation);
}

float UHeightmapAnalyzer::GetMinHeight(const TArray<float>& HeightData)
{
	if (HeightData.Num() == 0)
	{
		return 0.0f;
	}

	float MinValue = TNumericLimits<float>::Max();
	for (float Value : HeightData)
	{
		MinValue = FMath::Min(MinValue, Value);
	}

	return MinValue;
}

float UHeightmapAnalyzer::GetMaxHeight(const TArray<float>& HeightData)
{
	if (HeightData.Num() == 0)
	{
		return 0.0f;
	}

	float MaxValue = TNumericLimits<float>::Lowest();
	for (float Value : HeightData)
	{
		MaxValue = FMath::Max(MaxValue, Value);
	}

	return MaxValue;
}

float UHeightmapAnalyzer::GetAverageHeight(const TArray<float>& HeightData)
{
	if (HeightData.Num() == 0)
	{
		return 0.0f;
	}

	double Sum = 0.0;
	for (float Value : HeightData)
	{
		Sum += Value;
	}

	return static_cast<float>(Sum / HeightData.Num());
}

float UHeightmapAnalyzer::GetMedianHeight(const TArray<float>& HeightData)
{
	if (HeightData.Num() == 0)
	{
		return 0.0f;
	}

	TArray<float> SortedData = HeightData;
	SortedData.Sort();

	int32 MiddleIndex = SortedData.Num() / 2;

	if (SortedData.Num() % 2 == 0)
	{
		return (SortedData[MiddleIndex - 1] + SortedData[MiddleIndex]) / 2.0f;
	}
	else
	{
		return SortedData[MiddleIndex];
	}
}

float UHeightmapAnalyzer::GetStandardDeviation(const TArray<float>& HeightData, float Mean)
{
	if (HeightData.Num() <= 1)
	{
		return 0.0f;
	}

	if (Mean <= 0.0f)
	{
		Mean = GetAverageHeight(HeightData);
	}

	double Variance = 0.0;
	for (float Value : HeightData)
	{
		float Diff = Value - Mean;
		Variance += Diff * Diff;
	}

	Variance /= (HeightData.Num() - 1);

	return FMath::Sqrt(static_cast<float>(Variance));
}

// ============================================================================
// SLOPE ANALYSIS
// ============================================================================

void UHeightmapAnalyzer::CalculateSlopeStatistics(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	float HorizontalScale,
	float VerticalScale,
	FSlopeStatistics& OutSlope)
{
	if (HeightData.Num() != Width * Height || Width < 2 || Height < 2)
	{
		OutSlope = FSlopeStatistics();
		return;
	}

	TArray<float> SlopeMap;
	GenerateSlopeMap(HeightData, Width, Height, HorizontalScale, SlopeMap);

	OutSlope.MinSlope = GetMinHeight(SlopeMap);
	OutSlope.MaxSlope = GetMaxHeight(SlopeMap);
	OutSlope.AverageSlope = GetAverageHeight(SlopeMap);
	OutSlope.MedianSlope = GetMedianHeight(SlopeMap);

	// Count flat and steep regions
	OutSlope.FlatRegions = 0;
	OutSlope.SteepRegions = 0;

	const float FlatThreshold = 5.0f;   // Degrees
	const float SteepThreshold = 45.0f; // Degrees

	for (float Slope : SlopeMap)
	{
		if (Slope < FlatThreshold)
		{
			OutSlope.FlatRegions++;
		}
		else if (Slope > SteepThreshold)
		{
			OutSlope.SteepRegions++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HeightmapAnalyzer: Slope statistics calculated"));
	UE_LOG(LogTemp, Log, TEXT("  Min: %.2f°, Max: %.2f°, Average: %.2f°"),
		OutSlope.MinSlope, OutSlope.MaxSlope, OutSlope.AverageSlope);
	UE_LOG(LogTemp, Log, TEXT("  Flat regions: %d, Steep regions: %d"),
		OutSlope.FlatRegions, OutSlope.SteepRegions);
}

bool UHeightmapAnalyzer::GenerateSlopeMap(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	float HorizontalScale,
	TArray<float>& OutSlopeMap)
{
	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapAnalyzer: Data size mismatch"));
		return false;
	}

	OutSlopeMap.SetNum(Width * Height);

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			float Slope = GetSlopeAtPosition(HeightData, Width, Height, X, Y, HorizontalScale);
			OutSlopeMap[Y * Width + X] = Slope;
		}
	}

	return true;
}

float UHeightmapAnalyzer::GetSlopeAtPosition(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	int32 X,
	int32 Y,
	float HorizontalScale)
{
	float Dx, Dy;
	CalculateGradient(HeightData, Width, Height, X, Y, Dx, Dy);

	// Apply horizontal scale
	Dx /= HorizontalScale;
	Dy /= HorizontalScale;

	// Calculate slope magnitude
	float SlopeMagnitude = FMath::Sqrt(Dx * Dx + Dy * Dy);

	// Convert to degrees
	return FMath::RadiansToDegrees(FMath::Atan(SlopeMagnitude));
}

// ============================================================================
// HISTOGRAM
// ============================================================================

void UHeightmapAnalyzer::GenerateHistogram(
	const TArray<float>& HeightData,
	int32 NumBins,
	FHeightmapHistogram& OutHistogram)
{
	if (HeightData.Num() == 0 || NumBins <= 0)
	{
		OutHistogram = FHeightmapHistogram();
		return;
	}

	OutHistogram.NumBins = NumBins;
	OutHistogram.MinValue = GetMinHeight(HeightData);
	OutHistogram.MaxValue = GetMaxHeight(HeightData);
	OutHistogram.BinWidth = (OutHistogram.MaxValue - OutHistogram.MinValue) / NumBins;

	OutHistogram.Bins.SetNum(NumBins);
	for (int32& Bin : OutHistogram.Bins)
	{
		Bin = 0;
	}

	// Fill histogram
	for (float Value : HeightData)
	{
		int32 BinIndex = FMath::FloorToInt((Value - OutHistogram.MinValue) / OutHistogram.BinWidth);
		BinIndex = FMath::Clamp(BinIndex, 0, NumBins - 1);
		OutHistogram.Bins[BinIndex]++;
	}

	UE_LOG(LogTemp, Log, TEXT("HeightmapAnalyzer: Histogram generated with %d bins"), NumBins);
}

void UHeightmapAnalyzer::GetCumulativeDistribution(
	const TArray<float>& HeightData,
	int32 NumSamples,
	TArray<float>& OutCDF)
{
	if (HeightData.Num() == 0 || NumSamples <= 0)
	{
		OutCDF.Empty();
		return;
	}

	// Sort data
	TArray<float> SortedData = HeightData;
	SortedData.Sort();

	OutCDF.SetNum(NumSamples);

	for (int32 i = 0; i < NumSamples; ++i)
	{
		float Percentile = static_cast<float>(i) / (NumSamples - 1);
		int32 Index = FMath::FloorToInt(Percentile * (SortedData.Num() - 1));
		Index = FMath::Clamp(Index, 0, SortedData.Num() - 1);
		OutCDF[i] = SortedData[Index];
	}
}

// ============================================================================
// QUALITY METRICS
// ============================================================================

void UHeightmapAnalyzer::CalculateQualityMetrics(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	FHeightmapQualityMetrics& OutMetrics)
{
	if (HeightData.Num() != Width * Height)
	{
		OutMetrics = FHeightmapQualityMetrics();
		OutMetrics.bIsValidForUnreal = false;
		OutMetrics.ValidationMessage = TEXT("Invalid data size");
		return;
	}

	OutMetrics.Smoothness = CalculateSmoothness(HeightData, Width, Height);
	OutMetrics.Roughness = CalculateRoughness(HeightData, Width, Height);

	// Calculate contrast
	float MinHeight = GetMinHeight(HeightData);
	float MaxHeight = GetMaxHeight(HeightData);
	OutMetrics.Contrast = (MaxHeight - MinHeight) / FMath::Max(MaxHeight, 1.0f);

	// Calculate noise level (standard deviation normalized)
	float StdDev = GetStandardDeviation(HeightData, 0.0f);
	float Range = MaxHeight - MinHeight;
	OutMetrics.NoiseLevel = (Range > 0.0f) ? (StdDev / Range) : 0.0f;

	// Validate for Unreal
	OutMetrics.bIsValidForUnreal = ValidateForUnreal(Width, Height, OutMetrics.ValidationMessage);

	UE_LOG(LogTemp, Log, TEXT("HeightmapAnalyzer: Quality metrics calculated"));
	UE_LOG(LogTemp, Log, TEXT("  Smoothness: %.2f, Roughness: %.2f"),
		OutMetrics.Smoothness, OutMetrics.Roughness);
	UE_LOG(LogTemp, Log, TEXT("  Contrast: %.2f, Noise: %.2f"),
		OutMetrics.Contrast, OutMetrics.NoiseLevel);
	UE_LOG(LogTemp, Log, TEXT("  Valid for Unreal: %s"),
		OutMetrics.bIsValidForUnreal ? TEXT("Yes") : TEXT("No"));
}

float UHeightmapAnalyzer::CalculateSmoothness(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height)
{
	if (HeightData.Num() != Width * Height || Width < 2 || Height < 2)
	{
		return 0.0f;
	}

	double TotalVariation = 0.0;
	int32 SampleCount = 0;

	// Calculate average gradient magnitude
	for (int32 Y = 1; Y < Height - 1; ++Y)
	{
		for (int32 X = 1; X < Width - 1; ++X)
		{
			float Dx, Dy;
			CalculateGradient(HeightData, Width, Height, X, Y, Dx, Dy);

			float GradientMagnitude = FMath::Sqrt(Dx * Dx + Dy * Dy);
			TotalVariation += GradientMagnitude;
			SampleCount++;
		}
	}

	float AverageVariation = (SampleCount > 0) ? static_cast<float>(TotalVariation / SampleCount) : 0.0f;

	// Normalize to 0-1 range (0 = rough, 1 = smooth)
	// Lower variation = smoother
	float Smoothness = 1.0f - FMath::Clamp(AverageVariation, 0.0f, 1.0f);

	return Smoothness;
}

float UHeightmapAnalyzer::CalculateRoughness(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height)
{
	// Roughness is inverse of smoothness
	return 1.0f - CalculateSmoothness(HeightData, Width, Height);
}

// ============================================================================
// VALIDATION
// ============================================================================

bool UHeightmapAnalyzer::ValidateForUnreal(
	int32 Width,
	int32 Height,
	FString& OutMessage)
{
	// Unreal Landscape requires specific resolutions
	// Common valid sizes: 127, 255, 511, 1009, 2017, 4033, 8129
	// Formula: (2^n + 1) or combinations for components

	if (!IsPowerOfTwoPlusOne(Width))
	{
		OutMessage = FString::Printf(TEXT("Width %d is not valid. Must be (2^n + 1). Nearest valid: %d"),
			Width, GetNearestValidResolution(Width));
		return false;
	}

	if (!IsPowerOfTwoPlusOne(Height))
	{
		OutMessage = FString::Printf(TEXT("Height %d is not valid. Must be (2^n + 1). Nearest valid: %d"),
			Height, GetNearestValidResolution(Height));
		return false;
	}

	OutMessage = FString::Printf(TEXT("Resolution %dx%d is valid for Unreal Landscape"), Width, Height);
	return true;
}

bool UHeightmapAnalyzer::IsPowerOfTwoPlusOne(int32 Value)
{
	// Check if value is (2^n + 1)
	int32 Test = Value - 1;
	return (Test > 0) && ((Test & (Test - 1)) == 0);
}

int32 UHeightmapAnalyzer::GetNearestValidResolution(int32 Value)
{
	TArray<int32> ValidSizes = GetRecommendedLandscapeSizes();

	int32 NearestSize = ValidSizes[0];
	int32 MinDiff = FMath::Abs(Value - NearestSize);

	for (int32 Size : ValidSizes)
	{
		int32 Diff = FMath::Abs(Value - Size);
		if (Diff < MinDiff)
		{
			MinDiff = Diff;
			NearestSize = Size;
		}
	}

	return NearestSize;
}

TArray<int32> UHeightmapAnalyzer::GetRecommendedLandscapeSizes()
{
	// Common Unreal Landscape sizes
	return {
		127,   // 2^7 - 1 (very small)
		255,   // 2^8 - 1 (small)
		511,   // 2^9 - 1 (medium)
		1009,  // (2^5 - 1) * (2^5) + 1 (large)
		2017,  // (2^6 - 1) * (2^5) + 1 (very large)
		4033,  // (2^7 - 1) * (2^6) + 1 (huge)
		8129   // (2^7 - 1) * (2^7) + 1 (massive)
	};
}

// ============================================================================
// ADVANCED ANALYSIS
// ============================================================================

void UHeightmapAnalyzer::DetectFeatures(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	TArray<FVector2D>& OutPeakPositions,
	TArray<FVector2D>& OutValleyPositions,
	float Threshold)
{
	if (HeightData.Num() != Width * Height || Width < 3 || Height < 3)
	{
		OutPeakPositions.Empty();
		OutValleyPositions.Empty();
		return;
	}

	OutPeakPositions.Empty();
	OutValleyPositions.Empty();

	float MinHeight = GetMinHeight(HeightData);
	float MaxHeight = GetMaxHeight(HeightData);
	float Range = MaxHeight - MinHeight;
	float AbsoluteThreshold = Range * Threshold;

	// Detect local maxima (peaks) and minima (valleys)
	for (int32 Y = 1; Y < Height - 1; ++Y)
	{
		for (int32 X = 1; X < Width - 1; ++X)
		{
			float Center = GetHeightSafe(HeightData, Width, Height, X, Y);

			bool IsPeak = true;
			bool IsValley = true;

			// Check 8 neighbors
			for (int32 dy = -1; dy <= 1; ++dy)
			{
				for (int32 dx = -1; dx <= 1; ++dx)
				{
					if (dx == 0 && dy == 0) continue;

					float Neighbor = GetHeightSafe(HeightData, Width, Height, X + dx, Y + dy);

					if (Neighbor >= Center)
					{
						IsPeak = false;
					}

					if (Neighbor <= Center)
					{
						IsValley = false;
					}
				}
			}

			// Check if feature is significant enough
			if (IsPeak && (Center - MinHeight) > AbsoluteThreshold)
			{
				OutPeakPositions.Add(FVector2D(X, Y));
			}
			else if (IsValley && (MaxHeight - Center) > AbsoluteThreshold)
			{
				OutValleyPositions.Add(FVector2D(X, Y));
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HeightmapAnalyzer: Detected %d peaks and %d valleys"),
		OutPeakPositions.Num(), OutValleyPositions.Num());
}

float UHeightmapAnalyzer::CalculateFractalDimension(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height)
{
	if (HeightData.Num() != Width * Height || Width < 4 || Height < 4)
	{
		return 2.0f;
	}

	// Use box-counting method
	TArray<int32> BoxSizes = { 2, 4, 8, 16, 32, 64 };
	TArray<float> LogBoxSizes;
	TArray<float> LogCounts;

	for (int32 BoxSize : BoxSizes)
	{
		if (BoxSize >= Width || BoxSize >= Height)
		{
			break;
		}

		int32 NumBoxesX = Width / BoxSize;
		int32 NumBoxesY = Height / BoxSize;
		int32 BoxCount = 0;

		for (int32 By = 0; By < NumBoxesY; ++By)
		{
			for (int32 Bx = 0; Bx < NumBoxesX; ++Bx)
			{
				float MinBoxHeight = TNumericLimits<float>::Max();
				float MaxBoxHeight = TNumericLimits<float>::Lowest();

				for (int32 y = 0; y < BoxSize; ++y)
				{
					for (int32 x = 0; x < BoxSize; ++x)
					{
						int32 GlobalX = Bx * BoxSize + x;
						int32 GlobalY = By * BoxSize + y;

						float Height_f = GetHeightSafe(HeightData, Width, Height, GlobalX, GlobalY);
						MinBoxHeight = FMath::Min(MinBoxHeight, Height_f);
						MaxBoxHeight = FMath::Max(MaxBoxHeight, Height_f);
					}
				}

				float HeightRange = MaxBoxHeight - MinBoxHeight;
				if (HeightRange > SMALL_NUMBER)
				{
					int32 NumBoxesInHeight = FMath::CeilToInt(HeightRange / BoxSize);
					BoxCount += NumBoxesInHeight;
				}
			}
		}

		if (BoxCount > 0)
		{
			LogBoxSizes.Add(FMath::Loge(1.0f / BoxSize));
			LogCounts.Add(FMath::Loge(static_cast<float>(BoxCount)));
		}
	}

	// Calculate slope using linear regression
	if (LogBoxSizes.Num() < 2)
	{
		return 2.0f;
	}

	float SumX = 0.0f, SumY = 0.0f, SumXY = 0.0f, SumXX = 0.0f;
	int32 N = LogBoxSizes.Num();

	for (int32 i = 0; i < N; ++i)
	{
		SumX += LogBoxSizes[i];
		SumY += LogCounts[i];
		SumXY += LogBoxSizes[i] * LogCounts[i];
		SumXX += LogBoxSizes[i] * LogBoxSizes[i];
	}

	float Slope = (N * SumXY - SumX * SumY) / (N * SumXX - SumX * SumX);

	// Fractal dimension is the slope
	float FractalDimension = FMath::Clamp(Slope, 2.0f, 3.0f);

	UE_LOG(LogTemp, Log, TEXT("HeightmapAnalyzer: Fractal dimension = %.3f"), FractalDimension);

	return FractalDimension;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

float UHeightmapAnalyzer::GetHeightSafe(const TArray<float>& Data, int32 Width, int32 Height, int32 X, int32 Y)
{
	X = FMath::Clamp(X, 0, Width - 1);
	Y = FMath::Clamp(Y, 0, Height - 1);
	return Data[Y * Width + X];
}

void UHeightmapAnalyzer::CalculateGradient(const TArray<float>& Data, int32 Width, int32 Height, int32 X, int32 Y, float& OutDx, float& OutDy)
{
	// Sobel operator
	float TopLeft = GetHeightSafe(Data, Width, Height, X - 1, Y - 1);
	float Top = GetHeightSafe(Data, Width, Height, X, Y - 1);
	float TopRight = GetHeightSafe(Data, Width, Height, X + 1, Y - 1);

	float Left = GetHeightSafe(Data, Width, Height, X - 1, Y);
	float Right = GetHeightSafe(Data, Width, Height, X + 1, Y);

	float BottomLeft = GetHeightSafe(Data, Width, Height, X - 1, Y + 1);
	float Bottom = GetHeightSafe(Data, Width, Height, X, Y + 1);
	float BottomRight = GetHeightSafe(Data, Width, Height, X + 1, Y + 1);

	OutDx = (TopRight + 2.0f * Right + BottomRight - TopLeft - 2.0f * Left - BottomLeft) / 8.0f;
	OutDy = (BottomLeft + 2.0f * Bottom + BottomRight - TopLeft - 2.0f * Top - TopRight) / 8.0f;
}

bool UHeightmapAnalyzer::IsPowerOfTwo(int32 Value)
{
	return (Value > 0) && ((Value & (Value - 1)) == 0);
}

int32 UHeightmapAnalyzer::GetNearestPowerOfTwo(int32 Value)
{
	int32 Power = 1;
	while (Power < Value)
	{
		Power *= 2;
	}

	int32 LowerPower = Power / 2;

	return (Value - LowerPower < Power - Value) ? LowerPower : Power;
}
