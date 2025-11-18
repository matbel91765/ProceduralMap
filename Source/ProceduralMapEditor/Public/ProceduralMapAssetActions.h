// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * FProceduralMapAssetActions
 * Custom asset actions for ProceduralMap assets in Content Browser
 * Provides context menu actions like Apply, Duplicate, Export, etc.
 */
class PROCEDURALMAPEDITOR_API FProceduralMapAssetActions : public FAssetTypeActions_Base
{
public:
	FProceduralMapAssetActions();

	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual bool CanFilter() override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	virtual bool IsImportedAsset() const override;
	virtual TSharedPtr<class SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;
	// End of IAssetTypeActions interface

private:
	/** Apply preset to current terrain */
	void ExecuteApplyToTerrain(TArray<TWeakObjectPtr<class UTerrainPreset>> Objects);

	/** Duplicate preset */
	void ExecuteDuplicate(TArray<TWeakObjectPtr<class UTerrainPreset>> Objects);

	/** Export preset to file */
	void ExecuteExportPreset(TArray<TWeakObjectPtr<class UTerrainPreset>> Objects);

	/** Import preset from file */
	void ExecuteImportPreset();

	/** Generate preview thumbnail */
	void ExecuteGenerateThumbnail(TArray<TWeakObjectPtr<class UTerrainPreset>> Objects);

	/** Show preset details */
	void ExecuteShowDetails(TArray<TWeakObjectPtr<class UTerrainPreset>> Objects);

	/** Validate preset */
	void ExecuteValidatePreset(TArray<TWeakObjectPtr<class UTerrainPreset>> Objects);

	/** Find terrain in current level */
	class UQuadTreeTerrain* FindTerrainInLevel() const;

	/** Show notification */
	void ShowNotification(const FText& Message, bool bSuccess) const;
};
