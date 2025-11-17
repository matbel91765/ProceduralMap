// VegetationLayer.cpp - Vegetation Layer Data Asset
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#include "VegetationLayer.h"
#include "Kismet/KismetMathLibrary.h"

UVegetationLayer::UVegetationLayer()
{
	LayerName = TEXT("VegetationLayer");
	LayerType = EVegetationLayerType::Trees;
	bEnabled = true;
	LayerPriority = 0;
	LayerDensityMultiplier = 1.0f;
	MinInterSpeciesDistance = 100.0f;
	bEnableSpeciesCompetition = false;
}

int32 UVegetationLayer::GetRandomVegetationType(FRandomStream& RandomStream) const
{
	if (VegetationTypes.Num() == 0)
	{
		return INDEX_NONE;
	}

	// Get total spawn weight
	float TotalWeight = GetTotalSpawnWeight();
	if (TotalWeight <= 0.0f)
	{
		// If no weights, use random selection
		return RandomStream.RandRange(0, VegetationTypes.Num() - 1);
	}

	// Weighted random selection
	float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	for (int32 i = 0; i < VegetationTypes.Num(); ++i)
	{
		AccumulatedWeight += VegetationTypes[i].SpawnWeight;
		if (RandomValue <= AccumulatedWeight)
		{
			return i;
		}
	}

	// Fallback to last type
	return VegetationTypes.Num() - 1;
}

float UVegetationLayer::GetTotalSpawnWeight() const
{
	// Use cached value if available
	if (CachedTotalSpawnWeight >= 0.0f)
	{
		return CachedTotalSpawnWeight;
	}

	RecalculateCache();
	return CachedTotalSpawnWeight;
}

