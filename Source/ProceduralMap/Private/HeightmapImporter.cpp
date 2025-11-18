// Ultimate Procedural Map Generation System - Heightmap Import System
// Copyright (C) 2025. All Rights Reserved.

#include "HeightmapImporter.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "Misc/Paths.h"

// ============================================================================
// MAIN IMPORT FUNCTIONS
// ============================================================================

bool UHeightmapImporter::ImportHeightmapFromRAW(
	const FString& FilePath,
	int32 Width,
	int32 Height,
	TArray<float>& OutHeightData,
	float MinHeight,
	float MaxHeight)
{
	TArray<uint16> RawData;
	if (!ReadRAWFile(FilePath, Width, Height, RawData))
	{
		return false;
	}

	Convert16BitToFloat(RawData, OutHeightData, MinHeight, MaxHeight);

	UE_LOG(LogTemp, Log, TEXT("HeightmapImporter: Successfully imported RAW heightmap %dx%d from %s"),
		Width, Height, *FilePath);

	return true;
}

bool UHeightmapImporter::ImportHeightmapFromPNG(
	const FString& FilePath,
	TArray<float>& OutHeightData,
	int32& OutWidth,
	int32& OutHeight,
	float MinHeight,
	float MaxHeight)
{
	TArray<uint8> ImageData;
	int32 BitDepth;

	if (!ReadImageFile(FilePath, ImageData, OutWidth, OutHeight, BitDepth))
	{
		return false;
	}

	// Convert based on bit depth
	if (BitDepth == 16)
	{
		// 16-bit PNG - extract from RGBA format
		TArray<uint16> Data16Bit;
		Data16Bit.SetNum(OutWidth * OutHeight);

		for (int32 i = 0; i < OutWidth * OutHeight; ++i)
		{
			uint8 High = ImageData[i * 4 + 0];
			uint8 Low = ImageData[i * 4 + 1];
			Data16Bit[i] = (static_cast<uint16>(High) << 8) | Low;
		}

		Convert16BitToFloat(Data16Bit, OutHeightData, MinHeight, MaxHeight);
	}
	else
	{
		// 8-bit PNG - extract from RGBA
		ConvertRGBAToFloat(ImageData, OutHeightData, OutWidth, OutHeight, MinHeight, MaxHeight);
	}

	UE_LOG(LogTemp, Log, TEXT("HeightmapImporter: Successfully imported PNG heightmap %dx%d (%d-bit) from %s"),
		OutWidth, OutHeight, BitDepth, *FilePath);

	return true;
}

bool UHeightmapImporter::ImportHeightmapFromEXR(
	const FString& FilePath,
	TArray<float>& OutHeightData,
	int32& OutWidth,
	int32& OutHeight)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);

	if (!ImageWrapper.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to create EXR wrapper"));
		return false;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to load EXR file: %s"), *FilePath);
		return false;
	}

	if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to decompress EXR file"));
		return false;
	}

	OutWidth = ImageWrapper->GetWidth();
	OutHeight = ImageWrapper->GetHeight();

	TArray64<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::RGBAF, 32, RawData))
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to get raw EXR data"));
		return false;
	}

	// Extract height from R channel
	OutHeightData.SetNum(OutWidth * OutHeight);
	const float* RGBAFloat = reinterpret_cast<const float*>(RawData.GetData());

	for (int32 i = 0; i < OutWidth * OutHeight; ++i)
	{
		OutHeightData[i] = RGBAFloat[i * 4]; // R channel
	}

	UE_LOG(LogTemp, Log, TEXT("HeightmapImporter: Successfully imported EXR heightmap %dx%d from %s"),
		OutWidth, OutHeight, *FilePath);

	return true;
}

bool UHeightmapImporter::ImportHeightmapFromTGA(
	const FString& FilePath,
	TArray<float>& OutHeightData,
	int32& OutWidth,
	int32& OutHeight,
	float MinHeight,
	float MaxHeight)
{
	TArray<uint8> ImageData;
	int32 BitDepth;

	if (!ReadImageFile(FilePath, ImageData, OutWidth, OutHeight, BitDepth))
	{
		return false;
	}

	ConvertRGBAToFloat(ImageData, OutHeightData, OutWidth, OutHeight, MinHeight, MaxHeight);

	UE_LOG(LogTemp, Log, TEXT("HeightmapImporter: Successfully imported TGA heightmap %dx%d from %s"),
		OutWidth, OutHeight, *FilePath);

	return true;
}

