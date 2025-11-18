// Ultimate Procedural Map Generation System - Heightmap Export System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Landscape.h"
#include "HeightmapExporter.generated.h"

/**
 * Export format types for heightmap data
 */
UENUM(BlueprintType)
enum class EHeightmapExportFormat : uint8
{
	RAW_16Bit UMETA(DisplayName = "RAW 16-bit"),
	PNG_8Bit UMETA(DisplayName = "PNG 8-bit"),
	PNG_16Bit UMETA(DisplayName = "PNG 16-bit"),
	EXR_32Bit UMETA(DisplayName = "EXR 32-bit Float"),
	TGA_8Bit UMETA(DisplayName = "TGA 8-bit")
};

/**
 * Export mode for heightmap data
 */
UENUM(BlueprintType)
enum class EHeightmapExportMode : uint8
{
	Global UMETA(DisplayName = "Global (Single File)"),
	Batch UMETA(DisplayName = "Batch (Multiple Files)"),
	Tiles UMETA(DisplayName = "Tiles (Grid)")
};

/**
 * Export parameters structure
 */
USTRUCT(BlueprintType)
struct FHeightmapExportParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	EHeightmapExportFormat Format = EHeightmapExportFormat::RAW_16Bit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	EHeightmapExportMode Mode = EHeightmapExportMode::Global;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	FString OutputPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	FString FileName = TEXT("heightmap");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export|Tiling")
	int32 TileSize = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export|Tiling")
	int32 TileOverlap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export|Range")
	float MinHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export|Range")
	float MaxHeight = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export|Options")
	bool bNormalize = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export|Options")
	bool bFlipY = false;
};

/**
 * Heightmap Exporter - Exports terrain heightmap data to various formats
 * Supports RAW, PNG, EXR, and TGA formats with different bit depths
 */
UCLASS()
class PROCEDURALMAP_API UHeightmapExporter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// MAIN EXPORT FUNCTIONS
	// ============================================================================

	/**
	 * Export heightmap to RAW format (16-bit grayscale) - Industry standard
	 * @param HeightData - Array of height values
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param FilePath - Full path to output file
	 * @param MinHeight - Minimum height value for normalization
	 * @param MaxHeight - Maximum height value for normalization
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Export")
	static bool ExportHeightmapToRAW(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		const FString& FilePath,
		float MinHeight = 0.0f,
		float MaxHeight = 1000.0f
	);

	/**
	 * Export heightmap to PNG format with bit depth options
	 * @param HeightData - Array of height values
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param FilePath - Full path to output file
	 * @param b16Bit - Use 16-bit PNG (true) or 8-bit (false)
	 * @param MinHeight - Minimum height value for normalization
	 * @param MaxHeight - Maximum height value for normalization
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Export")
	static bool ExportHeightmapToPNG(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		const FString& FilePath,
		bool b16Bit = true,
		float MinHeight = 0.0f,
		float MaxHeight = 1000.0f
	);

	/**
	 * Export heightmap to EXR format (32-bit float, high precision)
	 * @param HeightData - Array of height values
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param FilePath - Full path to output file
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Export")
	static bool ExportHeightmapToEXR(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		const FString& FilePath
	);

	/**
	 * Export heightmap to TGA format (8-bit grayscale)
	 * @param HeightData - Array of height values
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param FilePath - Full path to output file
	 * @param MinHeight - Minimum height value for normalization
	 * @param MaxHeight - Maximum height value for normalization
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Export")
	static bool ExportHeightmapToTGA(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		const FString& FilePath,
		float MinHeight = 0.0f,
		float MaxHeight = 1000.0f
	);

	// ============================================================================
	// LANDSCAPE EXTRACTION
	// ============================================================================

	/**
	 * Extract heightmap data from Unreal Landscape
	 * @param Landscape - Landscape actor to extract from
	 * @param OutHeightData - Output height data array
	 * @param OutWidth - Output width
	 * @param OutHeight - Output height
	 * @param OutMinHeight - Output minimum height value
	 * @param OutMaxHeight - Output maximum height value
	 * @return True if extraction succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Extract")
	static bool ExtractHeightmapFromLandscape(
		ALandscape* Landscape,
		TArray<float>& OutHeightData,
		int32& OutWidth,
		int32& OutHeight,
		float& OutMinHeight,
		float& OutMaxHeight
	);

	/**
	 * Export heightmap directly from Landscape actor
	 * @param Landscape - Landscape actor to export from
	 * @param Params - Export parameters
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Export")
	static bool ExportFromLandscape(
		ALandscape* Landscape,
		const FHeightmapExportParams& Params
	);

	/**
	 * Export heightmap from procedural terrain data
	 * @param HeightData - Array of height values
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param Params - Export parameters
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Export")
	static bool ExportFromProceduralTerrain(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		const FHeightmapExportParams& Params
	);

	// ============================================================================
	// TILED EXPORT
	// ============================================================================

	/**
	 * Export heightmap in tiles (for large terrains)
	 * @param HeightData - Array of height values
	 * @param Width - Width of heightmap
	 * @param Height - Height of heightmap
	 * @param TileSize - Size of each tile
	 * @param OutputDirectory - Directory to save tiles
	 * @param FilePrefix - Prefix for tile filenames
	 * @param Format - Export format
	 * @param Overlap - Overlap between tiles in pixels
	 * @return Number of tiles exported
	 */
	UFUNCTION(BlueprintCallable, Category = "Heightmap|Export")
	static int32 ExportHeightmapTiles(
		const TArray<float>& HeightData,
		int32 Width,
		int32 Height,
		int32 TileSize,
		const FString& OutputDirectory,
		const FString& FilePrefix,
		EHeightmapExportFormat Format,
		int32 Overlap = 0
	);

private:
	// Helper functions
	static bool WriteRAWFile(const TArray<uint16>& Data, const FString& FilePath);
	static bool WritePNGFile(const TArray<uint8>& Data, int32 Width, int32 Height, const FString& FilePath, bool b16Bit);
	static bool WriteEXRFile(const TArray<float>& Data, int32 Width, int32 Height, const FString& FilePath);
	static bool WriteTGAFile(const TArray<uint8>& Data, int32 Width, int32 Height, const FString& FilePath);

	static void NormalizeHeightData(const TArray<float>& InData, TArray<float>& OutData, float MinHeight, float MaxHeight);
	static void ConvertFloatTo16Bit(const TArray<float>& InData, TArray<uint16>& OutData);
	static void ConvertFloatTo8Bit(const TArray<float>& InData, TArray<uint8>& OutData);
	static void FlipVertical(TArray<float>& Data, int32 Width, int32 Height);

	static bool ExtractTile(const TArray<float>& HeightData, int32 Width, int32 Height,
		int32 TileX, int32 TileY, int32 TileSize, int32 Overlap, TArray<float>& OutTileData);
};
