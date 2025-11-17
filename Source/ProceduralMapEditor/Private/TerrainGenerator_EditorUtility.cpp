// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "TerrainGenerator_EditorUtility.h"
#include "ProceduralMapSettings.h"
#include "TerrainPreset.h"
#include "QuadTreeTerrain.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UTerrainGenerator_EditorUtility::UTerrainGenerator_EditorUtility()
	: CurrentTerrainActor(nullptr)
	, bIsGenerating(false)
	, GenerationProgress(0.0f)
	, LastPreviewUpdateTime(0.0)
{
	// Initialize with default parameters
	GenerationParams = FTerrainGenerationParams();
	CurrentBiomeType = EBiomeType::Plains;
	bEnableErosion = true;
	ErosionIterations = 1000;
	TerrainSizeMultiplier = 1.0f;

	// Preview settings
	bShowPreview = true;
	bAutoUpdatePreview = true;
	PreviewResolution = 256;

	CurrentPresetName = TEXT("MyPreset");
}

void UTerrainGenerator_EditorUtility::NativeConstruct()
{
	Super::NativeConstruct();

	// Load available presets
	RefreshPresetsList();

	// Apply default parameters from settings
	if (UProceduralMapSettings* Settings = UProceduralMapSettings::Get())
	{
		GenerationParams = Settings->DefaultGenerationParams;
		bEnableErosion = Settings->bEnableErosionByDefault;
		ErosionIterations = Settings->DefaultErosionIterations;
	}

	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator_EditorUtility: Widget constructed"));
}

void UTerrainGenerator_EditorUtility::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update preview if needed
	if (bAutoUpdatePreview && bShowPreview)
	{
		double CurrentTime = FPlatformTime::Seconds();
		if (CurrentTime - LastPreviewUpdateTime > 0.5) // Update every 0.5 seconds
		{
			UpdatePreview();
			LastPreviewUpdateTime = CurrentTime;
		}
	}
}

void UTerrainGenerator_EditorUtility::GenerateTerrain()
{
	if (bIsGenerating)
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: Generation already in progress"));
		return;
	}

	if (!ValidateParameters())
	{
		OnGenerationFailed(TEXT("Invalid parameters"));
		return;
	}

	bIsGenerating = true;
	GenerationProgress = 0.0f;
	OnGenerationStarted();

	LogGenerationInfo();

	// Find or create terrain actor
	CurrentTerrainActor = FindOrCreateTerrainActor();

	if (CurrentTerrainActor)
	{
		// Apply parameters
		CurrentTerrainActor->GenerationParams = GenerationParams;
		CurrentTerrainActor->TerrainSize *= TerrainSizeMultiplier;

		// Generate terrain
		CurrentTerrainActor->RegenerateTerrain();

		bIsGenerating = false;
		GenerationProgress = 1.0f;
		OnGenerationCompleted();

		UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Generation completed successfully"));
	}
	else
	{
		bIsGenerating = false;
		OnGenerationFailed(TEXT("Failed to find or create terrain actor"));
		UE_LOG(LogTemp, Error, TEXT("TerrainGenerator: Failed to find or create terrain actor"));
	}
}

void UTerrainGenerator_EditorUtility::ClearTerrain()
{
	if (CurrentTerrainActor)
	{
		CurrentTerrainActor->CleanupTerrain();
		UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Terrain cleared"));
	}
}

void UTerrainGenerator_EditorUtility::RegenerateTerrain()
{
	ClearTerrain();
	GenerateTerrain();
}

void UTerrainGenerator_EditorUtility::ExportHeightmap(const FString& FilePath)
{
	UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: Export heightmap not yet implemented"));
	// Implementation would save heightmap data to file (PNG, RAW, etc.)
}

void UTerrainGenerator_EditorUtility::ImportHeightmap(const FString& FilePath)
{
	UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: Import heightmap not yet implemented"));
	// Implementation would load heightmap data from file
}

void UTerrainGenerator_EditorUtility::RandomizeSeed()
{
	GenerationParams.Seed = FMath::RandRange(1, 999999999);
	OnParametersChanged();

	if (bAutoUpdatePreview)
	{
		UpdatePreview();
	}

	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Randomized seed to %d"), GenerationParams.Seed);
}

void UTerrainGenerator_EditorUtility::UpdatePreview()
{
	// Preview update would trigger preview widget refresh
	// This is a simplified version
	UE_LOG(LogTemp, Verbose, TEXT("TerrainGenerator: Preview updated"));
}