bool UHeightmapImporter::ImportHeightmap(
	const FHeightmapImportParams& Params,
	TArray<float>& OutHeightData,
	int32& OutWidth,
	int32& OutHeight)
{
	EHeightmapImportFormat Format = Params.Format;

	// Auto-detect format if requested
	if (Format == EHeightmapImportFormat::Auto)
	{
		Format = DetectFormat(Params.FilePath);
	}

	bool bSuccess = false;

	switch (Format)
	{
	case EHeightmapImportFormat::RAW_16Bit:
		if (Params.ExpectedWidth > 0 && Params.ExpectedHeight > 0)
		{
			OutWidth = Params.ExpectedWidth;
			OutHeight = Params.ExpectedHeight;
			bSuccess = ImportHeightmapFromRAW(Params.FilePath, OutWidth, OutHeight, OutHeightData,
				Params.MinHeight, Params.MaxHeight);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: RAW format requires explicit width and height"));
		}
		break;

	case EHeightmapImportFormat::PNG:
		bSuccess = ImportHeightmapFromPNG(Params.FilePath, OutHeightData, OutWidth, OutHeight,
			Params.MinHeight, Params.MaxHeight);
		break;

	case EHeightmapImportFormat::EXR:
		bSuccess = ImportHeightmapFromEXR(Params.FilePath, OutHeightData, OutWidth, OutHeight);
		break;

	case EHeightmapImportFormat::TGA:
		bSuccess = ImportHeightmapFromTGA(Params.FilePath, OutHeightData, OutWidth, OutHeight,
			Params.MinHeight, Params.MaxHeight);
		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Unknown format"));
		return false;
	}

	if (!bSuccess)
	{
		return false;
	}

	// Validate resolution
	if (Params.bValidatePowerOfTwo && !ValidateResolution(OutWidth, OutHeight))
	{
		UE_LOG(LogTemp, Warning, TEXT("HeightmapImporter: Resolution %dx%d is not valid for Unreal Landscape"),
			OutWidth, OutHeight);

		int32 RecommendedWidth, RecommendedHeight;
		GetRecommendedResolution(OutWidth, OutHeight, RecommendedWidth, RecommendedHeight);

		UE_LOG(LogTemp, Warning, TEXT("HeightmapImporter: Recommended resolution: %dx%d"),
			RecommendedWidth, RecommendedHeight);
	}

	// Auto-normalize if requested
	if (Params.bAutoNormalize)
	{
		NormalizeHeightData(OutHeightData);
	}

	// Flip Y if requested
	if (Params.bFlipY)
	{
		FlipVertical(OutHeightData, OutWidth, OutHeight);
	}

	return true;
}

// ============================================================================
// LANDSCAPE APPLICATION
// ============================================================================

bool UHeightmapImporter::ApplyToLandscape(
	ALandscape* Landscape,
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height)
{
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Invalid landscape"));
		return false;
	}

	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Data size mismatch"));
		return false;
	}

	// Get landscape info
	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to get landscape info"));
		return false;
	}

	FIntRect LandscapeBounds = Landscape->GetBoundingRect();

	if (Width != LandscapeBounds.Width() || Height != LandscapeBounds.Height())
	{
		UE_LOG(LogTemp, Warning, TEXT("HeightmapImporter: Size mismatch. Landscape: %dx%d, Heightmap: %dx%d"),
			LandscapeBounds.Width(), LandscapeBounds.Height(), Width, Height);
	}

	// Access landscape data
	FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);

	// Write height data
	for (int32 Y = 0; Y < Height && Y < LandscapeBounds.Height(); ++Y)
	{
		for (int32 X = 0; X < Width && X < LandscapeBounds.Width(); ++X)
		{
			float Height_f = HeightData[Y * Width + X];

			// Convert from world height to uint16
			uint16 HeightValue = static_cast<uint16>(FMath::Clamp(
				(Height_f / LANDSCAPE_ZSCALE) + 32768.0f,
				0.0f, 65535.0f));

			int32 LocalX = LandscapeBounds.Min.X + X;
			int32 LocalY = LandscapeBounds.Min.Y + Y;

			LandscapeEdit.SetHeightData(LocalX, LocalY, HeightValue);
		}
	}

	// Flush changes
	LandscapeEdit.Flush();

	UE_LOG(LogTemp, Log, TEXT("HeightmapImporter: Successfully applied heightmap to landscape"));

	return true;
}

