// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "TerrainPresetFactory.generated.h"

/**
 * UTerrainPresetFactory
 * Factory for creating UTerrainPreset assets in the Content Browser
 */
UCLASS()
class PROCEDURALMAPEDITOR_API UTerrainPresetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UTerrainPresetFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetToolTip() const override;
	virtual FString GetDefaultNewAssetName() const override;
	// End of UFactory interface
};
