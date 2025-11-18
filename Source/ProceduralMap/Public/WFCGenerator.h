// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveFunctionCollapse.h"
#include "WFCTile.h"
#include "Engine/DataAsset.h"
#include "WFCGenerator.generated.h"

/**
 * Data Asset for WFC Tile Configuration
 * Allows designers to create reusable tile sets for different structure types
 */
UCLASS(BlueprintType)
class PROCEDURALMAP_API UWFCTileSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Name of this tile set (e.g., "Village", "Dungeon", "Road System") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Set")
	FString TileSetName;

	/** Description of what this tile set generates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Set", meta = (MultiLine = true))
	FString Description;

	/** All tiles in this set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Set")
	TArray<FWFCTile> Tiles;

	/** Default grid width for this tile set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Set")
	int32 DefaultGridWidth;

	/** Default grid height for this tile set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Set")
	int32 DefaultGridHeight;

	/** Default tile size in world units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Set")
	float DefaultTileSize;

	UWFCTileSet()
		: TileSetName(TEXT("New Tile Set"))
		, Description(TEXT(""))
		, DefaultGridWidth(10)
		, DefaultGridHeight(10)
		, DefaultTileSize(100.0f)
	{
	}
};

/**
 * Preset configurations for different structure types
 */
UENUM(BlueprintType)
enum class EWFCGenerationType : uint8
{
	Custom UMETA(DisplayName = "Custom"),
	Village UMETA(DisplayName = "Village"),
	Dungeon UMETA(DisplayName = "Dungeon"),
	RoadSystem UMETA(DisplayName = "Road System"),
	Building UMETA(DisplayName = "Building Interior"),
	Maze UMETA(DisplayName = "Maze")
};

/**
 * Actor that generates procedural structures using Wave Function Collapse
 * Blueprint-friendly and can be placed directly in levels
 */
UCLASS(Blueprintable, BlueprintType)
class PROCEDURALMAP_API AWFCGenerator : public AActor
{
	GENERATED_BODY()

public:
	AWFCGenerator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ========== Configuration ==========

	/** Type of structure to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Configuration")
	EWFCGenerationType GenerationType;

	/** Tile set data asset to use (overrides GenerationType if set) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Configuration")
	UWFCTileSet* TileSetAsset;

	/** Width of the generation grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Configuration", meta = (ClampMin = "1", ClampMax = "100"))
	int32 GridWidth;

	/** Height of the generation grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Configuration", meta = (ClampMin = "1", ClampMax = "100"))
	int32 GridHeight;

	/** Size of each tile in world units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Configuration", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float TileSize;

	/** Random seed for generation (0 = random each time) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Configuration")
	int32 Seed;

	/** Use random seed on each generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Configuration")
	bool bUseRandomSeed;

	// ========== Generation Settings ==========

	/** Enable backtracking to solve contradictions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
	bool bEnableBacktracking;

	/** Maximum backtrack depth */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings", meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxBacktrackDepth;

	/** Maximum iterations before giving up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings", meta = (ClampMin = "100", ClampMax = "100000"))
	int32 MaxIterations;

	/** Generate automatically on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
	bool bGenerateOnBeginPlay;

	/** Regenerate when properties change in editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
	bool bAutoRegenerateInEditor;

	// ========== Visualization ==========

	/** Automatically spawn meshes for tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
	bool bSpawnMeshes;

	/** Show debug grid visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
	bool bShowDebugGrid;

	/** Debug grid color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
	FColor DebugGridColor;

	/** Show tile IDs in debug display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
	bool bShowTileIDs;

	/** Offset for spawned meshes (e.g., to place on ground) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
	FVector MeshOffset;

	// ========== Generation Control ==========

	/** Generate the structure */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WFC")
	void Generate();

	/** Clear all generated content */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WFC")
	void Clear();

	/** Regenerate (clear then generate) */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WFC")
	void Regenerate();

	/** Get the WFC algorithm instance */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	UWaveFunctionCollapse* GetWFCAlgorithm() const { return WFCAlgorithm; }

	/** Check if generation was successful */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	bool IsGenerationSuccessful() const { return bGenerationSuccess; }

	/** Get generation statistics */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	void GetGenerationStats(int32& OutIterations, int32& OutTotalCells, int32& OutCollapsedCells) const;

	// ========== Tile Set Creation Helpers ==========

	/** Create a simple village tile set */
	UFUNCTION(BlueprintCallable, Category = "WFC|Presets")
	static TArray<FWFCTile> CreateVillageTileSet();

	/** Create a dungeon tile set */
	UFUNCTION(BlueprintCallable, Category = "WFC|Presets")
	static TArray<FWFCTile> CreateDungeonTileSet();

	/** Create a road system tile set */
	UFUNCTION(BlueprintCallable, Category = "WFC|Presets")
	static TArray<FWFCTile> CreateRoadTileSet();

	/** Create a maze tile set */
	UFUNCTION(BlueprintCallable, Category = "WFC|Presets")
	static TArray<FWFCTile> CreateMazeTileSet();

	/** Create a building interior tile set */
	UFUNCTION(BlueprintCallable, Category = "WFC|Presets")
	static TArray<FWFCTile> CreateBuildingTileSet();

	// ========== Events ==========

	/** Called when generation starts */
	UPROPERTY(BlueprintAssignable, Category = "WFC|Events")
	FSimpleDelegate OnGenerationStarted;

	/** Called when generation completes successfully */
	UPROPERTY(BlueprintAssignable, Category = "WFC|Events")
	FSimpleDelegate OnGenerationCompleted;

	/** Called when generation fails */
	UPROPERTY(BlueprintAssignable, Category = "WFC|Events")
	FSimpleDelegate OnGenerationFailed;

protected:
	/** The WFC algorithm instance */
	UPROPERTY(BlueprintReadOnly, Category = "WFC")
	UWaveFunctionCollapse* WFCAlgorithm;

	/** Generated mesh components */
	UPROPERTY()
	TArray<UStaticMeshComponent*> GeneratedMeshes;

	/** Whether last generation was successful */
	UPROPERTY(BlueprintReadOnly, Category = "WFC")
	bool bGenerationSuccess;

	/** Number of iterations used in last generation */
	UPROPERTY(BlueprintReadOnly, Category = "WFC")
	int32 LastIterationCount;

	/**
	 * Initialize the WFC algorithm with current settings
	 */
	void InitializeWFC();

	/**
	 * Apply tile set based on generation type
	 */
	void ApplyGenerationType();

	/**
	 * Spawn visual meshes for the generated grid
	 */
	void SpawnMeshes();

	/**
	 * Draw debug visualization
	 */
	void DrawDebugVisualization();

	/**
	 * Get or create a unique seed
	 */
	int32 GetSeed() const;

	/**
	 * Clear generated meshes
	 */
	void ClearMeshes();

	/**
	 * Apply tile set from data asset
	 */
	void ApplyTileSetAsset();
};
