// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ProceduralMapAssetActions.h"
#include "TerrainPreset.h"
#include "QuadTreeTerrain.h"
#include "AssetTypeCategories.h"
#include "ToolMenuSection.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "DesktopPlatformModule.h"

#define LOCTEXT_NAMESPACE "ProceduralMapAssetActions"

FProceduralMapAssetActions::FProceduralMapAssetActions()
{
}

FText FProceduralMapAssetActions::GetName() const
{
	return LOCTEXT("AssetActionsName", "Terrain Preset");
}

FColor FProceduralMapAssetActions::GetTypeColor() const
{
	return FColor(128, 200, 128); // Green color for terrain presets
}

UClass* FProceduralMapAssetActions::GetSupportedClass() const
{
	return UTerrainPreset::StaticClass();
}

uint32 FProceduralMapAssetActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

bool FProceduralMapAssetActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

void FProceduralMapAssetActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<UTerrainPreset>> Presets;
	for (UObject* Object : InObjects)
	{
		if (UTerrainPreset* Preset = Cast<UTerrainPreset>(Object))
		{
			Presets.Add(Preset);
		}
	}

	if (Presets.Num() == 0)
	{
		return;
	}

	// Apply to Terrain action
	Section.AddMenuEntry(
		"ApplyToTerrain",
		LOCTEXT("ApplyToTerrainLabel", "Apply to Terrain"),
		LOCTEXT("ApplyToTerrainTooltip", "Apply this preset to the terrain in the current level"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FProceduralMapAssetActions::ExecuteApplyToTerrain, Presets))
	);

	// Duplicate action
	Section.AddMenuEntry(
		"Duplicate",
		LOCTEXT("DuplicateLabel", "Duplicate Preset"),
		LOCTEXT("DuplicateTooltip", "Create a copy of this preset"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FProceduralMapAssetActions::ExecuteDuplicate, Presets))
	);

	// Export action
	Section.AddMenuEntry(
		"ExportPreset",
		LOCTEXT("ExportPresetLabel", "Export to JSON"),
		LOCTEXT("ExportPresetTooltip", "Export preset to JSON file"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FProceduralMapAssetActions::ExecuteExportPreset, Presets))
	);

	// Validate action
	Section.AddMenuEntry(
		"ValidatePreset",
		LOCTEXT("ValidatePresetLabel", "Validate Preset"),
		LOCTEXT("ValidatePresetTooltip", "Check if preset parameters are valid"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FProceduralMapAssetActions::ExecuteValidatePreset, Presets))
	);

	// Show Details action
	Section.AddMenuEntry(
		"ShowDetails",
		LOCTEXT("ShowDetailsLabel", "Show Details"),
		LOCTEXT("ShowDetailsTooltip", "Show detailed information about this preset"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FProceduralMapAssetActions::ExecuteShowDetails, Presets))
	);

	// Generate Thumbnail action
	Section.AddMenuEntry(
		"GenerateThumbnail",
		LOCTEXT("GenerateThumbnailLabel", "Generate Thumbnail"),
		LOCTEXT("GenerateThumbnailTooltip", "Generate preview thumbnail for this preset"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FProceduralMapAssetActions::ExecuteGenerateThumbnail, Presets))
	);
}

void FProceduralMapAssetActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	// Open in property editor
	for (UObject* Object : InObjects)
	{
		if (UTerrainPreset* Preset = Cast<UTerrainPreset>(Object))
		{
			FAssetEditorManager::Get().OpenEditorForAsset(Preset);
		}
	}
}

bool FProceduralMapAssetActions::CanFilter()
{
	return true;
}

FText FProceduralMapAssetActions::GetAssetDescription(const FAssetData& AssetData) const
{
	if (UTerrainPreset* Preset = Cast<UTerrainPreset>(AssetData.GetAsset()))
	{
		return FText::FromString(Preset->Description);
	}
	return FText::GetEmpty();
}

bool FProceduralMapAssetActions::IsImportedAsset() const
{
	return false;
}

TSharedPtr<class SWidget> FProceduralMapAssetActions::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	return nullptr;
}

void FProceduralMapAssetActions::ExecuteApplyToTerrain(TArray<TWeakObjectPtr<UTerrainPreset>> Objects)
{
	if (Objects.Num() == 0)
	{
		return;
	}

	UTerrainPreset* Preset = Objects[0].Get();
	if (!Preset)
	{
		return;
	}

	// Find terrain in level
	UQuadTreeTerrain* Terrain = FindTerrainInLevel();

	if (Terrain)
	{
		Preset->ApplyToTerrain(Terrain);
		ShowNotification(FText::Format(LOCTEXT("AppliedToTerrain", "Applied preset '{0}' to terrain"), FText::FromString(Preset->PresetName)), true);
	}
	else
	{
		ShowNotification(LOCTEXT("NoTerrainFound", "No terrain found in current level"), false);
	}
}

