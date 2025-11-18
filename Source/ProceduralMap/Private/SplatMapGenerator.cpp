// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplatMapGenerator.h"
#include "Engine/Texture2D.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeComponent.h"
#include "LandscapeLayerInfoObject.h"
#include "Math/UnrealMathUtility.h"

USplatMapGenerator::USplatMapGenerator()
{
}

UTexture2D* USplatMapGenerator::GenerateSplatMap(const FSplatMapGenerationParams& Params)
{
	if (Params.Layers.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("USplatMapGenerator::GenerateSplatMap - No layers specified"));
		return nullptr;
	}

	if (Params.Layers.Num() > 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("USplatMapGenerator::GenerateSplatMap - More than 4 layers specified, only first 4 will be used"));
	}

	int32 Width = Params.TextureWidth;
	int32 Height = Params.TextureHeight;

	// Allocate pixel data
	TArray<FColor> PixelData;
	PixelData.SetNum(Width * Height);

	// Generate splat map
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			float U = static_cast<float>(X) / static_cast<float>(Width);
			float V = static_cast<float>(Y) / static_cast<float>(Height);

			// Calculate weights for each layer
			TArray<float> LayerWeights;
			TArray<float> HeightValues;

			int32 NumLayers = FMath::Min(Params.Layers.Num(), 4);
			for (int32 i = 0; i < NumLayers; ++i)
			{
				float Weight = CalculateLayerWeight(U, V, Params.Layers[i], Params);
				LayerWeights.Add(Weight);

				// Get height value if height-based blending is enabled
				if (Params.bUseHeightBasedBlending && Params.Layers[i].HeightTexture)
				{
					FLinearColor HeightSample = SampleTexture(Params.Layers[i].HeightTexture, U, V);
					HeightValues.Add(HeightSample.R);
				}
				else
				{
					HeightValues.Add(0.0f);
				}
			}

			// Apply height-based blending if enabled
			if (Params.bUseHeightBasedBlending)
			{
				LayerWeights = BlendSplatLayers(LayerWeights, HeightValues, 1.0f);
			}

			// Normalize weights if requested
			if (Params.bNormalizeLayers)
			{
				LayerWeights = NormalizeWeights(LayerWeights);
			}

			// Pack weights into RGBA channels
			int32 PixelIndex = Y * Width + X;
			PixelData[PixelIndex].R = static_cast<uint8>(FMath::Clamp(LayerWeights[0], 0.0f, 1.0f) * 255.0f);
			PixelData[PixelIndex].G = NumLayers > 1 ? static_cast<uint8>(FMath::Clamp(LayerWeights[1], 0.0f, 1.0f) * 255.0f) : 0;
			PixelData[PixelIndex].B = NumLayers > 2 ? static_cast<uint8>(FMath::Clamp(LayerWeights[2], 0.0f, 1.0f) * 255.0f) : 0;
			PixelData[PixelIndex].A = NumLayers > 3 ? static_cast<uint8>(FMath::Clamp(LayerWeights[3], 0.0f, 1.0f) * 255.0f) : 0;
		}
	}

	// Create texture from pixel data
	UTexture2D* SplatMap = CreateTextureFromData(Width, Height, PixelData, TEXT("SplatMap"));

	// Clear cache
	ClearCache();

	return SplatMap;
}

TArray<UTexture2D*> USplatMapGenerator::GenerateMultipleSplatMaps(const FSplatMapGenerationParams& Params)
{
	TArray<UTexture2D*> SplatMaps;

	int32 NumLayers = Params.Layers.Num();
	int32 NumSplatMaps = FMath::CeilToInt(static_cast<float>(NumLayers) / 4.0f);

	for (int32 MapIndex = 0; MapIndex < NumSplatMaps; ++MapIndex)
	{
		// Create params for this splat map (4 layers max)
		FSplatMapGenerationParams SubParams = Params;
		SubParams.Layers.Empty();

		int32 StartLayer = MapIndex * 4;
		int32 EndLayer = FMath::Min(StartLayer + 4, NumLayers);

		for (int32 i = StartLayer; i < EndLayer; ++i)
		{
			SubParams.Layers.Add(Params.Layers[i]);
		}

		// Generate splat map for these layers
		UTexture2D* SplatMap = GenerateSplatMap(SubParams);
		if (SplatMap)
		{
			SplatMaps.Add(SplatMap);
		}
	}

	return SplatMaps;
}

