// Ultimate Procedural Map Generation System - Heightmap Export System
// Copyright (C) 2025. All Rights Reserved.

#include "HeightmapExporter.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"
#include "Misc/Paths.h"

// ============================================================================
// MAIN EXPORT FUNCTIONS
// ============================================================================

bool UHeightmapExporter::ExportHeightmapToRAW(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	const FString& FilePath,
	float MinHeight,
	float MaxHeight)
{
	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Data size mismatch. Expected %d, got %d"), Width * Height, HeightData.Num());
		return false;
	}

	// Normalize and convert to 16-bit
	TArray<float> NormalizedData;
	NormalizeHeightData(HeightData, NormalizedData, MinHeight, MaxHeight);

	TArray<uint16> Data16Bit;
	ConvertFloatTo16Bit(NormalizedData, Data16Bit);

	return WriteRAWFile(Data16Bit, FilePath);
}

bool UHeightmapExporter::ExportHeightmapToPNG(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	const FString& FilePath,
	bool b16Bit,
	float MinHeight,
	float MaxHeight)
{
	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Data size mismatch"));
		return false;
	}

	// Normalize data
	TArray<float> NormalizedData;
	NormalizeHeightData(HeightData, NormalizedData, MinHeight, MaxHeight);

	if (b16Bit)
	{
		// Convert to 16-bit grayscale
		TArray<uint16> Data16Bit;
		ConvertFloatTo16Bit(NormalizedData, Data16Bit);

		// Convert to RGBA for PNG (each pixel = RRGGBBAA from 16-bit value)
		TArray<uint8> RGBAData;
		RGBAData.SetNum(Width * Height * 4);

		for (int32 i = 0; i < Data16Bit.Num(); ++i)
		{
			uint16 Value = Data16Bit[i];
			uint8 High = (Value >> 8) & 0xFF;
			uint8 Low = Value & 0xFF;

			RGBAData[i * 4 + 0] = High;
			RGBAData[i * 4 + 1] = Low;
			RGBAData[i * 4 + 2] = 0;
			RGBAData[i * 4 + 3] = 255;
		}

		return WritePNGFile(RGBAData, Width, Height, FilePath, true);
	}
	else
	{
		// Convert to 8-bit grayscale
		TArray<uint8> Data8Bit;
		ConvertFloatTo8Bit(NormalizedData, Data8Bit);

		// Convert to RGBA for PNG
		TArray<uint8> RGBAData;
		RGBAData.SetNum(Width * Height * 4);

		for (int32 i = 0; i < Data8Bit.Num(); ++i)
		{
			RGBAData[i * 4 + 0] = Data8Bit[i];
			RGBAData[i * 4 + 1] = Data8Bit[i];
			RGBAData[i * 4 + 2] = Data8Bit[i];
			RGBAData[i * 4 + 3] = 255;
		}

		return WritePNGFile(RGBAData, Width, Height, FilePath, false);
	}
}

bool UHeightmapExporter::ExportHeightmapToEXR(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	const FString& FilePath)
{
	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Data size mismatch"));
		return false;
	}

	return WriteEXRFile(HeightData, Width, Height, FilePath);
}

bool UHeightmapExporter::ExportHeightmapToTGA(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	const FString& FilePath,
	float MinHeight,
	float MaxHeight)
{
	if (HeightData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Data size mismatch"));
		return false;
	}

	// Normalize and convert to 8-bit
	TArray<float> NormalizedData;
	NormalizeHeightData(HeightData, NormalizedData, MinHeight, MaxHeight);

	TArray<uint8> Data8Bit;
	ConvertFloatTo8Bit(NormalizedData, Data8Bit);

	return WriteTGAFile(Data8Bit, Width, Height, FilePath);
}

// ============================================================================
// LANDSCAPE EXTRACTION
// ============================================================================

