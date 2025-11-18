// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "BiomeSettingsCustomization.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/SlateTypes.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "BiomeSettingsCustomization"

TSharedRef<IPropertyTypeCustomization> FBiomeSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FBiomeSettingsCustomization);
}

void FBiomeSettingsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	CachedPropertyHandle = PropertyHandle;

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("BiomeSettingsValue", "Biome Configuration"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	];
}

void FBiomeSettingsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Get child property handles
	MinTemperatureHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBiomeSettings, MinTemperature));
	MaxTemperatureHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBiomeSettings, MaxTemperature));
	MinHumidityHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBiomeSettings, MinHumidity));
	MaxHumidityHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBiomeSettings, MaxHumidity));
	MinElevationHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBiomeSettings, MinElevation));
	MaxElevationHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBiomeSettings, MaxElevation));
	FoliageSettingsHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBiomeSettings, FoliageSettings));

	// Climate Visualization Section
	ChildBuilder.AddCustomRow(LOCTEXT("ClimateVisualization", "Climate"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ClimateLabel", "Climate Visualization"))
		.Font(IDetailLayoutBuilder::GetDetailFontBold())
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	[
		CreateClimateVisualization()
	];

	// Temperature Range
	ChildBuilder.AddCustomRow(LOCTEXT("TemperatureRange", "Temperature"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("TemperatureLabel", "Temperature Range"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			CreateSliderWithInput(MinTemperatureHandle, 0.0f, 1.0f)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			CreateSliderWithInput(MaxTemperatureHandle, 0.0f, 1.0f)
		]
	];

	// Humidity Range
	ChildBuilder.AddCustomRow(LOCTEXT("HumidityRange", "Humidity"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("HumidityLabel", "Humidity Range"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			CreateSliderWithInput(MinHumidityHandle, 0.0f, 1.0f)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			CreateSliderWithInput(MaxHumidityHandle, 0.0f, 1.0f)
		]
	];

	// Elevation Range
	ChildBuilder.AddCustomRow(LOCTEXT("ElevationRange", "Elevation"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ElevationLabel", "Elevation Range"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			CreateSliderWithInput(MinElevationHandle, 0.0f, 1.0f)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			CreateSliderWithInput(MaxElevationHandle, 0.0f, 1.0f)
		]
	];

	// Add other properties
	uint32 NumChildren;
	PropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedRef<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		FName PropertyName = ChildHandle->GetProperty()->GetFName();

		// Skip properties we've already customized
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, MinTemperature) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, MaxTemperature) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, MinHumidity) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, MaxHumidity) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, MinElevation) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, MaxElevation))
		{
			continue;
		}

		// Add texture previews for texture properties
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, BaseColor) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, NormalMap) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, RoughnessMap) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FBiomeSettings, DisplacementMap))
		{
			ChildBuilder.AddCustomRow(FText::FromName(PropertyName))
			.NameContent()
			[
				ChildHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					ChildHandle->CreatePropertyValueWidget()
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4, 0, 0, 0)
				[
					CreateTexturePreview(ChildHandle)
				]
			];
		}
		else
		{
			// Add default property widget
			ChildBuilder.AddProperty(ChildHandle);
		}
	}
}

TSharedRef<SWidget> FBiomeSettingsCustomization::CreateTexturePreview(TSharedPtr<IPropertyHandle> TextureProperty)
{
	return SNew(SBox)
		.WidthOverride(64.0f)
		.HeightOverride(64.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TexturePreview", "Preview"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
				]
			]
		];
}

TSharedRef<SWidget> FBiomeSettingsCustomization::CreateColorPicker(TSharedPtr<IPropertyHandle> ColorProperty)
{
	return SNew(SBox)
		.WidthOverride(100.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ColorPicker", "Color"))
			]
		];
}

TSharedRef<SWidget> FBiomeSettingsCustomization::CreateSliderWithInput(TSharedPtr<IPropertyHandle> PropertyHandle, float MinValue, float MaxValue)
{
	if (!PropertyHandle.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0, 0, 4, 0)
		[
			SNew(SBox)
			.MinDesiredWidth(100.0f)
			[
				PropertyHandle->CreatePropertyValueWidget()
			]
		];
}

TSharedRef<SWidget> FBiomeSettingsCustomization::CreateClimateVisualization()
{
	return SNew(SBox)
		.HeightOverride(100.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ClimateVisualizationPlaceholder", "Climate Visualization\n(Temperature vs Humidity)"))
						.Justification(ETextJustify::Center)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					]
				]
			]
		];
}

FLinearColor FBiomeSettingsCustomization::GetClimateColor(float Temperature, float Humidity) const
{
	// Simple color mapping based on climate
	// Cold + Dry = Blue-Gray (Tundra)
	// Hot + Dry = Yellow-Orange (Desert)
	// Cold + Wet = White-Blue (Snow)
	// Hot + Wet = Green (Forest)

	float R = FMath::Lerp(0.0f, 1.0f, Temperature);
	float G = FMath::Lerp(0.0f, 1.0f, Humidity);
	float B = FMath::Lerp(1.0f, 0.0f, Temperature);

	return FLinearColor(R, G, B);
}

#undef LOCTEXT_NAMESPACE