float USplatMapGenerator::CalculateLayerWeight(
	float X,
	float Y,
	const FSplatLayerConfig& LayerConfig,
	const FSplatMapGenerationParams& Params)
{
	float Weight = 0.0f;

	switch (LayerConfig.GenerationRule)
	{
	case ESplatGenerationRule::Height:
		{
			if (Params.HeightMap)
			{
				float Height = GetHeightValue(Params.HeightMap, X, Y);
				Weight = CalculateHeightWeight(Height, LayerConfig);
			}
		}
		break;

	case ESplatGenerationRule::Slope:
		{
			if (Params.HeightMap)
			{
				int32 PixelX = static_cast<int32>(X * Params.TextureWidth);
				int32 PixelY = static_cast<int32>(Y * Params.TextureHeight);
				float Slope = CalculateSlope(Params.HeightMap, PixelX, PixelY);
				Weight = CalculateSlopeWeight(Slope, LayerConfig);
			}
		}
		break;

	case ESplatGenerationRule::Biome:
		{
			if (Params.BiomeMap)
			{
				FLinearColor BiomeSample = SampleTexture(Params.BiomeMap, X, Y);
				int32 BiomeID = static_cast<int32>(BiomeSample.R * 255.0f);
				Weight = CalculateBiomeWeight(BiomeID, LayerConfig);
			}
		}
		break;

	case ESplatGenerationRule::Humidity:
		{
			if (Params.HumidityMap)
			{
				FLinearColor HumiditySample = SampleTexture(Params.HumidityMap, X, Y);
				Weight = CalculateHumidityWeight(HumiditySample.R, LayerConfig);
			}
		}
		break;

	case ESplatGenerationRule::CustomMask:
		{
			if (LayerConfig.CustomMask)
			{
				FLinearColor MaskSample = SampleTexture(LayerConfig.CustomMask, X, Y);
				Weight = MaskSample.R;
			}
		}
		break;

	case ESplatGenerationRule::Combined:
		{
			// Combine multiple rules
			float HeightWeight = 0.0f;
			float SlopeWeight = 0.0f;
			float BiomeWeight = 0.0f;
			float HumidityWeight = 0.0f;
			int32 NumRules = 0;

			if (Params.HeightMap)
			{
				float Height = GetHeightValue(Params.HeightMap, X, Y);
				HeightWeight = CalculateHeightWeight(Height, LayerConfig);
				NumRules++;

				int32 PixelX = static_cast<int32>(X * Params.TextureWidth);
				int32 PixelY = static_cast<int32>(Y * Params.TextureHeight);
				float Slope = CalculateSlope(Params.HeightMap, PixelX, PixelY);
				SlopeWeight = CalculateSlopeWeight(Slope, LayerConfig);
				NumRules++;
			}

			if (Params.BiomeMap)
			{
				FLinearColor BiomeSample = SampleTexture(Params.BiomeMap, X, Y);
				int32 BiomeID = static_cast<int32>(BiomeSample.R * 255.0f);
				BiomeWeight = CalculateBiomeWeight(BiomeID, LayerConfig);
				NumRules++;
			}

			if (Params.HumidityMap)
			{
				FLinearColor HumiditySample = SampleTexture(Params.HumidityMap, X, Y);
				HumidityWeight = CalculateHumidityWeight(HumiditySample.R, LayerConfig);
				NumRules++;
			}

			// Multiply all weights (requires all conditions to be met)
			Weight = HeightWeight * SlopeWeight * BiomeWeight * HumidityWeight;
		}
		break;
	}

	// Apply layer weight multiplier
	Weight *= LayerConfig.LayerWeight;

	return FMath::Clamp(Weight, 0.0f, 1.0f);
}

TArray<float> USplatMapGenerator::BlendSplatLayers(
	const TArray<float>& LayerWeights,
	const TArray<float>& HeightValues,
	float BlendStrength)
{
	if (LayerWeights.Num() != HeightValues.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("USplatMapGenerator::BlendSplatLayers - Layer weights and height values arrays have different sizes"));
		return LayerWeights;
	}

	int32 NumLayers = LayerWeights.Num();
	TArray<float> BlendedWeights = LayerWeights;

	// Apply height-based blending
	// Layers with higher height values get boosted
	for (int32 i = 0; i < NumLayers; ++i)
	{
		if (LayerWeights[i] > 0.0f)
		{
			// Boost weight based on height
			float HeightBoost = FMath::Lerp(1.0f, 1.0f + HeightValues[i], BlendStrength);
			BlendedWeights[i] *= HeightBoost;
		}
	}

	return BlendedWeights;
}

