// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "TerrainPreviewWidget.h"
#include "Widgets/SViewport.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/SlateTypes.h"
#include "AdvancedPreviewScene.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Engine/Texture2D.h"
#include "Rendering/Texture2DResource.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "TerrainPreviewWidget"

void STerrainPreviewWidget::Construct(const FArguments& InArgs)
{
	PreviewResolution = InArgs._PreviewResolution;
	bShow3DPreview = InArgs._Show3DPreview;
	bShow2DPreview = InArgs._Show2DPreview;
	bShowStatistics = InArgs._ShowStatistics;

	CameraRotation = FRotator(-45.0f, -45.0f, 0.0f);
	CameraDistance = 2000.0f;
	bIsDragging = false;
	bNeedsUpdate = false;
	LastUpdateTime = 0.0;
	UpdateThrottleTime = 0.1f;

	// Initialize height data
	HeightData.SetNum(PreviewResolution * PreviewResolution);
	for (int32 i = 0; i < HeightData.Num(); i++)
	{
		HeightData[i] = 0.0f;
	}

	// Initialize color gradient
	InitializeColorGradient();

	// Create layout
	ChildSlot
	[
		SNew(SVerticalBox)

		// Main preview area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// 3D Preview
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				SNew(SBox)
				.Visibility(bShow3DPreview ? EVisibility::Visible : EVisibility::Collapsed)
				[
					Create3DPreviewViewport()
				]
			]

			// 2D Heightmap Preview
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				SNew(SBox)
				.Visibility(bShow2DPreview ? EVisibility::Visible : EVisibility::Collapsed)
				[
					Create2DHeightmapPreview()
				]
			]
		]

		// Statistics Panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.Visibility(bShowStatistics ? EVisibility::Visible : EVisibility::Collapsed)
			.HeightOverride(120.0f)
			[
				CreateStatisticsPanel()
			]
		]
	];

	// Calculate initial statistics
	CalculateStatistics();
}

STerrainPreviewWidget::~STerrainPreviewWidget()
{
	// Cleanup
	PreviewScene.Reset();
	PreviewViewport.Reset();
}

void STerrainPreviewWidget::UpdatePreview(const FTerrainGenerationParams& Params, const TArray<float>& InHeightData)
{
	CurrentParams = Params;

	if (InHeightData.Num() == HeightData.Num())
	{
		HeightData = InHeightData;
	}
	else
	{
		// Resize if needed
		HeightData = InHeightData;
	}

	bNeedsUpdate = true;
}

void STerrainPreviewWidget::RefreshPreview()
{
	double CurrentTime = FPlatformTime::Seconds();

	if (CurrentTime - LastUpdateTime < UpdateThrottleTime)
	{
		return;
	}

	LastUpdateTime = CurrentTime;

	// Generate preview mesh
	if (bShow3DPreview)
	{
		GeneratePreviewMesh();
	}

	// Generate heightmap texture
	if (bShow2DPreview)
	{
		GenerateHeightmapTexture();
	}

	// Calculate statistics
	CalculateStatistics();
	UpdateStatisticsText();

	bNeedsUpdate = false;
}

void STerrainPreviewWidget::SetShow3DPreview(bool bShow)
{
	bShow3DPreview = bShow;
}

void STerrainPreviewWidget::SetShow2DPreview(bool bShow)
{
	bShow2DPreview = bShow;
}

void STerrainPreviewWidget::SetShowStatistics(bool bShow)
{
	bShowStatistics = bShow;
}

void STerrainPreviewWidget::SetPreviewResolution(int32 Resolution)
{
	PreviewResolution = FMath::Clamp(Resolution, 32, 1024);
	HeightData.SetNum(PreviewResolution * PreviewResolution);
	bNeedsUpdate = true;
}

void STerrainPreviewWidget::RotatePreview(float DeltaYaw, float DeltaPitch)
{
	CameraRotation.Yaw += DeltaYaw;
	CameraRotation.Pitch += DeltaPitch;
	CameraRotation.Pitch = FMath::Clamp(CameraRotation.Pitch, -89.0f, 89.0f);
}

void STerrainPreviewWidget::ResetCamera()
{
	CameraRotation = FRotator(-45.0f, -45.0f, 0.0f);
	CameraDistance = 2000.0f;
}

void STerrainPreviewWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bNeedsUpdate)
	{
		RefreshPreview();
	}
}

int32 STerrainPreviewWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// Draw 2D heightmap if visible
	if (bShow2DPreview)
	{
		Draw2DHeightmap(AllottedGeometry, OutDrawElements, LayerId);
		DrawGridOverlay(AllottedGeometry, OutDrawElements, LayerId + 1);
	}

	return LayerId + 2;
}

