// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeBlender.h"
#include "FastNoiseLite.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Texture2D.h"

ABiomeBlender::ABiomeBlender()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABiomeBlender::BeginPlay()
{
	Super::BeginPlay();

	if (!BiomeConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("BiomeBlender: No BiomeConfig asset assigned!"));
	}
}

void ABiomeBlender::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FBlendedBiomeResult ABiomeBlender::BlendBiomes(const TArray<FBiomeData>& BiomeData, FVector Position, FVector Normal)
{
	FBlendedBiomeResult Result;

	if (BiomeData.Num() == 0 || !BiomeConfig)
	{
		return Result;
	}

	// Store biome weights
	for (const FBiomeData& Data : BiomeData)
	{
		Result.BiomeWeights.Add(Data.BiomeType, Data.Weight);
	}

	TArray<FLinearColor> Colors;
	TArray<FVector> Normals;
	TArray<float> Weights;
	TArray<float> RoughnessValues;

	// Calculate triplanar weights for this position
	float TriplanarSharpness = 4.0f;
	if (BiomeConfig->TransitionSettings.bUseHeightBasedBlending)
	{
		TriplanarSharpness = BiomeConfig->TransitionSettings.BlendSharpness;
	}
	FTriplanarWeights TriplanarW = CalculateTriplanarWeights(Normal, TriplanarSharpness);

	// Collect colors, normals, and roughness from each biome
	for (const FBiomeData& Data : BiomeData)
	{
		if (!BiomeConfig->BiomeSettings.Contains(Data.BiomeType))
		{
			continue;
		}

		const FBiomeSettings& Settings = BiomeConfig->BiomeSettings[Data.BiomeType];
		float Weight = Data.Weight;

		// Sample base color using triplanar mapping
		FLinearColor BiomeColor = FLinearColor::White;
		if (Settings.BaseColor)
		{
			BiomeColor = SampleTriplanar(
				Settings.BaseColor,
				Position,
				Normal,
				Settings.TriplanarScale,
				Settings.TriplanarSharpness
			);
		}

		// Sample normal map
		FVector BiomeNormal = Normal;
		if (Settings.NormalMap)
		{
			FLinearColor NormalSample = SampleTriplanar(
				Settings.NormalMap,
				Position,
				Normal,
				Settings.TriplanarScale,
				Settings.TriplanarSharpness
			);
			// Convert normal map color to vector
			BiomeNormal = FVector(
				NormalSample.R * 2.0f - 1.0f,
				NormalSample.G * 2.0f - 1.0f,
				NormalSample.B * 2.0f - 1.0f
			).GetSafeNormal();
		}

		// Sample roughness
		float BiomeRoughness = 0.5f;
		if (Settings.RoughnessMap)
		{
			FLinearColor RoughnessSample = SampleTriplanar(
				Settings.RoughnessMap,
				Position,
				Normal,
				Settings.TriplanarScale,
				Settings.TriplanarSharpness
			);
			BiomeRoughness = RoughnessSample.R;
		}

		Colors.Add(BiomeColor);
		Normals.Add(BiomeNormal);
		Weights.Add(Weight);
		RoughnessValues.Add(BiomeRoughness);
	}

	// Apply noise to weights for organic transitions
	if (BiomeConfig)
	{
		float NoiseStrength = BiomeConfig->TransitionSettings.NoiseStrength;
		Weights = ApplyBlendNoise(Weights, Position, NoiseStrength);
	}

	// Blend results
	Result.BlendedColor = BlendColors(Colors, Weights);
	Result.BlendedNormal = BlendNormals(Normals, Weights);

	// Blend roughness
	Result.BlendedRoughness = 0.0f;
	for (int32 i = 0; i < RoughnessValues.Num(); i++)
	{
		Result.BlendedRoughness += RoughnessValues[i] * Weights[i];
	}

	return Result;
}