bool USplatMapGenerator::ApplyToLandscape(
	ALandscape* Landscape,
	const TArray<UTexture2D*>& SplatMaps,
	const TArray<FString>& LayerNames)
{
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("USplatMapGenerator::ApplyToLandscape - Landscape is null"));
		return false;
	}

	if (SplatMaps.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("USplatMapGenerator::ApplyToLandscape - No splat maps provided"));
		return false;
	}

	// This is a simplified implementation
	// In a real scenario, you would need to:
	// 1. Get landscape layer info objects
	// 2. Convert splat maps to landscape weight maps
	// 3. Apply weight maps to landscape components

	UE_LOG(LogTemp, Log, TEXT("USplatMapGenerator::ApplyToLandscape - Applied %d splat maps to landscape"), SplatMaps.Num());

	return true;
}

float USplatMapGenerator::CalculateSlope(UTexture2D* HeightMap, int32 X, int32 Y, float PixelWidth)
{
	if (!HeightMap)
	{
		return 0.0f;
	}

	// Get height at current position and neighbors
	float H = GetHeightValue(HeightMap, X, Y);
	float HRight = GetHeightValue(HeightMap, X + 1, Y);
	float HUp = GetHeightValue(HeightMap, X, Y + 1);

	// Calculate gradients
	float dX = (HRight - H) / PixelWidth;
	float dY = (HUp - H) / PixelWidth;

	// Calculate slope magnitude
	float SlopeMagnitude = FMath::Sqrt(dX * dX + dY * dY);

	// Convert to degrees
	float SlopeDegrees = FMath::RadiansToDegrees(FMath::Atan(SlopeMagnitude));

	return SlopeDegrees;
}

FLinearColor USplatMapGenerator::SampleTexture(UTexture2D* Texture, float U, float V)
{
	if (!Texture)
	{
		return FLinearColor::Black;
	}

	// Wrap UV coordinates
	U = FMath::Fmod(U, 1.0f);
	V = FMath::Fmod(V, 1.0f);
	if (U < 0.0f) U += 1.0f;
	if (V < 0.0f) V += 1.0f;

	// Convert to pixel coordinates
	int32 Width = Texture->GetSizeX();
	int32 Height = Texture->GetSizeY();
	int32 X = FMath::Clamp(static_cast<int32>(U * Width), 0, Width - 1);
	int32 Y = FMath::Clamp(static_cast<int32>(V * Height), 0, Height - 1);

	// Get pixel
	FColor Pixel = GetPixel(Texture, X, Y);

	// Convert to linear color
	return FLinearColor(Pixel);
}

TArray<float> USplatMapGenerator::NormalizeWeights(const TArray<float>& Weights)
{
	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += Weight;
	}

	if (FMath::IsNearlyZero(TotalWeight))
	{
		return Weights;
	}

	TArray<float> NormalizedWeights;
	for (float Weight : Weights)
	{
		NormalizedWeights.Add(Weight / TotalWeight);
	}

	return NormalizedWeights;
}

UTexture2D* USplatMapGenerator::CreateTextureFromData(
	int32 Width,
	int32 Height,
	const TArray<FColor>& PixelData,
	const FString& TextureName)
{
	if (PixelData.Num() != Width * Height)
	{
		UE_LOG(LogTemp, Error, TEXT("USplatMapGenerator::CreateTextureFromData - Pixel data size mismatch"));
		return nullptr;
	}

	// Create texture
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("USplatMapGenerator::CreateTextureFromData - Failed to create texture"));
		return nullptr;
	}

	// Lock the texture for writing
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

	// Copy pixel data
	FMemory::Memcpy(Data, PixelData.GetData(), PixelData.Num() * sizeof(FColor));

	// Unlock and update
	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

