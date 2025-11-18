// Ultimate Procedural Map Generation System - Heightmap Converter
// Copyright (C) 2025. All Rights Reserved.

#include "HeightmapConverter.h"
#include "Misc/FileHelper.h"

// ============================================================================
// RESIZING AND RESAMPLING
// ============================================================================

bool UHeightmapConverter::ResizeHeightmap(
	const TArray<float>& HeightData,
	int32 SourceWidth,
	int32 SourceHeight,
	int32 TargetWidth,
	int32 TargetHeight,
	EResamplingFilter Filter,
	TArray<float>& OutResizedData)
{
	if (HeightData.Num() != SourceWidth * SourceHeight)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapConverter: Source data size mismatch"));
		return false;
	}

	if (TargetWidth <= 0 || TargetHeight <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapConverter: Invalid target dimensions"));
		return false;
	}

	switch (Filter)
	{
	case EResamplingFilter::Nearest:
		return ResizeNearestNeighbor(HeightData, SourceWidth, SourceHeight, TargetWidth, TargetHeight, OutResizedData);

	case EResamplingFilter::Bilinear:
		return ResizeBilinear(HeightData, SourceWidth, SourceHeight, TargetWidth, TargetHeight, OutResizedData);

	case EResamplingFilter::Bicubic:
		return ResizeBicubic(HeightData, SourceWidth, SourceHeight, TargetWidth, TargetHeight, OutResizedData);

	case EResamplingFilter::Lanczos:
	{
		OutResizedData.SetNum(TargetWidth * TargetHeight);

		float ScaleX = static_cast<float>(SourceWidth) / TargetWidth;
		float ScaleY = static_cast<float>(SourceHeight) / TargetHeight;

		for (int32 Y = 0; Y < TargetHeight; ++Y)
		{
			for (int32 X = 0; X < TargetWidth; ++X)
			{
				float SourceX = (X + 0.5f) * ScaleX - 0.5f;
				float SourceY = (Y + 0.5f) * ScaleY - 0.5f;

				float Value = SampleLanczos(HeightData, SourceWidth, SourceHeight, SourceX, SourceY);
				OutResizedData[Y * TargetWidth + X] = Value;
			}
		}

		return true;
	}

	default:
		UE_LOG(LogTemp, Error, TEXT("HeightmapConverter: Unknown filter type"));
		return false;
	}
}

bool UHeightmapConverter::ResizeNearestNeighbor(
	const TArray<float>& HeightData,
	int32 SourceWidth,
	int32 SourceHeight,
	int32 TargetWidth,
	int32 TargetHeight,
	TArray<float>& OutResizedData)
{
	OutResizedData.SetNum(TargetWidth * TargetHeight);

	float ScaleX = static_cast<float>(SourceWidth) / TargetWidth;
	float ScaleY = static_cast<float>(SourceHeight) / TargetHeight;

	for (int32 Y = 0; Y < TargetHeight; ++Y)
	{
		for (int32 X = 0; X < TargetWidth; ++X)
		{
			int32 SourceX = FMath::FloorToInt(X * ScaleX);
			int32 SourceY = FMath::FloorToInt(Y * ScaleY);

			SourceX = FMath::Clamp(SourceX, 0, SourceWidth - 1);
			SourceY = FMath::Clamp(SourceY, 0, SourceHeight - 1);

			OutResizedData[Y * TargetWidth + X] = HeightData[SourceY * SourceWidth + SourceX];
		}
	}

	return true;
}

bool UHeightmapConverter::ResizeBilinear(
	const TArray<float>& HeightData,
	int32 SourceWidth,
	int32 SourceHeight,
	int32 TargetWidth,
	int32 TargetHeight,
	TArray<float>& OutResizedData)
{
	OutResizedData.SetNum(TargetWidth * TargetHeight);

	float ScaleX = static_cast<float>(SourceWidth) / TargetWidth;
	float ScaleY = static_cast<float>(SourceHeight) / TargetHeight;

	for (int32 Y = 0; Y < TargetHeight; ++Y)
	{
		for (int32 X = 0; X < TargetWidth; ++X)
		{
			float SourceX = (X + 0.5f) * ScaleX - 0.5f;
			float SourceY = (Y + 0.5f) * ScaleY - 0.5f;

			float Value = SampleBilinear(HeightData, SourceWidth, SourceHeight, SourceX, SourceY);
			OutResizedData[Y * TargetWidth + X] = Value;
		}
	}

	return true;
}