bool UTerrainGenerator_EditorUtility::SavePreset(const FString& PresetName, const FString& Description)
{
	if (PresetName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: Preset name cannot be empty"));
		return false;
	}

	// Create new preset asset
	UTerrainPreset* NewPreset = NewObject<UTerrainPreset>(GetTransientPackage(), UTerrainPreset::StaticClass());
	if (!NewPreset)
	{
		return false;
	}

	// Set preset data
	NewPreset->PresetName = PresetName;
	NewPreset->Description = Description;
	NewPreset->GenerationParams = GenerationParams;
	NewPreset->BiomeType = CurrentBiomeType;
	NewPreset->bEnableErosion = bEnableErosion;
	NewPreset->ErosionIterations = ErosionIterations;

	// Get settings for save path
	UProceduralMapSettings* Settings = UProceduralMapSettings::Get();
	FString SavePath = Settings ? Settings->AssetPaths.PresetsPath.Path : TEXT("/Game/ProceduralMap/Presets");
	FString PackageName = FString::Printf(TEXT("%s/%s"), *SavePath, *PresetName);

	// Create package
	UPackage* Package = CreatePackage(*PackageName);
	NewPreset->Rename(*PresetName, Package, REN_DontCreateRedirectors);

	// Mark package as dirty
	Package->MarkPackageDirty();

	// Save package
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

	bool bSuccess = UPackage::SavePackage(Package, NewPreset, *PackageFileName, SaveArgs);

	if (bSuccess)
	{
		OnPresetSaved(PresetName);
		RefreshPresetsList();
		UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Saved preset '%s'"), *PresetName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainGenerator: Failed to save preset '%s'"), *PresetName);
	}

	return bSuccess;
}