bool UHeightmapExporter::ExtractHeightmapFromLandscape(
	ALandscape* Landscape,
	TArray<float>& OutHeightData,
	int32& OutWidth,
	int32& OutHeight,
	float& OutMinHeight,
	float& OutMaxHeight)
{
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Invalid landscape"));
		return false;
	}

	// Get landscape info
	FIntRect LandscapeBounds = Landscape->GetBoundingRect();
	OutWidth = LandscapeBounds.Width();
	OutHeight = LandscapeBounds.Height();

	if (OutWidth <= 0 || OutHeight <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Invalid landscape dimensions"));
		return false;
	}

	// Prepare output array
	OutHeightData.SetNum(OutWidth * OutHeight);

	// Access landscape data
	FLandscapeEditDataInterface LandscapeEdit(Landscape->GetLandscapeInfo());

	OutMinHeight = TNumericLimits<float>::Max();
	OutMaxHeight = TNumericLimits<float>::Lowest();

	// Read height data
	for (int32 Y = 0; Y < OutHeight; ++Y)
	{
		for (int32 X = 0; X < OutWidth; ++X)
		{
			int32 LocalX = LandscapeBounds.Min.X + X;
			int32 LocalY = LandscapeBounds.Min.Y + Y;

			uint16 HeightValue = 0;
			LandscapeEdit.GetHeightData(LocalX, LocalY, HeightValue);

			// Convert from uint16 to world height
			float Height = ((float)HeightValue - 32768.0f) * LANDSCAPE_ZSCALE;

			OutHeightData[Y * OutWidth + X] = Height;

			OutMinHeight = FMath::Min(OutMinHeight, Height);
			OutMaxHeight = FMath::Max(OutMaxHeight, Height);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HeightmapExporter: Extracted %dx%d heightmap (range: %.2f to %.2f)"),
		OutWidth, OutHeight, OutMinHeight, OutMaxHeight);

	return true;
}

bool UHeightmapExporter::ExportFromLandscape(
	ALandscape* Landscape,
	const FHeightmapExportParams& Params)
{
	TArray<float> HeightData;
	int32 Width, Height;
	float MinHeight, MaxHeight;

	if (!ExtractHeightmapFromLandscape(Landscape, HeightData, Width, Height, MinHeight, MaxHeight))
	{
		return false;
	}

	// Use extracted min/max if not specified in params
	float ExportMinHeight = Params.bNormalize ? Params.MinHeight : MinHeight;
	float ExportMaxHeight = Params.bNormalize ? Params.MaxHeight : MaxHeight;

	return ExportFromProceduralTerrain(HeightData, Width, Height, Params);
}

bool UHeightmapExporter::ExportFromProceduralTerrain(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	const FHeightmapExportParams& Params)
{
	TArray<float> ProcessedData = HeightData;

	// Apply flip if requested
	if (Params.bFlipY)
	{
		FlipVertical(ProcessedData, Width, Height);
	}

	FString FullPath = FPaths::Combine(Params.OutputPath, Params.FileName);

	// Export based on mode
	switch (Params.Mode)
	{
	case EHeightmapExportMode::Global:
	{
		// Single file export
		switch (Params.Format)
		{
		case EHeightmapExportFormat::RAW_16Bit:
			FullPath += TEXT(".raw");
			return ExportHeightmapToRAW(ProcessedData, Width, Height, FullPath, Params.MinHeight, Params.MaxHeight);

		case EHeightmapExportFormat::PNG_8Bit:
			FullPath += TEXT(".png");
			return ExportHeightmapToPNG(ProcessedData, Width, Height, FullPath, false, Params.MinHeight, Params.MaxHeight);

		case EHeightmapExportFormat::PNG_16Bit:
			FullPath += TEXT(".png");
			return ExportHeightmapToPNG(ProcessedData, Width, Height, FullPath, true, Params.MinHeight, Params.MaxHeight);

		case EHeightmapExportFormat::EXR_32Bit:
			FullPath += TEXT(".exr");
			return ExportHeightmapToEXR(ProcessedData, Width, Height, FullPath);

		case EHeightmapExportFormat::TGA_8Bit:
			FullPath += TEXT(".tga");
			return ExportHeightmapToTGA(ProcessedData, Width, Height, FullPath, Params.MinHeight, Params.MaxHeight);
		}
		break;
	}

	case EHeightmapExportMode::Tiles:
	{
		// Tiled export
		int32 NumTiles = ExportHeightmapTiles(
			ProcessedData, Width, Height,
			Params.TileSize,
			Params.OutputPath,
			Params.FileName,
			Params.Format,
			Params.TileOverlap
		);

		UE_LOG(LogTemp, Log, TEXT("HeightmapExporter: Exported %d tiles"), NumTiles);
		return NumTiles > 0;
	}

	default:
		break;
	}

	return false;
}