bool UHeightmapConverter::ResizeBicubic(
	const TArray<float>& HeightData,
	int32 SourceWidth,
	int32 SourceHeight,
	int32 TargetWidth,
	int32 TargetHeight,
	TArray<float>& OutResizedData)
{
	OutResizedData.SetNum(TargetWidth * TargetHeight);

	float ScaleX = static_cast<float>(SourceWidth) / TargetWidth;
	float ScaleY = static_cast<float>(SourceHeight) / TargetHeight;

	for (int32 Y = 0; Y < TargetHeight; ++Y)
	{
		for (int32 X = 0; X < TargetWidth; ++X)
		{
			float SourceX = (X + 0.5f) * ScaleX - 0.5f;
			float SourceY = (Y + 0.5f) * ScaleY - 0.5f;

			float Value = SampleBicubic(HeightData, SourceWidth, SourceHeight, SourceX, SourceY);
			OutResizedData[Y * TargetWidth + X] = Value;
		}
	}

	return true;
}

// ============================================================================
// NORMALIZATION AND CLAMPING
// ============================================================================

void UHeightmapConverter::NormalizeHeightmap(
	TArray<float>& HeightData,
	float& OutMin,
	float& OutMax)
{
	if (HeightData.Num() == 0)
	{
		OutMin = 0.0f;
		OutMax = 0.0f;
		return;
	}

	FindMinMax(HeightData, OutMin, OutMax);

	float Range = OutMax - OutMin;
	if (Range > 0.0f)
	{
		for (float& Value : HeightData)
		{
			Value = (Value - OutMin) / Range;
		}
	}
}

void UHeightmapConverter::NormalizeToRange(
	TArray<float>& HeightData,
	float TargetMin,
	float TargetMax)
{
	if (HeightData.Num() == 0)
	{
		return;
	}

	float CurrentMin, CurrentMax;
	FindMinMax(HeightData, CurrentMin, CurrentMax);

	float CurrentRange = CurrentMax - CurrentMin;
	float TargetRange = TargetMax - TargetMin;

	if (CurrentRange > 0.0f)
	{
		for (float& Value : HeightData)
		{
			float Normalized = (Value - CurrentMin) / CurrentRange;
			Value = TargetMin + (Normalized * TargetRange);
		}
	}
}

void UHeightmapConverter::ClampHeightmap(
	TArray<float>& HeightData,
	float MinValue,
	float MaxValue)
{
	for (float& Value : HeightData)
	{
		Value = FMath::Clamp(Value, MinValue, MaxValue);
	}
}

// ============================================================================
// TRANSFORMATIONS
// ============================================================================

void UHeightmapConverter::FlipHorizontal(
	TArray<float>& HeightData,
	int32 Width,
	int32 Height)
{
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width / 2; ++X)
		{
			int32 LeftIndex = Y * Width + X;
			int32 RightIndex = Y * Width + (Width - 1 - X);

			float Temp = HeightData[LeftIndex];
			HeightData[LeftIndex] = HeightData[RightIndex];
			HeightData[RightIndex] = Temp;
		}
	}
}

void UHeightmapConverter::FlipVertical(
	TArray<float>& HeightData,
	int32 Width,
	int32 Height)
{
	for (int32 Y = 0; Y < Height / 2; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 TopIndex = Y * Width + X;
			int32 BottomIndex = (Height - 1 - Y) * Width + X;

			float Temp = HeightData[TopIndex];
			HeightData[TopIndex] = HeightData[BottomIndex];
			HeightData[BottomIndex] = Temp;
		}
	}
}

