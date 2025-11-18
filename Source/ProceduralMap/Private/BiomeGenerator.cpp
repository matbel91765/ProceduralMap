// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeGenerator.h"
#include "FastNoiseLite.h"
#include "Math/UnrealMathUtility.h"

ABiomeGenerator::ABiomeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	// Default world parameters
	WorldSizeX = 10000.0f;
	WorldSizeY = 10000.0f;
	MaxElevation = 1000.0f;

	// Default temperature noise parameters
	TemperatureNoise.Frequency = 0.005f;
	TemperatureNoise.Octaves = 3;
	TemperatureNoise.Lacunarity = 2.0f;
	TemperatureNoise.Persistence = 0.5f;
	TemperatureNoise.Seed = 1337;

	// Default humidity noise parameters
	HumidityNoise.Frequency = 0.008f;
	HumidityNoise.Octaves = 4;
	HumidityNoise.Lacunarity = 2.0f;
	HumidityNoise.Persistence = 0.5f;
	HumidityNoise.Seed = 7331;

	TemperatureScale = 1.0f;
	HumidityScale = 1.0f;
}

void ABiomeGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (!BiomeConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("BiomeGenerator: No BiomeConfig asset assigned!"));
	}
}

void ABiomeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ABiomeGenerator::CalculateTemperature(FVector Position, float Elevation)
{
	// Calculate latitude factor (0 at equator, 1 at poles)
	float LatitudeFactor = GetLatitudeFactor(Position);

	// Base temperature decreases with latitude
	float BaseTemperature = 1.0f - LatitudeFactor;

	// Temperature decreases with elevation (lapse rate: ~6.5°C per 1000m)
	float ElevationFactor = Elevation * 0.7f; // Reduce by 70% at max elevation
	BaseTemperature -= ElevationFactor;

	// Add noise variation
	float NoiseValue = GenerateNoise(Position.X, Position.Y, TemperatureNoise);
	NoiseValue = (NoiseValue + 1.0f) * 0.5f; // Normalize to 0-1

	float Temperature = BaseTemperature + (NoiseValue - 0.5f) * 0.3f * TemperatureScale;

	// Clamp to valid range
	return FMath::Clamp(Temperature, 0.0f, 1.0f);
}

float ABiomeGenerator::CalculateHumidity(FVector Position, float DistanceToWater)
{
	// Base humidity decreases with distance to water
	float DistanceFactor = FMath::Clamp(DistanceToWater / 5000.0f, 0.0f, 1.0f);
	float BaseHumidity = 1.0f - DistanceFactor;

	// Add multi-octave noise for variation
	float NoiseValue = GenerateNoise(Position.X, Position.Y, HumidityNoise);
	NoiseValue = (NoiseValue + 1.0f) * 0.5f; // Normalize to 0-1

	float Humidity = BaseHumidity * 0.6f + NoiseValue * 0.4f * HumidityScale;

	// Clamp to valid range
	return FMath::Clamp(Humidity, 0.0f, 1.0f);
}

EBiomeType ABiomeGenerator::GetBiomeFromClimate(float Temperature, float Humidity, float Elevation)
{
	// Whittaker diagram implementation
	// Biome determination based on temperature, humidity, and elevation

	// Special case: High elevation = Mountains or Snow
	if (Elevation > 0.7f)
	{
		if (Temperature < 0.3f)
		{
			return EBiomeType::Snow;
		}
		return EBiomeType::Mountains;
	}

	// Very low elevation near water = Beach
	if (Elevation < 0.15f)
	{
		return EBiomeType::Beach;
	}

	// Cold biomes (Temperature < 0.3)
	if (Temperature < 0.3f)
	{
		if (Humidity < 0.3f)
		{
			return EBiomeType::Tundra;
		}
		return EBiomeType::Snow;
	}

	// Cool biomes (Temperature 0.3 - 0.5)
	if (Temperature < 0.5f)
	{
		if (Humidity > 0.6f)
		{
			return EBiomeType::Forest;
		}
		return EBiomeType::Tundra;
	}

	// Temperate biomes (Temperature 0.5 - 0.7)
	if (Temperature < 0.7f)
	{
		if (Humidity > 0.7f)
		{
			return EBiomeType::Swamp;
		}
		if (Humidity > 0.4f)
		{
			return EBiomeType::Forest;
		}
		return EBiomeType::Plains;
	}

	// Hot biomes (Temperature > 0.7)
	if (Humidity < 0.3f)
	{
		return EBiomeType::Desert;
	}
	if (Humidity > 0.7f)
	{
		return EBiomeType::Swamp;
	}
	if (Humidity > 0.4f)
	{
		return EBiomeType::Forest;
	}

	return EBiomeType::Plains;
}