int32 UVegetationLayer::GetVegetationTypeByName(FName TypeName) const
{
	for (int32 i = 0; i < VegetationTypes.Num(); ++i)
	{
		if (VegetationTypes[i].TypeName == TypeName)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

float UVegetationLayer::CalculateEffectiveDensity(int32 TypeIndex, const FVector& Location, float BaseDensity) const
{
	if (!VegetationTypes.IsValidIndex(TypeIndex))
	{
		return 0.0f;
	}

	const FVegetationTypeConfig& VegType = VegetationTypes[TypeIndex];

	// Start with base density
	float EffectiveDensity = BaseDensity * VegType.BaseDensity * LayerDensityMultiplier;

	// Apply density variation (noise-based)
	if (VegType.DensityVariation > 0.0f)
	{
		// Simple noise calculation (in production, use FastNoiseLite)
		float NoiseValue = FMath::PerlinNoise2D(FVector2D(Location.X * 0.01f, Location.Y * 0.01f));
		NoiseValue = (NoiseValue + 1.0f) * 0.5f; // Normalize to 0-1

		float VariationFactor = 1.0f + (NoiseValue - 0.5f) * VegType.DensityVariation * 2.0f;
		EffectiveDensity *= FMath::Clamp(VariationFactor, 0.0f, 2.0f);
	}

	// Apply slope preference if curve is set
	if (VegType.SlopePreferenceCurve)
	{
		// Would need terrain normal here - placeholder for now
		// float SlopeAngle = CalculateSlopeAtLocation(Location);
		// float SlopeMultiplier = VegType.SlopePreferenceCurve->GetFloatValue(SlopeAngle / 90.0f);
		// EffectiveDensity *= SlopeMultiplier;
	}

	// Apply water proximity if curve is set
	if (VegType.WaterProximityCurve)
	{
		// Would need water distance calculation here - placeholder
		// float WaterDistance = CalculateWaterDistance(Location);
		// float WaterMultiplier = VegType.WaterProximityCurve->GetFloatValue(WaterDistance);
		// EffectiveDensity *= WaterMultiplier;
	}

	return FMath::Max(0.0f, EffectiveDensity);
}

bool UVegetationLayer::CanSpawnAtLocation(int32 TypeIndex, const FVector& Location, const FVector& Normal) const
{
	if (!VegetationTypes.IsValidIndex(TypeIndex))
	{
		return false;
	}

	const FVegetationTypeConfig& VegType = VegetationTypes[TypeIndex];

	// Check height constraints
	if (Location.Z < VegType.MinHeight || Location.Z > VegType.MaxHeight)
	{
		return false;
	}

	// Check slope constraints
	float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
	if (SlopeAngle < VegType.MinSlope || SlopeAngle > VegType.MaxSlope)
	{
		return false;
	}

	// Check biome restrictions
	if (VegType.AllowedBiomes.Num() > 0)
	{
		// Would need biome checking here
		// FName CurrentBiome = GetBiomeAtLocation(Location);
		// if (!VegType.AllowedBiomes.Contains(CurrentBiome))
		// {
		//     return false;
		// }
	}

	// Check water distance constraints
	if (VegType.MinDistanceFromWater > 0.0f || VegType.MaxDistanceFromWater > 0.0f)
	{
		// Would need water distance calculation here
		// float WaterDistance = CalculateWaterDistance(Location);
		// if (VegType.MinDistanceFromWater > 0.0f && WaterDistance < VegType.MinDistanceFromWater)
		// {
		//     return false;
		// }
		// if (VegType.MaxDistanceFromWater > 0.0f && WaterDistance > VegType.MaxDistanceFromWater)
		// {
		//     return false;
		// }
	}

	return true;
}

TArray<int32> UVegetationLayer::GetEnabledVegetationTypes() const
{
	TArray<int32> EnabledTypes;

	for (int32 i = 0; i < VegetationTypes.Num(); ++i)
	{
		// All types are considered enabled if they have a valid mesh
		if (VegetationTypes[i].StaticMesh != nullptr)
		{
			EnabledTypes.Add(i);
		}
	}

	return EnabledTypes;
}

TArray<int32> UVegetationLayer::GetVegetationTypesForBiome(FName BiomeName) const
{
	TArray<int32> ValidTypes;

	for (int32 i = 0; i < VegetationTypes.Num(); ++i)
	{
		const FVegetationTypeConfig& VegType = VegetationTypes[i];

		// If no biome restrictions, or if biome is in allowed list
		if (VegType.AllowedBiomes.Num() == 0 || VegType.AllowedBiomes.Contains(BiomeName))
		{
			ValidTypes.Add(i);
		}
	}

	return ValidTypes;
}

bool UVegetationLayer::ValidateConfiguration(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();

	// Check layer configuration
	if (LayerName.IsNone())
	{
		OutErrors.Add(TEXT("Layer name is not set"));
	}

	if (VegetationTypes.Num() == 0)
	{
		OutErrors.Add(TEXT("No vegetation types configured"));
		return false;
	}

	// Check each vegetation type
	for (int32 i = 0; i < VegetationTypes.Num(); ++i)
	{
		const FVegetationTypeConfig& VegType = VegetationTypes[i];

		if (VegType.StaticMesh == nullptr)
		{
			OutErrors.Add(FString::Printf(TEXT("Vegetation type %d has no static mesh"), i));
		}

		if (VegType.TypeName.IsNone())
		{
			OutErrors.Add(FString::Printf(TEXT("Vegetation type %d has no name"), i));
		}

		if (VegType.BaseDensity < 0.0f)
		{
			OutErrors.Add(FString::Printf(TEXT("Vegetation type %d has negative density"), i));
		}

		if (VegType.MinScale > VegType.MaxScale)
		{
			OutErrors.Add(FString::Printf(TEXT("Vegetation type %d: MinScale > MaxScale"), i));
		}

		if (VegType.MinHeight > VegType.MaxHeight)
		{
			OutErrors.Add(FString::Printf(TEXT("Vegetation type %d: MinHeight > MaxHeight"), i));
		}

		if (VegType.MinSlope > VegType.MaxSlope)
		{
			OutErrors.Add(FString::Printf(TEXT("Vegetation type %d: MinSlope > MaxSlope"), i));
		}

		if (VegType.MinimumSpacing < 1.0f)
		{
			OutErrors.Add(FString::Printf(TEXT("Vegetation type %d: MinimumSpacing too small (< 1cm)"), i));
		}
	}

	// Check for duplicate names
	TSet<FName> UsedNames;
	for (int32 i = 0; i < VegetationTypes.Num(); ++i)
	{
		if (!VegetationTypes[i].TypeName.IsNone())
		{
			if (UsedNames.Contains(VegetationTypes[i].TypeName))
			{
				OutErrors.Add(FString::Printf(TEXT("Duplicate vegetation type name: %s"), *VegetationTypes[i].TypeName.ToString()));
			}
			else
			{
				UsedNames.Add(VegetationTypes[i].TypeName);
			}
		}
	}

	// Check species competition settings
	if (bEnableSpeciesCompetition)
	{
		for (int32 DominantIndex : DominantSpeciesIndices)
		{
			if (!VegetationTypes.IsValidIndex(DominantIndex))
			{
				OutErrors.Add(FString::Printf(TEXT("Invalid dominant species index: %d"), DominantIndex));
			}
		}
	}

	return OutErrors.Num() == 0;
}

#if WITH_EDITOR
void UVegetationLayer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Invalidate cache when properties change
	CachedTotalSpawnWeight = -1.0f;

	// Validate configuration
	TArray<FString> Errors;
	if (!ValidateConfiguration(Errors))
	{
		for (const FString& Error : Errors)
		{
			UE_LOG(LogTemp, Warning, TEXT("VegetationLayer validation error: %s"), *Error);
		}
	}
}
#endif

void UVegetationLayer::RecalculateCache() const
{
	CachedTotalSpawnWeight = 0.0f;

	for (const FVegetationTypeConfig& VegType : VegetationTypes)
	{
		CachedTotalSpawnWeight += VegType.SpawnWeight;
	}
}
