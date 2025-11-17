// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "NoiseParametersCustomization.h"
#include "TerrainPreviewWidget.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/SlateTypes.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "NoiseParametersCustomization"

TSharedRef<IPropertyTypeCustomization> FNoiseParametersCustomization::MakeInstance()
{
	return MakeShareable(new FNoiseParametersCustomization);
}

void FNoiseParametersCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	CachedPropertyHandle = PropertyHandle;
	bEnableRealtimePreview = true;

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("NoiseParametersValue", "Noise Configuration"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	];
}

void FNoiseParametersCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Get child property handles
	SeedHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTerrainGenerationParams, Seed));
	FrequencyHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTerrainGenerationParams, Frequency));
	OctavesHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTerrainGenerationParams, Octaves));
	LacunarityHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTerrainGenerationParams, Lacunarity));
	GainHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTerrainGenerationParams, Gain));
	HeightScaleHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTerrainGenerationParams, HeightScale));
	HeightOffsetHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTerrainGenerationParams, HeightOffset));

	// Preview Section
	ChildBuilder.AddCustomRow(LOCTEXT("NoisePreview", "Preview"))
	.WholeRowContent()
	[
		CreateNoisePreview()
	];

	// Seed with randomize button
	ChildBuilder.AddCustomRow(LOCTEXT("Seed", "Seed"))
	.NameContent()
	[
		SeedHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0, 0, 4, 0)
		[
			SNew(SNumericEntryBox<int32>)
			.Value(this, &FNoiseParametersCustomization::GetSeedValue)
			.OnValueChanged(this, &FNoiseParametersCustomization::SetSeedValue)
			.AllowSpin(true)
			.MinValue(1)
			.MaxValue(999999999)
			.MinSliderValue(1)
			.MaxSliderValue(999999999)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			CreateRandomizeSeedButton()
		]
	];

	// Frequency slider
	ChildBuilder.AddCustomRow(LOCTEXT("Frequency", "Frequency"))
	.NameContent()
	[
		FrequencyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		CreateSliderWithNumericInput(FrequencyHandle, 0.0001f, 0.1f, false)
	];

	// Octaves slider
	ChildBuilder.AddCustomRow(LOCTEXT("Octaves", "Octaves"))
	.NameContent()
	[
		OctavesHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		CreateSliderWithNumericInput(OctavesHandle, 1.0f, 16.0f, true)
	];

	// Lacunarity slider
	ChildBuilder.AddCustomRow(LOCTEXT("Lacunarity", "Lacunarity"))
	.NameContent()
	[
		LacunarityHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		CreateSliderWithNumericInput(LacunarityHandle, 1.0f, 4.0f, false)
	];

	// Gain slider
	ChildBuilder.AddCustomRow(LOCTEXT("Gain", "Gain"))
	.NameContent()
	[
		GainHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		CreateSliderWithNumericInput(GainHandle, 0.0f, 1.0f, false)
	];

	// Height Scale slider
	ChildBuilder.AddCustomRow(LOCTEXT("HeightScale", "Height Scale"))
	.NameContent()
	[
		HeightScaleHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		CreateSliderWithNumericInput(HeightScaleHandle, 0.0f, 5000.0f, false)
	];

	// Height Offset slider
	ChildBuilder.AddCustomRow(LOCTEXT("HeightOffset", "Height Offset"))
	.NameContent()
	[
		HeightOffsetHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		CreateSliderWithNumericInput(HeightOffsetHandle, -1000.0f, 1000.0f, false)
	];
}

TSharedRef<SWidget> FNoiseParametersCustomization::CreateRandomizeSeedButton()
{
	return SNew(SButton)
		.Text(LOCTEXT("Randomize", "Random"))
		.ToolTipText(LOCTEXT("RandomizeTooltip", "Generate a random seed value"))
		.OnClicked(this, &FNoiseParametersCustomization::OnRandomizeSeedClicked)
		.ContentPadding(FMargin(4, 2));
}

TSharedRef<SWidget> FNoiseParametersCustomization::CreateSliderWithNumericInput(TSharedPtr<IPropertyHandle> PropertyHandle, float MinValue, float MaxValue, bool bIntValue)
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
			PropertyHandle->CreatePropertyValueWidget()
		];
}

TSharedRef<SWidget> FNoiseParametersCustomization::CreateNoisePreview()
{
	return SNew(SBox)
		.HeightOverride(200.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SNew(SVerticalBox)

				// Preview header
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 4)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NoisePreviewLabel", "Noise Preview"))
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				]

				// Preview content
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PreviewPlaceholder", "Real-time noise preview\n(Updates automatically when parameters change)"))
						.Justification(ETextJustify::Center)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					]
				]
			]
		];
}

FReply FNoiseParametersCustomization::OnRandomizeSeedClicked()
{
	if (SeedHandle.IsValid())
	{
		int32 RandomSeed = FMath::RandRange(1, 999999999);
		SeedHandle->SetValue(RandomSeed);
		OnPropertyChanged();
	}

	return FReply::Handled();
}

void FNoiseParametersCustomization::OnPropertyChanged()
{
	// Trigger preview update
	if (bEnableRealtimePreview && PreviewWidget.IsValid())
	{
		// Update preview
	}
}

TOptional<int32> FNoiseParametersCustomization::GetSeedValue() const
{
	if (SeedHandle.IsValid())
	{
		int32 Value;
		if (SeedHandle->GetValue(Value) == FPropertyAccess::Success)
		{
			return Value;
		}
	}
	return TOptional<int32>();
}

void FNoiseParametersCustomization::SetSeedValue(int32 NewValue)
{
	if (SeedHandle.IsValid())
	{
		SeedHandle->SetValue(NewValue);
		OnPropertyChanged();
	}
}

TOptional<float> FNoiseParametersCustomization::GetFrequencyValue() const
{
	if (FrequencyHandle.IsValid())
	{
		float Value;
		if (FrequencyHandle->GetValue(Value) == FPropertyAccess::Success)
		{
			return Value;
		}
	}
	return TOptional<float>();
}

void FNoiseParametersCustomization::SetFrequencyValue(float NewValue)
{
	if (FrequencyHandle.IsValid())
	{
		FrequencyHandle->SetValue(NewValue);
		OnPropertyChanged();
	}
}

#undef LOCTEXT_NAMESPACE