bool UHeightmapImporter::ImportAndApplyToLandscape(
	ALandscape* Landscape,
	const FHeightmapImportParams& Params)
{
	TArray<float> HeightData;
	int32 Width, Height;

	if (!ImportHeightmap(Params, HeightData, Width, Height))
	{
		return false;
	}

	return ApplyToLandscape(Landscape, HeightData, Width, Height);
}

bool UHeightmapImporter::ApplyToProceduralTerrain(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	TArray<float>& OutProceduralData)
{
	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Data size mismatch"));
		return false;
	}

	OutProceduralData = HeightData;

	UE_LOG(LogTemp, Log, TEXT("HeightmapImporter: Applied heightmap to procedural terrain (%dx%d)"),
		Width, Height);

	return true;
}

// ============================================================================
// VALIDATION
// ============================================================================

bool UHeightmapImporter::ValidateResolution(int32 Width, int32 Height)
{
	// Unreal Landscape requires (2^n + 1) x (2^n + 1) or similar valid combinations
	// Common valid sizes: 127, 255, 511, 1009, 2017, 4033, 8129

	auto IsValidSize = [](int32 Size) -> bool
	{
		// Check if it's (2^n + 1)
		int32 Test = Size - 1;
		return (Test > 0) && ((Test & (Test - 1)) == 0);
	};

	return IsValidSize(Width) && IsValidSize(Height);
}

void UHeightmapImporter::GetRecommendedResolution(
	int32 Width,
	int32 Height,
	int32& OutRecommendedWidth,
	int32& OutRecommendedHeight)
{
	OutRecommendedWidth = GetNearestUnrealResolution(Width);
	OutRecommendedHeight = GetNearestUnrealResolution(Height);
}

EHeightmapImportFormat UHeightmapImporter::DetectFormat(const FString& FilePath)
{
	FString Extension = FPaths::GetExtension(FilePath).ToLower();

	if (Extension == TEXT("raw") || Extension == TEXT("r16"))
	{
		return EHeightmapImportFormat::RAW_16Bit;
	}
	else if (Extension == TEXT("png"))
	{
		return EHeightmapImportFormat::PNG;
	}
	else if (Extension == TEXT("exr"))
	{
		return EHeightmapImportFormat::EXR;
	}
	else if (Extension == TEXT("tga"))
	{
		return EHeightmapImportFormat::TGA;
	}

	UE_LOG(LogTemp, Warning, TEXT("HeightmapImporter: Unknown extension '%s', defaulting to PNG"), *Extension);
	return EHeightmapImportFormat::PNG;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

bool UHeightmapImporter::ReadRAWFile(const FString& FilePath, int32 Width, int32 Height, TArray<uint16>& OutData)
{
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to load RAW file: %s"), *FilePath);
		return false;
	}

	int32 ExpectedSize = Width * Height * 2;
	if (FileData.Num() != ExpectedSize)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: RAW file size mismatch. Expected %d bytes, got %d"),
			ExpectedSize, FileData.Num());
		return false;
	}

	OutData.SetNum(Width * Height);

	for (int32 i = 0; i < Width * Height; ++i)
	{
		uint8 High = FileData[i * 2 + 0];
		uint8 Low = FileData[i * 2 + 1];
		OutData[i] = (static_cast<uint16>(High) << 8) | Low;
	}

	return true;
}