FReply STerrainPreviewWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		LastMousePosition = MouseEvent.GetScreenSpacePosition();
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	return FReply::Unhandled();
}

FReply STerrainPreviewWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		bIsDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

FReply STerrainPreviewWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bIsDragging)
	{
		FVector2D CurrentMousePosition = MouseEvent.GetScreenSpacePosition();
		FVector2D Delta = CurrentMousePosition - LastMousePosition;

		RotatePreview(Delta.X * 0.5f, Delta.Y * 0.5f);

		LastMousePosition = CurrentMousePosition;
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply STerrainPreviewWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	float WheelDelta = MouseEvent.GetWheelDelta();
	CameraDistance = FMath::Clamp(CameraDistance - WheelDelta * 100.0f, 500.0f, 10000.0f);

	return FReply::Handled();
}

TSharedRef<SWidget> STerrainPreviewWidget::Create3DPreviewViewport()
{
	return SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("3DPreview", "3D Preview (Implementation requires viewport setup)"))
			.Justification(ETextJustify::Center)
		];
}

TSharedRef<SWidget> STerrainPreviewWidget::Create2DHeightmapPreview()
{
	return SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SOverlay)

			// Background
			+ SOverlay::Slot()
			[
				SNew(SColorBlock)
				.Color(FLinearColor(0.02f, 0.02f, 0.02f))
			]

			// Heightmap rendering happens in OnPaint
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HeightmapPreview", "2D Heightmap Preview"))
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f))
			]
		];
}

TSharedRef<SWidget> STerrainPreviewWidget::CreateStatisticsPanel()
{
	return SNew(SScrollBox)
		.Orientation(Orient_Vertical)

		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Statistics", "Terrain Statistics"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			// Stats grid
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5.0f)
			[
				SNew(SHorizontalBox)

				// Column 1
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(VerticesText, STextBlock)
						.Text(LOCTEXT("Vertices", "Vertices: 0"))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(TrianglesText, STextBlock)
						.Text(LOCTEXT("Triangles", "Triangles: 0"))
					]
				]

				// Column 2
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(MinHeightText, STextBlock)
						.Text(LOCTEXT("MinHeight", "Min Height: 0.0"))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(MaxHeightText, STextBlock)
						.Text(LOCTEXT("MaxHeight", "Max Height: 0.0"))
					]
				]

				// Column 3
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(AvgHeightText, STextBlock)
						.Text(LOCTEXT("AvgHeight", "Avg Height: 0.0"))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(MemoryText, STextBlock)
						.Text(LOCTEXT("Memory", "Memory: 0 KB"))
					]
				]
			]
		];
}

void STerrainPreviewWidget::GeneratePreviewMesh()
{
	// This would generate actual mesh geometry from height data
	// Implementation requires creating procedural mesh
	double StartTime = FPlatformTime::Seconds();

	// Placeholder for mesh generation
	Statistics.GenerationTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
}

void STerrainPreviewWidget::GenerateHeightmapTexture()
{
	// This would generate a texture from height data
	// Implementation requires creating UTexture2D and updating pixel data
}

void STerrainPreviewWidget::CalculateStatistics()
{
	if (HeightData.Num() == 0)
	{
		return;
	}

	Statistics.NumVertices = PreviewResolution * PreviewResolution;
	Statistics.NumTriangles = (PreviewResolution - 1) * (PreviewResolution - 1) * 2;
	Statistics.MemoryUsageBytes = Statistics.NumVertices * sizeof(FVector) * 3; // Position, Normal, UV

	float MinH = FLT_MAX;
	float MaxH = -FLT_MAX;
	float Sum = 0.0f;

	for (float Height : HeightData)
	{
		MinH = FMath::Min(MinH, Height);
		MaxH = FMath::Max(MaxH, Height);
		Sum += Height;
	}

	Statistics.MinHeight = MinH;
	Statistics.MaxHeight = MaxH;
	Statistics.AvgHeight = Sum / HeightData.Num();
}

FLinearColor STerrainPreviewWidget::GetHeightColor(float Height) const
{
	// Normalize height to 0-1 range
	float NormalizedHeight = FMath::Clamp(Height, 0.0f, 1.0f);

	// Sample from color gradient
	int32 GradientIndex = FMath::FloorToInt(NormalizedHeight * (HeightColorGradient.Num() - 1));
	GradientIndex = FMath::Clamp(GradientIndex, 0, HeightColorGradient.Num() - 1);

	return HeightColorGradient[GradientIndex];
}