bool UTerrainGenerator_EditorUtility::LoadPreset(const FString& PresetName)
{
	for (const FTerrainPresetInfo& PresetInfo : AvailablePresets)
	{
		if (PresetInfo.PresetName == PresetName && PresetInfo.Preset)
		{
			LoadPresetFromAsset(PresetInfo.Preset);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: Preset '%s' not found"), *PresetName);
	return false;
}

void UTerrainGenerator_EditorUtility::LoadPresetFromAsset(UTerrainPreset* Preset)
{
	if (!Preset)
	{
		return;
	}

	GenerationParams = Preset->GenerationParams;
	CurrentBiomeType = Preset->BiomeType;
	bEnableErosion = Preset->bEnableErosion;
	ErosionIterations = Preset->ErosionIterations;

	CurrentPreset = Preset;
	CurrentPresetName = Preset->PresetName;

	OnPresetLoaded(Preset->PresetName);
	OnParametersChanged();

	if (bAutoUpdatePreview)
	{
		UpdatePreview();
	}

	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Loaded preset '%s'"), *Preset->PresetName);
}

bool UTerrainGenerator_EditorUtility::DeletePreset(const FString& PresetName)
{
	// Find preset
	for (const FTerrainPresetInfo& PresetInfo : AvailablePresets)
	{
		if (PresetInfo.PresetName == PresetName && PresetInfo.Preset)
		{
			// Delete asset
			// Note: This is simplified - proper implementation would use AssetTools
			UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: Delete preset not fully implemented"));
			return false;
		}
	}

	return false;
}

void UTerrainGenerator_EditorUtility::RefreshPresetsList()
{
	AvailablePresets.Empty();

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Find all UTerrainPreset assets
	TArray<FAssetData> PresetAssets;
	AssetRegistry.GetAssetsByClass(UTerrainPreset::StaticClass()->GetClassPathName(), PresetAssets);

	// Add to available presets
	for (const FAssetData& AssetData : PresetAssets)
	{
		UTerrainPreset* Preset = Cast<UTerrainPreset>(AssetData.GetAsset());
		if (Preset)
		{
			FTerrainPresetInfo Info;
			Info.PresetName = Preset->PresetName;
			Info.Description = Preset->Description;
			Info.Preset = Preset;
			AvailablePresets.Add(Info);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Refreshed presets list - found %d presets"), AvailablePresets.Num());
}

TArray<FString> UTerrainGenerator_EditorUtility::GetPresetNames() const
{
	TArray<FString> Names;
	for (const FTerrainPresetInfo& Info : AvailablePresets)
	{
		Names.Add(Info.PresetName);
	}
	return Names;
}

void UTerrainGenerator_EditorUtility::ApplyDefaultParameters()
{
	if (UProceduralMapSettings* Settings = UProceduralMapSettings::Get())
	{
		GenerationParams = Settings->DefaultGenerationParams;
		bEnableErosion = Settings->bEnableErosionByDefault;
		ErosionIterations = Settings->DefaultErosionIterations;
	}
	else
	{
		GenerationParams = FTerrainGenerationParams();
		bEnableErosion = true;
		ErosionIterations = 1000;
	}

	OnParametersChanged();
	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Applied default parameters"));
}

void UTerrainGenerator_EditorUtility::ApplyPlainsPreset()
{
	GenerationParams.Frequency = 0.003f;
	GenerationParams.Octaves = 4;
	GenerationParams.Lacunarity = 2.0f;
	GenerationParams.Gain = 0.5f;
	GenerationParams.HeightScale = 200.0f;
	GenerationParams.HeightOffset = 0.0f;

	CurrentBiomeType = EBiomeType::Plains;
	bEnableErosion = true;
	ErosionIterations = 500;

	OnParametersChanged();
	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Applied Plains preset"));
}

void UTerrainGenerator_EditorUtility::ApplyMountainsPreset()
{
	GenerationParams.Frequency = 0.002f;
	GenerationParams.Octaves = 8;
	GenerationParams.Lacunarity = 2.5f;
	GenerationParams.Gain = 0.6f;
	GenerationParams.HeightScale = 1500.0f;
	GenerationParams.HeightOffset = 0.0f;

	CurrentBiomeType = EBiomeType::Mountains;
	bEnableErosion = true;
	ErosionIterations = 2000;

	OnParametersChanged();
	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Applied Mountains preset"));
}

void UTerrainGenerator_EditorUtility::ApplyDesertPreset()
{
	GenerationParams.Frequency = 0.004f;
	GenerationParams.Octaves = 3;
	GenerationParams.Lacunarity = 1.8f;
	GenerationParams.Gain = 0.4f;
	GenerationParams.HeightScale = 150.0f;
	GenerationParams.HeightOffset = 0.0f;

	CurrentBiomeType = EBiomeType::Desert;
	bEnableErosion = false;
	ErosionIterations = 0;

	OnParametersChanged();
	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Applied Desert preset"));
}

void UTerrainGenerator_EditorUtility::ApplyIslandsPreset()
{
	GenerationParams.Frequency = 0.005f;
	GenerationParams.Octaves = 6;
	GenerationParams.Lacunarity = 2.2f;
	GenerationParams.Gain = 0.55f;
	GenerationParams.HeightScale = 600.0f;
	GenerationParams.HeightOffset = -200.0f;

	CurrentBiomeType = EBiomeType::Beach;
	bEnableErosion = true;
	ErosionIterations = 1000;

	OnParametersChanged();
	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Applied Islands preset"));
}

UQuadTreeTerrain* UTerrainGenerator_EditorUtility::GetCurrentTerrainActor()
{
	if (!CurrentTerrainActor)
	{
		CurrentTerrainActor = FindOrCreateTerrainActor();
	}
	return CurrentTerrainActor;
}

float UTerrainGenerator_EditorUtility::GetGenerationProgress() const
{
	return GenerationProgress;
}

bool UTerrainGenerator_EditorUtility::IsGenerating() const
{
	return bIsGenerating;
}

void UTerrainGenerator_EditorUtility::CancelGeneration()
{
	if (bIsGenerating)
	{
		bIsGenerating = false;
		GenerationProgress = 0.0f;
		UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Generation cancelled"));
	}
}

UQuadTreeTerrain* UTerrainGenerator_EditorUtility::FindOrCreateTerrainActor()
{
	// Find existing terrain in level
	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		for (TActorIterator<AActor> It(GEditor->GetEditorWorldContext().World()); It; ++It)
		{
			UQuadTreeTerrain* TerrainComponent = It->FindComponentByClass<UQuadTreeTerrain>();
			if (TerrainComponent)
			{
				return TerrainComponent;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: No terrain actor found in level"));
	return nullptr;
}

void UTerrainGenerator_EditorUtility::LoadAvailablePresets()
{
	RefreshPresetsList();
}

void UTerrainGenerator_EditorUtility::CreateDefaultPresets()
{
	// Create default presets if they don't exist
	// This is a simplified version
}

bool UTerrainGenerator_EditorUtility::ValidateParameters() const
{
	// Validate generation parameters
	if (GenerationParams.Frequency <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainGenerator: Invalid frequency"));
		return false;
	}

	if (GenerationParams.Octaves < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainGenerator: Invalid octaves"));
		return false;
	}

	return true;
}

void UTerrainGenerator_EditorUtility::LogGenerationInfo() const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Starting generation"));
	UE_LOG(LogTemp, Log, TEXT("Seed: %d"), GenerationParams.Seed);
	UE_LOG(LogTemp, Log, TEXT("Frequency: %.4f"), GenerationParams.Frequency);
	UE_LOG(LogTemp, Log, TEXT("Octaves: %d"), GenerationParams.Octaves);
	UE_LOG(LogTemp, Log, TEXT("Lacunarity: %.2f"), GenerationParams.Lacunarity);
	UE_LOG(LogTemp, Log, TEXT("Gain: %.2f"), GenerationParams.Gain);
	UE_LOG(LogTemp, Log, TEXT("Height Scale: %.2f"), GenerationParams.HeightScale);
	UE_LOG(LogTemp, Log, TEXT("Erosion: %s (%d iterations)"), bEnableErosion ? TEXT("Enabled") : TEXT("Disabled"), ErosionIterations);
	UE_LOG(LogTemp, Log, TEXT("========================================"));
}