TArray<FBiomeData> ABiomeBlender::HeightBasedBlending(const TArray<FBiomeData>& BiomeData, float Height, float Slope)
{
	if (!BiomeConfig || !BiomeConfig->TransitionSettings.bUseHeightBasedBlending)
	{
		return BiomeData;
	}

	TArray<FBiomeData> ModifiedData = BiomeData;
	float HeightInfluence = BiomeConfig->TransitionSettings.HeightBlendInfluence;

	for (FBiomeData& Data : ModifiedData)
	{
		if (!BiomeConfig->BiomeSettings.Contains(Data.BiomeType))
		{
			continue;
		}

		const FBiomeSettings& Settings = BiomeConfig->BiomeSettings[Data.BiomeType];

		// Calculate height match
		float HeightMatch = 1.0f;
		if (Height < Settings.MinElevation)
		{
			HeightMatch = 1.0f - FMath::Clamp((Settings.MinElevation - Height) / 0.1f, 0.0f, 1.0f);
		}
		else if (Height > Settings.MaxElevation)
		{
			HeightMatch = 1.0f - FMath::Clamp((Height - Settings.MaxElevation) / 0.1f, 0.0f, 1.0f);
		}

		// Modify weight based on height match
		Data.Weight *= FMath::Lerp(1.0f, HeightMatch, HeightInfluence);

		// Slope influence (some biomes prefer flat or steep terrain)
		if (Data.BiomeType == EBiomeType::Mountains)
		{
			// Mountains favor steep slopes
			Data.Weight *= 1.0f + Slope * 0.5f;
		}
		else if (Data.BiomeType == EBiomeType::Beach || Data.BiomeType == EBiomeType::Plains)
		{
			// Flat biomes favor low slopes
			Data.Weight *= 1.0f + (1.0f - Slope) * 0.3f;
		}
	}

	// Renormalize weights
	float TotalWeight = 0.0f;
	for (const FBiomeData& Data : ModifiedData)
	{
		TotalWeight += Data.Weight;
	}

	if (TotalWeight > 0.0f)
	{
		for (FBiomeData& Data : ModifiedData)
		{
			Data.Weight /= TotalWeight;
		}
	}

	return ModifiedData;
}

float ABiomeBlender::CalculateTransitionMask(EBiomeType BiomeA, EBiomeType BiomeB, FVector Position, float BlendFactor)
{
	if (!BiomeConfig)
	{
		return BlendFactor;
	}

	const FBiomeTransitionSettings& TransSettings = BiomeConfig->TransitionSettings;

	// Add noise to transition
	float NoiseValue = GetBlendNoise(Position, TransSettings.NoiseScale);
	NoiseValue = (NoiseValue + 1.0f) * 0.5f; // Normalize to 0-1

	// Blend noise with base blend factor
	float Mask = FMath::Lerp(BlendFactor, NoiseValue, TransSettings.NoiseStrength);

	// Apply sharpness
	Mask = PowerCurve(Mask, TransSettings.BlendSharpness);

	return FMath::Clamp(Mask, 0.0f, 1.0f);
}

FTriplanarWeights ABiomeBlender::CalculateTriplanarWeights(FVector Normal, float Sharpness)
{
	FTriplanarWeights Weights;

	// Use absolute normal components
	FVector AbsNormal = FVector(
		FMath::Abs(Normal.X),
		FMath::Abs(Normal.Y),
		FMath::Abs(Normal.Z)
	);

	// Apply sharpness
	AbsNormal.X = FMath::Pow(AbsNormal.X, Sharpness);
	AbsNormal.Y = FMath::Pow(AbsNormal.Y, Sharpness);
	AbsNormal.Z = FMath::Pow(AbsNormal.Z, Sharpness);

	// Normalize
	float TotalWeight = AbsNormal.X + AbsNormal.Y + AbsNormal.Z;
	if (TotalWeight > 0.0f)
	{
		Weights.WeightX = AbsNormal.X / TotalWeight;
		Weights.WeightY = AbsNormal.Y / TotalWeight;
		Weights.WeightZ = AbsNormal.Z / TotalWeight;
	}

	return Weights;
}

FLinearColor ABiomeBlender::SampleTriplanar(UTexture2D* Texture, FVector Position, FVector Normal, float Scale, float Sharpness)
{
	if (!Texture)
	{
		return FLinearColor::White;
	}

	// Calculate triplanar weights
	FTriplanarWeights Weights = CalculateTriplanarWeights(Normal, Sharpness);

	// Calculate UV coordinates for each axis
	FVector2D UV_X = FVector2D(Position.Y, Position.Z) * Scale;
	FVector2D UV_Y = FVector2D(Position.X, Position.Z) * Scale;
	FVector2D UV_Z = FVector2D(Position.X, Position.Y) * Scale;

	// Sample texture for each axis
	FLinearColor ColorX = SampleTexture(Texture, UV_X);
	FLinearColor ColorY = SampleTexture(Texture, UV_Y);
	FLinearColor ColorZ = SampleTexture(Texture, UV_Z);

	// Blend using triplanar weights
	FLinearColor Result = ColorX * Weights.WeightX + ColorY * Weights.WeightY + ColorZ * Weights.WeightZ;

	return Result;
}

