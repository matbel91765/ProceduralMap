// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

class SWidget;

/**
 * FBiomeSettingsCustomization
 * Custom property editor UI for FBiomeSettings struct
 * Provides enhanced visualization and interactive controls
 */
class PROCEDURALMAPEDITOR_API FBiomeSettingsCustomization : public IPropertyTypeCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	// IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	// End of IPropertyTypeCustomization interface

private:
	/** Create texture preview widget */
	TSharedRef<SWidget> CreateTexturePreview(TSharedPtr<IPropertyHandle> TextureProperty);

	/** Create color picker widget */
	TSharedRef<SWidget> CreateColorPicker(TSharedPtr<IPropertyHandle> ColorProperty);

	/** Create slider with text input */
	TSharedRef<SWidget> CreateSliderWithInput(TSharedPtr<IPropertyHandle> PropertyHandle, float MinValue, float MaxValue);

	/** Create temperature/humidity visualization */
	TSharedRef<SWidget> CreateClimateVisualization();

	/** Get climate color based on temperature and humidity */
	FLinearColor GetClimateColor(float Temperature, float Humidity) const;

	/** Property handles for climate data */
	TSharedPtr<IPropertyHandle> MinTemperatureHandle;
	TSharedPtr<IPropertyHandle> MaxTemperatureHandle;
	TSharedPtr<IPropertyHandle> MinHumidityHandle;
	TSharedPtr<IPropertyHandle> MaxHumidityHandle;
	TSharedPtr<IPropertyHandle> MinElevationHandle;
	TSharedPtr<IPropertyHandle> MaxElevationHandle;

	/** Property handle for foliage settings */
	TSharedPtr<IPropertyHandle> FoliageSettingsHandle;

	/** Cached property handle */
	TSharedPtr<IPropertyHandle> CachedPropertyHandle;
};