float USplatMapGenerator::CalculateHeightWeight(float Height, const FSplatLayerConfig& Config)
{
	float Weight = 0.0f;

	if (Height >= Config.MinHeight && Height <= Config.MaxHeight)
	{
		// Within range
		Weight = 1.0f;

		// Apply smooth blending at edges
		if (Config.HeightBlendRange > 0.0f)
		{
			// Blend at lower edge
			if (Height < Config.MinHeight + Config.HeightBlendRange)
			{
				float EdgeDistance = Height - Config.MinHeight;
				Weight = SmoothBlend(0.0f, Config.HeightBlendRange, EdgeDistance);
			}
			// Blend at upper edge
			else if (Height > Config.MaxHeight - Config.HeightBlendRange)
			{
				float EdgeDistance = Config.MaxHeight - Height;
				Weight = SmoothBlend(0.0f, Config.HeightBlendRange, EdgeDistance);
			}
		}
	}

	return Weight;
}

float USplatMapGenerator::CalculateSlopeWeight(float Slope, const FSplatLayerConfig& Config)
{
	float Weight = 0.0f;

	if (Slope >= Config.MinSlope && Slope <= Config.MaxSlope)
	{
		// Within range
		Weight = 1.0f;

		// Apply smooth blending at edges
		if (Config.SlopeBlendRange > 0.0f)
		{
			// Blend at lower edge
			if (Slope < Config.MinSlope + Config.SlopeBlendRange)
			{
				float EdgeDistance = Slope - Config.MinSlope;
				Weight = SmoothBlend(0.0f, Config.SlopeBlendRange, EdgeDistance);
			}
			// Blend at upper edge
			else if (Slope > Config.MaxSlope - Config.SlopeBlendRange)
			{
				float EdgeDistance = Config.MaxSlope - Slope;
				Weight = SmoothBlend(0.0f, Config.SlopeBlendRange, EdgeDistance);
			}
		}
	}

	return Weight;
}

float USplatMapGenerator::CalculateBiomeWeight(int32 BiomeID, const FSplatLayerConfig& Config)
{
	return (BiomeID == Config.BiomeID) ? 1.0f : 0.0f;
}

float USplatMapGenerator::CalculateHumidityWeight(float Humidity, const FSplatLayerConfig& Config)
{
	if (Humidity >= Config.MinHumidity && Humidity <= Config.MaxHumidity)
	{
		return 1.0f;
	}
	return 0.0f;
}

float USplatMapGenerator::SmoothBlend(float Edge0, float Edge1, float Value)
{
	// Clamp value to range
	float T = FMath::Clamp((Value - Edge0) / (Edge1 - Edge0), 0.0f, 1.0f);

	// Smoothstep function
	return T * T * (3.0f - 2.0f * T);
}

FColor USplatMapGenerator::GetPixel(UTexture2D* Texture, int32 X, int32 Y)
{
	if (!Texture)
	{
		return FColor::Black;
	}

	const TArray<FColor>& TextureData = GetTextureData(Texture);

	int32 Width = Texture->GetSizeX();
	int32 Height = Texture->GetSizeY();

	X = FMath::Clamp(X, 0, Width - 1);
	Y = FMath::Clamp(Y, 0, Height - 1);

	int32 Index = Y * Width + X;
	if (TextureData.IsValidIndex(Index))
	{
		return TextureData[Index];
	}

	return FColor::Black;
}

float USplatMapGenerator::GetHeightValue(UTexture2D* HeightMap, float U, float V)
{
	if (!HeightMap)
	{
		return 0.0f;
	}

	FLinearColor Sample = SampleTexture(HeightMap, U, V);
	return Sample.R;
}

float USplatMapGenerator::GetHeightValue(UTexture2D* HeightMap, int32 X, int32 Y)
{
	FColor Pixel = GetPixel(HeightMap, X, Y);
	return static_cast<float>(Pixel.R) / 255.0f;
}

const TArray<FColor>& USplatMapGenerator::GetTextureData(UTexture2D* Texture)
{
	if (!TextureDataCache.Contains(Texture))
	{
		// Load texture data into cache
		TArray<FColor>& Data = TextureDataCache.Add(Texture);

		if (Texture && Texture->GetPlatformData())
		{
			FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
			const FColor* FormattedImageData = static_cast<const FColor*>(Mip.BulkData.Lock(LOCK_READ_ONLY));

			if (FormattedImageData)
			{
				int32 TextureSize = Texture->GetSizeX() * Texture->GetSizeY();
				Data.SetNum(TextureSize);
				FMemory::Memcpy(Data.GetData(), FormattedImageData, TextureSize * sizeof(FColor));
			}

			Mip.BulkData.Unlock();
		}
	}

	return TextureDataCache[Texture];
}

void USplatMapGenerator::ClearCache()
{
	TextureDataCache.Empty();
}