// ============================================================================
// TILED EXPORT
// ============================================================================

int32 UHeightmapExporter::ExportHeightmapTiles(
	const TArray<float>& HeightData,
	int32 Width,
	int32 Height,
	int32 TileSize,
	const FString& OutputDirectory,
	const FString& FilePrefix,
	EHeightmapExportFormat Format,
	int32 Overlap)
{
	int32 TilesExported = 0;
	int32 EffectiveTileSize = TileSize + Overlap;

	int32 NumTilesX = FMath::CeilToInt((float)Width / TileSize);
	int32 NumTilesY = FMath::CeilToInt((float)Height / TileSize);

	for (int32 TileY = 0; TileY < NumTilesY; ++TileY)
	{
		for (int32 TileX = 0; TileX < NumTilesX; ++TileX)
		{
			TArray<float> TileData;
			if (ExtractTile(HeightData, Width, Height, TileX, TileY, TileSize, Overlap, TileData))
			{
				FString TileName = FString::Printf(TEXT("%s_tile_%d_%d"), *FilePrefix, TileX, TileY);
				FString TilePath = FPaths::Combine(OutputDirectory, TileName);

				bool bSuccess = false;
				switch (Format)
				{
				case EHeightmapExportFormat::RAW_16Bit:
					TilePath += TEXT(".raw");
					bSuccess = ExportHeightmapToRAW(TileData, EffectiveTileSize, EffectiveTileSize, TilePath, 0.0f, 1000.0f);
					break;

				case EHeightmapExportFormat::PNG_16Bit:
					TilePath += TEXT(".png");
					bSuccess = ExportHeightmapToPNG(TileData, EffectiveTileSize, EffectiveTileSize, TilePath, true, 0.0f, 1000.0f);
					break;

				case EHeightmapExportFormat::EXR_32Bit:
					TilePath += TEXT(".exr");
					bSuccess = ExportHeightmapToEXR(TileData, EffectiveTileSize, EffectiveTileSize, TilePath);
					break;

				default:
					break;
				}

				if (bSuccess)
				{
					TilesExported++;
				}
			}
		}
	}

	return TilesExported;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

bool UHeightmapExporter::WriteRAWFile(const TArray<uint16>& Data, const FString& FilePath)
{
	TArray<uint8> ByteData;
	ByteData.SetNum(Data.Num() * 2);

	for (int32 i = 0; i < Data.Num(); ++i)
	{
		ByteData[i * 2 + 0] = (Data[i] >> 8) & 0xFF;
		ByteData[i * 2 + 1] = Data[i] & 0xFF;
	}

	if (FFileHelper::SaveArrayToFile(ByteData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("HeightmapExporter: Successfully exported RAW to %s"), *FilePath);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Failed to write RAW file to %s"), *FilePath);
	return false;
}

bool UHeightmapExporter::WritePNGFile(const TArray<uint8>& Data, int32 Width, int32 Height, const FString& FilePath, bool b16Bit)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Failed to create PNG wrapper"));
		return false;
	}

	ERGBFormat Format = ERGBFormat::RGBA;
	int32 BitDepth = b16Bit ? 16 : 8;

	if (ImageWrapper->SetRaw(Data.GetData(), Data.Num(), Width, Height, Format, BitDepth))
	{
		const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();

		if (FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
		{
			UE_LOG(LogTemp, Log, TEXT("HeightmapExporter: Successfully exported PNG to %s"), *FilePath);
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Failed to write PNG file to %s"), *FilePath);
	return false;
}

bool UHeightmapExporter::WriteEXRFile(const TArray<float>& Data, int32 Width, int32 Height, const FString& FilePath)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);

	if (!ImageWrapper.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Failed to create EXR wrapper"));
		return false;
	}

	// Convert float data to RGBA format (store height in R channel)
	TArray<uint8> RGBAData;
	RGBAData.SetNum(Width * Height * 4 * sizeof(float));

	float* RGBAFloat = reinterpret_cast<float*>(RGBAData.GetData());

	for (int32 i = 0; i < Data.Num(); ++i)
	{
		RGBAFloat[i * 4 + 0] = Data[i];
		RGBAFloat[i * 4 + 1] = Data[i];
		RGBAFloat[i * 4 + 2] = Data[i];
		RGBAFloat[i * 4 + 3] = 1.0f;
	}

	if (ImageWrapper->SetRaw(RGBAData.GetData(), RGBAData.Num(), Width, Height, ERGBFormat::RGBAF, 32))
	{
		const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();

		if (FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
		{
			UE_LOG(LogTemp, Log, TEXT("HeightmapExporter: Successfully exported EXR to %s"), *FilePath);
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Failed to write EXR file to %s"), *FilePath);
	return false;
}

bool UHeightmapExporter::WriteTGAFile(const TArray<uint8>& Data, int32 Width, int32 Height, const FString& FilePath)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::TGA);

	if (!ImageWrapper.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Failed to create TGA wrapper"));
		return false;
	}

	// Convert to RGBA
	TArray<uint8> RGBAData;
	RGBAData.SetNum(Width * Height * 4);

	for (int32 i = 0; i < Data.Num(); ++i)
	{
		RGBAData[i * 4 + 0] = Data[i];
		RGBAData[i * 4 + 1] = Data[i];
		RGBAData[i * 4 + 2] = Data[i];
		RGBAData[i * 4 + 3] = 255;
	}

	if (ImageWrapper->SetRaw(RGBAData.GetData(), RGBAData.Num(), Width, Height, ERGBFormat::RGBA, 8))
	{
		const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();

		if (FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
		{
			UE_LOG(LogTemp, Log, TEXT("HeightmapExporter: Successfully exported TGA to %s"), *FilePath);
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("HeightmapExporter: Failed to write TGA file to %s"), *FilePath);
	return false;
}

void UHeightmapExporter::NormalizeHeightData(const TArray<float>& InData, TArray<float>& OutData, float MinHeight, float MaxHeight)
{
	OutData.SetNum(InData.Num());

	float Range = MaxHeight - MinHeight;
	if (Range <= 0.0f)
	{
		Range = 1.0f;
	}

	for (int32 i = 0; i < InData.Num(); ++i)
	{
		OutData[i] = FMath::Clamp((InData[i] - MinHeight) / Range, 0.0f, 1.0f);
	}
}

void UHeightmapExporter::ConvertFloatTo16Bit(const TArray<float>& InData, TArray<uint16>& OutData)
{
	OutData.SetNum(InData.Num());

	for (int32 i = 0; i < InData.Num(); ++i)
	{
		OutData[i] = static_cast<uint16>(FMath::Clamp(InData[i], 0.0f, 1.0f) * 65535.0f);
	}
}

void UHeightmapExporter::ConvertFloatTo8Bit(const TArray<float>& InData, TArray<uint8>& OutData)
{
	OutData.SetNum(InData.Num());

	for (int32 i = 0; i < InData.Num(); ++i)
	{
		OutData[i] = static_cast<uint8>(FMath::Clamp(InData[i], 0.0f, 1.0f) * 255.0f);
	}
}

void UHeightmapExporter::FlipVertical(TArray<float>& Data, int32 Width, int32 Height)
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

bool UHeightmapExporter::ExtractTile(const TArray<float>& HeightData, int32 Width, int32 Height,
	int32 TileX, int32 TileY, int32 TileSize, int32 Overlap, TArray<float>& OutTileData)
{
	int32 EffectiveTileSize = TileSize + Overlap;
	OutTileData.SetNum(EffectiveTileSize * EffectiveTileSize);

	int32 StartX = TileX * TileSize;
	int32 StartY = TileY * TileSize;

	for (int32 Y = 0; Y < EffectiveTileSize; ++Y)
	{
		for (int32 X = 0; X < EffectiveTileSize; ++X)
		{
			int32 SourceX = FMath::Clamp(StartX + X, 0, Width - 1);
			int32 SourceY = FMath::Clamp(StartY + Y, 0, Height - 1);

			int32 SourceIndex = SourceY * Width + SourceX;
			int32 TileIndex = Y * EffectiveTileSize + X;

			OutTileData[TileIndex] = HeightData[SourceIndex];
		}
	}

	return true;
}