bool UHeightmapConverter::RotateHeightmap(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	ERotationAngle Angle,
	TArray<float>& OutRotatedData,
	int32& OutNewWidth,
	int32& OutNewHeight)
{
	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapConverter: Data size mismatch"));
		return false;
	}

	switch (Angle)
	{
	case ERotationAngle::Rotate90:
		OutNewWidth = Height;
		OutNewHeight = Width;
		OutRotatedData.SetNum(Width * Height);

		for (int32 Y = 0; Y < Height; ++Y)
		{
			for (int32 X = 0; X < Width; ++X)
			{
				int32 SrcIndex = Y * Width + X;
				int32 DstIndex = X * Height + (Height - 1 - Y);
				OutRotatedData[DstIndex] = HeightData[SrcIndex];
			}
		}
		break;

	case ERotationAngle::Rotate180:
		OutNewWidth = Width;
		OutNewHeight = Height;
		OutRotatedData.SetNum(Width * Height);

		for (int32 Y = 0; Y < Height; ++Y)
		{
			for (int32 X = 0; X < Width; ++X)
			{
				int32 SrcIndex = Y * Width + X;
				int32 DstIndex = (Height - 1 - Y) * Width + (Width - 1 - X);
				OutRotatedData[DstIndex] = HeightData[SrcIndex];
			}
		}
		break;

	case ERotationAngle::Rotate270:
		OutNewWidth = Height;
		OutNewHeight = Width;
		OutRotatedData.SetNum(Width * Height);

		for (int32 Y = 0; Y < Height; ++Y)
		{
			for (int32 X = 0; X < Width; ++X)
			{
				int32 SrcIndex = Y * Width + X;
				int32 DstIndex = (Width - 1 - X) * Height + Y;
				OutRotatedData[DstIndex] = HeightData[SrcIndex];
			}
		}
		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("HeightmapConverter: Unknown rotation angle"));
		return false;
	}

	return true;
}

// ============================================================================
// TILING
// ============================================================================

void UHeightmapConverter::MakeTileable(
	TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	int32 BlendWidth)
{
	if (BlendWidth <= 0 || BlendWidth >= Width / 2 || BlendWidth >= Height / 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("HeightmapConverter: Invalid blend width"));
		return;
	}

	TArray<float> OriginalData = HeightData;

	// Blend horizontal edges
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < BlendWidth; ++X)
		{
			float BlendFactor = static_cast<float>(X) / BlendWidth;

			int32 LeftIndex = Y * Width + X;
			int32 RightIndex = Y * Width + (Width - BlendWidth + X);
			int32 WrapIndex = Y * Width + (Width - 1 - X);

			float BlendedValue = FMath::Lerp(
				OriginalData[WrapIndex],
				OriginalData[LeftIndex],
				BlendFactor
			);

			HeightData[LeftIndex] = BlendedValue;

			BlendedValue = FMath::Lerp(
				OriginalData[RightIndex],
				OriginalData[X + Y * Width],
				BlendFactor
			);

			HeightData[RightIndex] = BlendedValue;
		}
	}

	// Blend vertical edges
	for (int32 Y = 0; Y < BlendWidth; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			float BlendFactor = static_cast<float>(Y) / BlendWidth;

			int32 TopIndex = Y * Width + X;
			int32 BottomIndex = (Height - BlendWidth + Y) * Width + X;
			int32 WrapIndex = (Height - 1 - Y) * Width + X;

			float BlendedValue = FMath::Lerp(
				OriginalData[WrapIndex],
				OriginalData[TopIndex],
				BlendFactor
			);

			HeightData[TopIndex] = BlendedValue;

			BlendedValue = FMath::Lerp(
				OriginalData[BottomIndex],
				OriginalData[X + Y * Width],
				BlendFactor
			);

			HeightData[BottomIndex] = BlendedValue;
		}
	}
}