FColor STerrainPreviewWidget::HeightToColor(float Height) const
{
	return GetHeightColor(Height).ToFColor(false);
}

void STerrainPreviewWidget::Draw2DHeightmap(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	// This would draw the actual heightmap using Slate drawing
	// Implementation requires iterating through height data and drawing colored rectangles
}

void STerrainPreviewWidget::DrawGridOverlay(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	// Draw grid lines over heightmap
	const int32 GridSize = 8;
	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const float CellWidth = Size.X / GridSize;
	const float CellHeight = Size.Y / GridSize;

	TArray<FVector2D> LinePoints;

	// Vertical lines
	for (int32 i = 0; i <= GridSize; i++)
	{
		float X = i * CellWidth;
		LinePoints.Add(FVector2D(X, 0.0f));
		LinePoints.Add(FVector2D(X, Size.Y));
	}

	// Horizontal lines
	for (int32 i = 0; i <= GridSize; i++)
	{
		float Y = i * CellHeight;
		LinePoints.Add(FVector2D(0.0f, Y));
		LinePoints.Add(FVector2D(Size.X, Y));
	}

	// Draw lines
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		LinePoints,
		ESlateDrawEffect::None,
		FLinearColor(0.3f, 0.3f, 0.3f, 0.5f),
		false,
		1.0f
	);
}

void STerrainPreviewWidget::InitializeColorGradient()
{
	HeightColorGradient.Empty();

	// Create a nice terrain color gradient
	HeightColorGradient.Add(FLinearColor(0.0f, 0.0f, 0.5f));      // Deep water (blue)
	HeightColorGradient.Add(FLinearColor(0.0f, 0.3f, 0.8f));      // Shallow water
	HeightColorGradient.Add(FLinearColor(0.8f, 0.7f, 0.4f));      // Beach (sand)
	HeightColorGradient.Add(FLinearColor(0.2f, 0.6f, 0.2f));      // Grass (green)
	HeightColorGradient.Add(FLinearColor(0.3f, 0.5f, 0.2f));      // Forest
	HeightColorGradient.Add(FLinearColor(0.5f, 0.5f, 0.5f));      // Mountain (gray)
	HeightColorGradient.Add(FLinearColor(0.9f, 0.9f, 0.9f));      // Snow (white)
}

FString STerrainPreviewWidget::GetFormattedMemoryString(int32 Bytes) const
{
	if (Bytes < 1024)
	{
		return FString::Printf(TEXT("%d B"), Bytes);
	}
	else if (Bytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f KB"), Bytes / 1024.0f);
	}
	else
	{
		return FString::Printf(TEXT("%.2f MB"), Bytes / (1024.0f * 1024.0f));
	}
}

FString STerrainPreviewWidget::GetFormattedNumberString(int32 Number) const
{
	FString NumberString = FString::FromInt(Number);
	FString Result;

	int32 Count = 0;
	for (int32 i = NumberString.Len() - 1; i >= 0; i--)
	{
		if (Count > 0 && Count % 3 == 0)
		{
			Result.InsertAt(0, TEXT(","));
		}
		Result.InsertAt(0, FString::Chr(NumberString[i]));
		Count++;
	}

	return Result;
}

void STerrainPreviewWidget::UpdateStatisticsText()
{
	if (VerticesText.IsValid())
	{
		VerticesText->SetText(FText::FromString(FString::Printf(TEXT("Vertices: %s"),
			*GetFormattedNumberString(Statistics.NumVertices))));
	}

	if (TrianglesText.IsValid())
	{
		TrianglesText->SetText(FText::FromString(FString::Printf(TEXT("Triangles: %s"),
			*GetFormattedNumberString(Statistics.NumTriangles))));
	}

	if (MemoryText.IsValid())
	{
		MemoryText->SetText(FText::FromString(FString::Printf(TEXT("Memory: %s"),
			*GetFormattedMemoryString(Statistics.MemoryUsageBytes))));
	}

	if (MinHeightText.IsValid())
	{
		MinHeightText->SetText(FText::FromString(FString::Printf(TEXT("Min Height: %.2f"),
			Statistics.MinHeight)));
	}

	if (MaxHeightText.IsValid())
	{
		MaxHeightText->SetText(FText::FromString(FString::Printf(TEXT("Max Height: %.2f"),
			Statistics.MaxHeight)));
	}

	if (AvgHeightText.IsValid())
	{
		AvgHeightText->SetText(FText::FromString(FString::Printf(TEXT("Avg Height: %.2f"),
			Statistics.AvgHeight)));
	}
}

#undef LOCTEXT_NAMESPACE