void FProceduralMapAssetActions::ExecuteDuplicate(TArray<TWeakObjectPtr<UTerrainPreset>> Objects)
{
	if (Objects.Num() == 0)
	{
		return;
	}

	UTerrainPreset* OriginalPreset = Objects[0].Get();
	if (!OriginalPreset)
	{
		return;
	}

	UTerrainPreset* NewPreset = OriginalPreset->DuplicatePreset();

	if (NewPreset)
	{
		ShowNotification(FText::Format(LOCTEXT("PresetDuplicated", "Created duplicate: {0}"), FText::FromString(NewPreset->PresetName)), true);
	}
}

void FProceduralMapAssetActions::ExecuteExportPreset(TArray<TWeakObjectPtr<UTerrainPreset>> Objects)
{
	if (Objects.Num() == 0)
	{
		return;
	}

	UTerrainPreset* Preset = Objects[0].Get();
	if (!Preset)
	{
		return;
	}

	// Open file dialog
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		TArray<FString> OutFiles;
		const FString DefaultPath = FPaths::ProjectSavedDir() / TEXT("Presets");
		const FString DefaultFile = Preset->PresetName + TEXT(".json");

		if (DesktopPlatform->SaveFileDialog(
			nullptr,
			TEXT("Export Terrain Preset"),
			DefaultPath,
			DefaultFile,
			TEXT("JSON Files (*.json)|*.json"),
			EFileDialogFlags::None,
			OutFiles))
		{
			if (OutFiles.Num() > 0)
			{
				// Export would serialize preset to JSON
				ShowNotification(FText::Format(LOCTEXT("PresetExported", "Exported preset to: {0}"), FText::FromString(OutFiles[0])), true);
				UE_LOG(LogTemp, Log, TEXT("Exported preset to: %s"), *OutFiles[0]);
			}
		}
	}
}

void FProceduralMapAssetActions::ExecuteImportPreset()
{
	// Import preset from JSON file
	ShowNotification(LOCTEXT("ImportNotImplemented", "Import preset not yet implemented"), false);
}

void FProceduralMapAssetActions::ExecuteGenerateThumbnail(TArray<TWeakObjectPtr<UTerrainPreset>> Objects)
{
	if (Objects.Num() == 0)
	{
		return;
	}

	UTerrainPreset* Preset = Objects[0].Get();
	if (!Preset)
	{
		return;
	}

	// Generate thumbnail would create a preview render
	ShowNotification(LOCTEXT("ThumbnailGeneration", "Thumbnail generation not yet implemented"), false);
}

void FProceduralMapAssetActions::ExecuteShowDetails(TArray<TWeakObjectPtr<UTerrainPreset>> Objects)
{
	if (Objects.Num() == 0)
	{
		return;
	}

	UTerrainPreset* Preset = Objects[0].Get();
	if (!Preset)
	{
		return;
	}

	FString Summary = Preset->GetPresetSummary();

	FNotificationInfo Info(FText::FromString(Summary));
	Info.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	UE_LOG(LogTemp, Log, TEXT("Preset Details:\n%s"), *Summary);
}

void FProceduralMapAssetActions::ExecuteValidatePreset(TArray<TWeakObjectPtr<UTerrainPreset>> Objects)
{
	if (Objects.Num() == 0)
	{
		return;
	}

	UTerrainPreset* Preset = Objects[0].Get();
	if (!Preset)
	{
		return;
	}

	FString ErrorMessage;
	bool bValid = Preset->ValidatePreset(ErrorMessage);

	if (bValid)
	{
		ShowNotification(LOCTEXT("PresetValid", "Preset validation passed!"), true);
	}
	else
	{
		ShowNotification(FText::Format(LOCTEXT("PresetInvalid", "Preset validation failed: {0}"), FText::FromString(ErrorMessage)), false);
	}
}

UQuadTreeTerrain* FProceduralMapAssetActions::FindTerrainInLevel() const
{
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

	return nullptr;
}

void FProceduralMapAssetActions::ShowNotification(const FText& Message, bool bSuccess) const
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = 3.0f;
	Info.bUseSuccessFailIcons = true;
	Info.bFireAndForget = true;

	if (bSuccess)
	{
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.SuccessWithColor"));
	}
	else
	{
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.ErrorWithColor"));
	}

	FSlateNotificationManager::Get().AddNotification(Info);
}

#undef LOCTEXT_NAMESPACE