bool UHeightmapImporter::ReadImageFile(const FString& FilePath, TArray<uint8>& OutData, int32& OutWidth, int32& OutHeight, int32& OutBitDepth)
{
	FString Extension = FPaths::GetExtension(FilePath).ToLower();
	EImageFormat ImageFormat = EImageFormat::Invalid;

	if (Extension == TEXT("png"))
	{
		ImageFormat = EImageFormat::PNG;
	}
	else if (Extension == TEXT("tga"))
	{
		ImageFormat = EImageFormat::TGA;
	}
	else if (Extension == TEXT("jpg") || Extension == TEXT("jpeg"))
	{
		ImageFormat = EImageFormat::JPEG;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Unsupported image format: %s"), *Extension);
		return false;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

	if (!ImageWrapper.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to create image wrapper for %s"), *Extension);
		return false;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to load file: %s"), *FilePath);
		return false;
	}

	if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to decompress image"));
		return false;
	}

	OutWidth = ImageWrapper->GetWidth();
	OutHeight = ImageWrapper->GetHeight();
	OutBitDepth = ImageWrapper->GetBitDepth();

	TArray64<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData))
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapImporter: Failed to get raw image data"));
		return false;
	}

	OutData = TArray<uint8>(RawData);

	return true;
}

void UHeightmapImporter::Convert16BitToFloat(const TArray<uint16>& InData, TArray<float>& OutData, float MinHeight, float MaxHeight)
{
	OutData.SetNum(InData.Num());

	float Range = MaxHeight - MinHeight;

	for (int32 i = 0; i < InData.Num(); ++i)
	{
		float Normalized = InData[i] / 65535.0f;
		OutData[i] = MinHeight + (Normalized * Range);
	}
}

void UHeightmapImporter::Convert8BitToFloat(const TArray<uint8>& InData, TArray<float>& OutData, float MinHeight, float MaxHeight)
{
	OutData.SetNum(InData.Num());

	float Range = MaxHeight - MinHeight;

	for (int32 i = 0; i < InData.Num(); ++i)
	{
		float Normalized = InData[i] / 255.0f;
		OutData[i] = MinHeight + (Normalized * Range);
	}
}

void UHeightmapImporter::ConvertRGBAToFloat(const TArray<uint8>& InData, TArray<float>& OutData, int32 Width, int32 Height, float MinHeight, float MaxHeight)
{
	OutData.SetNum(Width * Height);

	float Range = MaxHeight - MinHeight;

	for (int32 i = 0; i < Width * Height; ++i)
	{
		// Use R channel for grayscale
		uint8 Value = InData[i * 4];
		float Normalized = Value / 255.0f;
		OutData[i] = MinHeight + (Normalized * Range);
	}
}

void UHeightmapImporter::NormalizeHeightData(TArray<float>& Data)
{
	if (Data.Num() == 0)
	{
		return;
	}

	float MinVal = TNumericLimits<float>::Max();
	float MaxVal = TNumericLimits<float>::Lowest();

	for (float Value : Data)
	{
		MinVal = FMath::Min(MinVal, Value);
		MaxVal = FMath::Max(MaxVal, Value);
	}

	float Range = MaxVal - MinVal;
	if (Range > 0.0f)
	{
		for (float& Value : Data)
		{
			Value = (Value - MinVal) / Range;
		}
	}
}

void UHeightmapImporter::FlipVertical(TArray<float>& Data, int32 Width, int32 Height)
{
	for (int32 Y = 0; Y < Height / 2; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 TopIndex = Y * Width + X;
			int32 BottomIndex = (Height - 1 - Y) * Width + X;

			float Temp = Data[TopIndex];
			Data[TopIndex] = Data[BottomIndex];
			Data[BottomIndex] = Temp;
		}
	}
}

bool UHeightmapImporter::IsPowerOfTwo(int32 Value)
{
	return (Value > 0) && ((Value & (Value - 1)) == 0);
}

int32 UHeightmapImporter::GetNearestPowerOfTwo(int32 Value)
{
	int32 Power = 1;
	while (Power < Value)
	{
		Power *= 2;
	}

	int32 LowerPower = Power / 2;

	return (Value - LowerPower < Power - Value) ? LowerPower : Power;
}

int32 UHeightmapImporter::GetNearestUnrealResolution(int32 Value)
{
	// Unreal valid sizes: (2^n + 1)
	// Common sizes: 127, 255, 511, 1009, 2017, 4033, 8129

	TArray<int32> ValidSizes = { 127, 255, 511, 1009, 2017, 4033, 8129 };

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