TArray<FBiomeData> ABiomeGenerator::CalculateBiomeWeights(FVector Position, float Elevation, float DistanceToWater)
{
	TArray<FBiomeData> BiomeWeights;

	// Calculate climate parameters
	float Temperature = CalculateTemperature(Position, Elevation);
	float Humidity = CalculateHumidity(Position, DistanceToWater);

	// Get primary biome
	EBiomeType PrimaryBiome = GetBiomeFromClimate(Temperature, Humidity, Elevation);

	if (!BiomeConfig)
	{
		// Return only primary biome if no config
		BiomeWeights.Add(FBiomeData(PrimaryBiome, 1.0f, Temperature, Humidity));
		return BiomeWeights;
	}

	// Calculate match scores for all biomes
	for (const auto& BiomePair : BiomeConfig->BiomeSettings)
	{
		EBiomeType BiomeType = BiomePair.Key;
		const FBiomeSettings& Settings = BiomePair.Value;

		float MatchScore = CalculateBiomeMatchScore(Temperature, Humidity, Elevation, Settings);

		if (MatchScore > 0.01f) // Only include biomes with significant influence
		{
			BiomeWeights.Add(FBiomeData(BiomeType, MatchScore, Temperature, Humidity));
		}
	}

	// Normalize weights
	NormalizeWeights(BiomeWeights);

	// Sort by weight (descending)
	BiomeWeights.Sort([](const FBiomeData& A, const FBiomeData& B) {
		return A.Weight > B.Weight;
	});

	// Keep only top 3 biomes for performance
	if (BiomeWeights.Num() > 3)
	{
		BiomeWeights.SetNum(3);
		NormalizeWeights(BiomeWeights);
	}

	return BiomeWeights;
}

float ABiomeGenerator::GenerateNoise(float X, float Y, const FNoiseParameters& Params)
{
	FastNoiseLite Noise;
	Noise.SetSeed(Params.Seed);
	Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise.SetFrequency(Params.Frequency);
	Noise.SetFractalType(FastNoiseLite::FractalType_FBm);
	Noise.SetFractalOctaves(Params.Octaves);
	Noise.SetFractalLacunarity(Params.Lacunarity);
	Noise.SetFractalGain(Params.Persistence);

	return Noise.GetNoise(X, Y);
}

FBiomeSettings ABiomeGenerator::GetBiomeSettings(EBiomeType BiomeType)
{
	if (BiomeConfig && BiomeConfig->BiomeSettings.Contains(BiomeType))
	{
		return BiomeConfig->BiomeSettings[BiomeType];
	}

	// Return default settings if not found
	return FBiomeSettings();
}

float ABiomeGenerator::GetLatitudeFactor(FVector Position) const
{
	// Calculate normalized position in world (0-1)
	float NormalizedY = (Position.Y + WorldSizeY * 0.5f) / WorldSizeY;
	NormalizedY = FMath::Clamp(NormalizedY, 0.0f, 1.0f);

	// Convert to latitude factor (0 at equator, 1 at poles)
	// Using abs to make it symmetric around equator
	return FMath::Abs(NormalizedY - 0.5f) * 2.0f;
}

float ABiomeGenerator::GetNoiseValue(float X, float Y, int32 Seed) const
{
	FastNoiseLite Noise;
	Noise.SetSeed(Seed);
	Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	return Noise.GetNoise(X, Y);
}

float ABiomeGenerator::SmoothStep(float Edge0, float Edge1, float X) const
{
	float T = FMath::Clamp((X - Edge0) / (Edge1 - Edge0), 0.0f, 1.0f);
	return T * T * (3.0f - 2.0f * T);
}

float ABiomeGenerator::CalculateBiomeMatchScore(float Temperature, float Humidity, float Elevation, const FBiomeSettings& Settings) const
{
	// Calculate how well climate parameters match biome requirements

	// Temperature match
	float TempMatch = 1.0f;
	if (Temperature < Settings.MinTemperature)
	{
		TempMatch = 1.0f - FMath::Clamp((Settings.MinTemperature - Temperature) / 0.2f, 0.0f, 1.0f);
	}
	else if (Temperature > Settings.MaxTemperature)
	{
		TempMatch = 1.0f - FMath::Clamp((Temperature - Settings.MaxTemperature) / 0.2f, 0.0f, 1.0f);
	}

	// Humidity match
	float HumidityMatch = 1.0f;
	if (Humidity < Settings.MinHumidity)
	{
		HumidityMatch = 1.0f - FMath::Clamp((Settings.MinHumidity - Humidity) / 0.2f, 0.0f, 1.0f);
	}
	else if (Humidity > Settings.MaxHumidity)
	{
		HumidityMatch = 1.0f - FMath::Clamp((Humidity - Settings.MaxHumidity) / 0.2f, 0.0f, 1.0f);
	}

	// Elevation match
	float ElevationMatch = 1.0f;
	if (Elevation < Settings.MinElevation)
	{
		ElevationMatch = 1.0f - FMath::Clamp((Settings.MinElevation - Elevation) / 0.2f, 0.0f, 1.0f);
	}
	else if (Elevation > Settings.MaxElevation)
	{
		ElevationMatch = 1.0f - FMath::Clamp((Elevation - Settings.MaxElevation) / 0.2f, 0.0f, 1.0f);
	}

	// Combined match score (weighted product)
	float MatchScore = TempMatch * HumidityMatch * ElevationMatch;

	// Apply smooth falloff
	MatchScore = FMath::Pow(MatchScore, 2.0f);

	return MatchScore;
}

void ABiomeGenerator::NormalizeWeights(TArray<FBiomeData>& BiomeData) const
{
	float TotalWeight = 0.0f;
	for (const FBiomeData& Data : BiomeData)
	{
		TotalWeight += Data.Weight;
	}

	if (TotalWeight > 0.0f)
	{
		for (FBiomeData& Data : BiomeData)
		{
			Data.Weight /= TotalWeight;
		}
	}
}