bool UHeightmapConverter::ExtractRegion(
	const TArray<float>& HeightData,
	int32 SourceWidth,
	int32 SourceHeight,
	int32 StartX,
	int32 StartY,
	int32 RegionWidth,
	int32 RegionHeight,
	TArray<float>& OutRegionData)
{
	if (StartX < 0 || StartY < 0 ||
		StartX + RegionWidth > SourceWidth ||
		StartY + RegionHeight > SourceHeight)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapConverter: Region out of bounds"));
		return false;
	}

	OutRegionData.SetNum(RegionWidth * RegionHeight);

	for (int32 Y = 0; Y < RegionHeight; ++Y)
	{
		for (int32 X = 0; X < RegionWidth; ++X)
		{
			int32 SrcIndex = (StartY + Y) * SourceWidth + (StartX + X);
			int32 DstIndex = Y * RegionWidth + X;
			OutRegionData[DstIndex] = HeightData[SrcIndex];
		}
	}

	return true;
}

bool UHeightmapConverter::MergeTiles(
	const TArray<TArray<float>>& TileData,
	int32 TileWidth,
	int32 TileHeight,
	int32 TilesX,
	int32 TilesY,
	TArray<float>& OutMergedData)
{
	if (TileData.Num() != TilesX * TilesY)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapConverter: Tile count mismatch"));
		return false;
	}

	int32 TotalWidth = TilesX * TileWidth;
	int32 TotalHeight = TilesY * TileHeight;

	OutMergedData.SetNum(TotalWidth * TotalHeight);

	for (int32 TileY = 0; TileY < TilesY; ++TileY)
	{
		for (int32 TileX = 0; TileX < TilesX; ++TileX)
		{
			int32 TileIndex = TileY * TilesX + TileX;
			const TArray<float>& Tile = TileData[TileIndex];

			if (Tile.Num() != TileWidth * TileHeight)
			{
				UE_LOG(LogTemp, Error, TEXT("HeightmapConverter: Invalid tile size at [%d, %d]"), TileX, TileY);
				return false;
			}

			for (int32 Y = 0; Y < TileHeight; ++Y)
			{
				for (int32 X = 0; X < TileWidth; ++X)
				{
					int32 GlobalX = TileX * TileWidth + X;
					int32 GlobalY = TileY * TileHeight + Y;
					int32 GlobalIndex = GlobalY * TotalWidth + GlobalX;
					int32 LocalIndex = Y * TileWidth + X;

					OutMergedData[GlobalIndex] = Tile[LocalIndex];
				}
			}
		}
	}

	return true;
}

// ============================================================================
// UTILITY
// ============================================================================

bool UHeightmapConverter::ConvertHeightmap(
	const TArray<float>& HeightData,
	int32 SourceWidth,
	int32 SourceHeight,
	const FHeightmapConversionParams& Params,
	TArray<float>& OutConvertedData)
{
	// Resize
	if (!ResizeHeightmap(HeightData, SourceWidth, SourceHeight,
		Params.TargetWidth, Params.TargetHeight,
		Params.Filter, OutConvertedData))
	{
		return false;
	}

	// Normalize
	if (Params.bNormalize)
	{
		float Min, Max;
		NormalizeHeightmap(OutConvertedData, Min, Max);

		// Scale to target range
		float Range = Params.MaxValue - Params.MinValue;
		for (float& Value : OutConvertedData)
		{
			Value = Params.MinValue + (Value * Range);
		}
	}

	// Clamp
	if (Params.bClamp)
	{
		ClampHeightmap(OutConvertedData, Params.MinValue, Params.MaxValue);
	}

	return true;
}

// ============================================================================
// INTERPOLATION HELPERS
// ============================================================================

