// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "TerrainGenerationTask.h"
#include "Rendering/DrawElements.h"

class SViewport;
class FAdvancedPreviewScene;
class UStaticMeshComponent;
class UTexture2D;

/**
 * STerrainPreviewWidget
 * Professional Slate widget for real-time 3D and 2D terrain preview
 * Features:
 * - Interactive 3D preview with rotation
 * - 2D heightmap visualization with color mapping
 * - Real-time updates when parameters change
 * - Performance statistics display
 */
class PROCEDURALMAPEDITOR_API STerrainPreviewWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STerrainPreviewWidget)
		: _PreviewResolution(256)
		, _ShowStatistics(true)
		, _Show3DPreview(true)
		, _Show2DPreview(true)
		{}

		/** Preview resolution (heightmap size) */
		SLATE_ARGUMENT(int32, PreviewResolution)

		/** Show statistics panel */
		SLATE_ARGUMENT(bool, ShowStatistics)

		/** Show 3D preview */
		SLATE_ARGUMENT(bool, Show3DPreview)

		/** Show 2D preview (heightmap) */
		SLATE_ARGUMENT(bool, Show2DPreview)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual ~STerrainPreviewWidget();

	/** Update preview with new terrain parameters */
	void UpdatePreview(const FTerrainGenerationParams& Params, const TArray<float>& HeightData);

	/** Refresh the preview display */
	void RefreshPreview();

	/** Set whether to show 3D preview */
	void SetShow3DPreview(bool bShow);

	/** Set whether to show 2D preview */
	void SetShow2DPreview(bool bShow);

	/** Set whether to show statistics */
	void SetShowStatistics(bool bShow);

	/** Get current preview resolution */
	int32 GetPreviewResolution() const { return PreviewResolution; }

	/** Set preview resolution */
	void SetPreviewResolution(int32 Resolution);

	/** Rotate 3D preview */
	void RotatePreview(float DeltaYaw, float DeltaPitch);

	/** Reset 3D preview camera */
	void ResetCamera();

	/** Get statistics */
	struct FTerrainStats
	{
		int32 NumVertices;
		int32 NumTriangles;
		int32 MemoryUsageBytes;
		float MinHeight;
		float MaxHeight;
		float AvgHeight;
		float GenerationTime;

		FTerrainStats()
			: NumVertices(0)
			, NumTriangles(0)
			, MemoryUsageBytes(0)
			, MinHeight(0.0f)
			, MaxHeight(0.0f)
			, AvgHeight(0.0f)
			, GenerationTime(0.0f)
		{}
	};

	const FTerrainStats& GetStatistics() const { return Statistics; }

protected:
	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End of SWidget interface

private:
	/** Create 3D preview viewport */
	TSharedRef<SWidget> Create3DPreviewViewport();

	/** Create 2D heightmap preview */
	TSharedRef<SWidget> Create2DHeightmapPreview();

	/** Create statistics panel */
	TSharedRef<SWidget> CreateStatisticsPanel();

	/** Generate preview mesh from height data */
	void GeneratePreviewMesh();

	/** Generate heightmap texture from height data */
	void GenerateHeightmapTexture();

	/** Calculate statistics from height data */
	void CalculateStatistics();

	/** Update heightmap colors based on height */
	FLinearColor GetHeightColor(float Height) const;

	/** Convert height to color for visualization */
	FColor HeightToColor(float Height) const;

	/** Draw 2D heightmap */
	void Draw2DHeightmap(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	/** Draw grid overlay */
	void DrawGridOverlay(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	/** Preview resolution */
	int32 PreviewResolution;

	/** Show flags */
	bool bShow3DPreview;
	bool bShow2DPreview;
	bool bShowStatistics;

	/** Current terrain generation parameters */
	FTerrainGenerationParams CurrentParams;

	/** Height data for preview */
	TArray<float> HeightData;

	/** Preview mesh component */
	TSharedPtr<SViewport> PreviewViewport;

	/** Advanced preview scene for 3D preview */
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;

	/** Static mesh for preview */
	UStaticMeshComponent* PreviewMeshComponent;

	/** Heightmap texture */
	UTexture2D* HeightmapTexture;

	/** Camera rotation */
	FRotator CameraRotation;

	/** Camera distance */
	float CameraDistance;

	/** Mouse drag state */
	bool bIsDragging;
	FVector2D LastMousePosition;

	/** Statistics */
	FTerrainStats Statistics;

	/** Preview needs update */
	bool bNeedsUpdate;

	/** Last update time */
	double LastUpdateTime;

	/** Update throttle time (seconds) */
	float UpdateThrottleTime;

	/** Heightmap brush for painting */
	TSharedPtr<FSlateBrush> HeightmapBrush;

	/** Color gradient for heightmap visualization */
	TArray<FLinearColor> HeightColorGradient;

	/** Statistics text blocks */
	TSharedPtr<class STextBlock> VerticesText;
	TSharedPtr<class STextBlock> TrianglesText;
	TSharedPtr<class STextBlock> MemoryText;
	TSharedPtr<class STextBlock> MinHeightText;
	TSharedPtr<class STextBlock> MaxHeightText;
	TSharedPtr<class STextBlock> AvgHeightText;
	TSharedPtr<class STextBlock> GenerationTimeText;

	/** Initialize color gradient for heightmap */
	void InitializeColorGradient();

	/** Get formatted memory string */
	FString GetFormattedMemoryString(int32 Bytes) const;

	/** Get formatted number string */
	FString GetFormattedNumberString(int32 Number) const;

	/** Update statistics text blocks */
	void UpdateStatisticsText();
};
