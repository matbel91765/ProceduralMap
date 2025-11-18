// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

class SWidget;
class STerrainPreviewWidget;

/**
 * FNoiseParametersCustomization
 * Custom property editor UI for FTerrainGenerationParams struct
 * Provides interactive sliders with real-time preview
 */
class PROCEDURALMAPEDITOR_API FNoiseParametersCustomization : public IPropertyTypeCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	// IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	// End of IPropertyTypeCustomization interface

private:
	/** Create randomize seed button */
	TSharedRef<SWidget> CreateRandomizeSeedButton();

	/** Create slider with numeric input */
	TSharedRef<SWidget> CreateSliderWithNumericInput(TSharedPtr<IPropertyHandle> PropertyHandle, float MinValue, float MaxValue, bool bIntValue = false);

	/** Create noise preview widget */
	TSharedRef<SWidget> CreateNoisePreview();

	/** Handle randomize seed button clicked */
	FReply OnRandomizeSeedClicked();

	/** Handle property value changed */
	void OnPropertyChanged();

	/** Get current seed value */
	TOptional<int32> GetSeedValue() const;

	/** Set seed value */
	void SetSeedValue(int32 NewValue);

	/** Get current frequency value */
	TOptional<float> GetFrequencyValue() const;

	/** Set frequency value */
	void SetFrequencyValue(float NewValue);

	/** Property handles */
	TSharedPtr<IPropertyHandle> SeedHandle;
	TSharedPtr<IPropertyHandle> FrequencyHandle;
	TSharedPtr<IPropertyHandle> OctavesHandle;
	TSharedPtr<IPropertyHandle> LacunarityHandle;
	TSharedPtr<IPropertyHandle> GainHandle;
	TSharedPtr<IPropertyHandle> HeightScaleHandle;
	TSharedPtr<IPropertyHandle> HeightOffsetHandle;

	/** Cached property handle */
	TSharedPtr<IPropertyHandle> CachedPropertyHandle;

	/** Preview widget */
	TSharedPtr<STerrainPreviewWidget> PreviewWidget;

	/** Enable real-time preview */
	bool bEnableRealtimePreview;
};