FLinearColor ABiomeBlender::BlendColors(const TArray<FLinearColor>& Colors, const TArray<float>& Weights)
{
	if (Colors.Num() == 0 || Colors.Num() != Weights.Num())
	{
		return FLinearColor::White;
	}

	FLinearColor Result = FLinearColor::Black;
	for (int32 i = 0; i < Colors.Num(); i++)
	{
		Result += Colors[i] * Weights[i];
	}

	return Result;
}

FVector ABiomeBlender::BlendNormals(const TArray<FVector>& Normals, const TArray<float>& Weights)
{
	if (Normals.Num() == 0 || Normals.Num() != Weights.Num())
	{
		return FVector::UpVector;
	}

	FVector Result = FVector::ZeroVector;
	for (int32 i = 0; i < Normals.Num(); i++)
	{
		Result += Normals[i] * Weights[i];
	}

	return Result.GetSafeNormal();
}

TArray<float> ABiomeBlender::ApplyBlendNoise(const TArray<float>& Weights, FVector Position, float NoiseStrength)
{
	if (Weights.Num() == 0 || NoiseStrength <= 0.0f)
	{
		return Weights;
	}

	TArray<float> ModifiedWeights = Weights;

	// Apply noise variation to each weight
	for (int32 i = 0; i < ModifiedWeights.Num(); i++)
	{
		float NoiseValue = GetBlendNoise(Position + FVector(i * 100.0f, 0, 0), 0.01f);
		NoiseValue = (NoiseValue + 1.0f) * 0.5f; // Normalize to 0-1

		// Modify weight
		float Variation = (NoiseValue - 0.5f) * NoiseStrength;
		ModifiedWeights[i] = FMath::Clamp(ModifiedWeights[i] + Variation, 0.0f, 1.0f);
	}

	// Renormalize
	NormalizeWeights(ModifiedWeights);

	return ModifiedWeights;
}

float ABiomeBlender::CalculateSlope(FVector Normal) const
{
	// Slope is 0 for flat terrain (normal pointing up), 1 for vertical
	float DotUp = FVector::DotProduct(Normal.GetSafeNormal(), FVector::UpVector);
	return 1.0f - FMath::Abs(DotUp);
}

float ABiomeBlender::SmoothStep(float Edge0, float Edge1, float X) const
{
	float T = FMath::Clamp((X - Edge0) / (Edge1 - Edge0), 0.0f, 1.0f);
	return T * T * (3.0f - 2.0f * T);
}

float ABiomeBlender::PowerCurve(float Value, float Power) const
{
	return FMath::Pow(FMath::Clamp(Value, 0.0f, 1.0f), Power);
}

float ABiomeBlender::GetBlendNoise(FVector Position, float Scale) const
{
	FastNoiseLite Noise;
	Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise.SetFrequency(Scale);

	if (BiomeConfig)
	{
		Noise.SetFractalOctaves(BiomeConfig->TransitionSettings.NoiseOctaves);
	}

	return Noise.GetNoise(Position.X, Position.Y);
}

FLinearColor ABiomeBlender::SampleTexture(UTexture2D* Texture, FVector2D UV) const
{
	if (!Texture)
	{
		return FLinearColor::White;
	}

	// Wrap UV coordinates
	UV.X = FMath::Fmod(UV.X, 1.0f);
	UV.Y = FMath::Fmod(UV.Y, 1.0f);
	if (UV.X < 0.0f) UV.X += 1.0f;
	if (UV.Y < 0.0f) UV.Y += 1.0f;

	// Note: In a real implementation, you would use texture sampling methods
	// This is a simplified placeholder that returns a default color
	// In actual UE5 usage, texture sampling would be done in material shaders
	return FLinearColor::White;
}

void ABiomeBlender::NormalizeWeights(TArray<float>& Weights) const
{
	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += Weight;
	}

	if (TotalWeight > 0.0f)
	{
		for (float& Weight : Weights)
		{
			Weight /= TotalWeight;
		}
	}
}