float UHeightmapConverter::SampleBilinear(const TArray<float>& Data, int32 Width, int32 Height, float X, float Y)
{
	int32 X0 = FMath::FloorToInt(X);
	int32 Y0 = FMath::FloorToInt(Y);
	int32 X1 = X0 + 1;
	int32 Y1 = Y0 + 1;

	float FracX = X - X0;
	float FracY = Y - Y0;

	float V00 = GetHeightSafe(Data, Width, Height, X0, Y0);
	float V10 = GetHeightSafe(Data, Width, Height, X1, Y0);
	float V01 = GetHeightSafe(Data, Width, Height, X0, Y1);
	float V11 = GetHeightSafe(Data, Width, Height, X1, Y1);

	float V0 = FMath::Lerp(V00, V10, FracX);
	float V1 = FMath::Lerp(V01, V11, FracX);

	return FMath::Lerp(V0, V1, FracY);
}

float UHeightmapConverter::SampleBicubic(const TArray<float>& Data, int32 Width, int32 Height, float X, float Y)
{
	int32 X1 = FMath::FloorToInt(X);
	int32 Y1 = FMath::FloorToInt(Y);

	float FracX = X - X1;
	float FracY = Y - Y1;

	float Values[4];
	for (int32 j = 0; j < 4; ++j)
	{
		int32 YPos = Y1 - 1 + j;
		float Row[4];

		for (int32 i = 0; i < 4; ++i)
		{
			int32 XPos = X1 - 1 + i;
			Row[i] = GetHeightSafe(Data, Width, Height, XPos, YPos);
		}

		Values[j] = CubicInterpolate(Row[0], Row[1], Row[2], Row[3], FracX);
	}

	return CubicInterpolate(Values[0], Values[1], Values[2], Values[3], FracY);
}

float UHeightmapConverter::CubicInterpolate(float p0, float p1, float p2, float p3, float t)
{
	float a = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
	float b = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
	float c = -0.5f * p0 + 0.5f * p2;
	float d = p1;

	return a * t * t * t + b * t * t + c * t + d;
}

float UHeightmapConverter::LanczosKernel(float x, int32 a)
{
	if (x == 0.0f)
	{
		return 1.0f;
	}

	if (FMath::Abs(x) >= a)
	{
		return 0.0f;
	}

	float PiX = PI * x;
	float PiXa = PiX / a;

	return (a * FMath::Sin(PiX) * FMath::Sin(PiXa)) / (PiX * PiXa);
}

float UHeightmapConverter::SampleLanczos(const TArray<float>& Data, int32 Width, int32 Height, float X, float Y)
{
	const int32 a = 3;
	int32 X0 = FMath::FloorToInt(X);
	int32 Y0 = FMath::FloorToInt(Y);

	float Sum = 0.0f;
	float WeightSum = 0.0f;

	for (int32 j = -a + 1; j <= a; ++j)
	{
		for (int32 i = -a + 1; i <= a; ++i)
		{
			float SampleX = X0 + i;
			float SampleY = Y0 + j;

			float Weight = LanczosKernel(X - SampleX, a) * LanczosKernel(Y - SampleY, a);
			float Value = GetHeightSafe(Data, Width, Height, static_cast<int32>(SampleX), static_cast<int32>(SampleY));

			Sum += Value * Weight;
			WeightSum += Weight;
		}
	}

	return (WeightSum > 0.0f) ? (Sum / WeightSum) : 0.0f;
}

// ============================================================================
// UTILITY HELPERS
// ============================================================================

float UHeightmapConverter::GetHeightSafe(const TArray<float>& Data, int32 Width, int32 Height, int32 X, int32 Y)
{
	X = FMath::Clamp(X, 0, Width - 1);
	Y = FMath::Clamp(Y, 0, Height - 1);
	return Data[Y * Width + X];
}

void UHeightmapConverter::FindMinMax(const TArray<float>& Data, float& OutMin, float& OutMax)
{
	if (Data.Num() == 0)
	{
		OutMin = 0.0f;
		OutMax = 0.0f;
		return;
	}

	OutMin = TNumericLimits<float>::Max();
	OutMax = TNumericLimits<float>::Lowest();

	for (float Value : Data)
	{
		OutMin = FMath::Min(OutMin, Value);
		OutMax = FMath::Max(OutMax, Value);
	}
}
