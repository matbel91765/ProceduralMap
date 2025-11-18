# 📚 API Reference

> **Complete API documentation for Ultimate Procedural Map System**

---

## 📑 Table of Contents

1. [Core Classes](#core-classes)
2. [Terrain System](#terrain-system)
3. [Biome System](#biome-system)
4. [Erosion System](#erosion-system)
5. [WFC System](#wfc-system)
6. [River & Road System](#river--road-system)
7. [Cave Generation](#cave-generation)
8. [Foliage System](#foliage-system)
9. [Material System](#material-system)
10. [Editor Tools](#editor-tools)
11. [Data Structures](#data-structures)
12. [Enumerations](#enumerations)

---

## 🎮 Core Classes

### UQuadTreeTerrain

**Parent**: `UActorComponent`
**Module**: `ProceduralMap` (Runtime)
**Header**: `QuadTreeTerrain.h`

Main component for QuadTree-based terrain with dynamic LOD.

#### Properties

```cpp
// Configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Configuration")
float TerrainSize;
// Total terrain size in world units (width and height)
// Default: 10000.0f
// Range: 1000.0 - 100000.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Configuration")
int32 MaxLODLevel;
// Maximum LOD level depth
// Default: 4
// Range: 1 - 8

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Configuration")
float LODDistanceMultiplier;
// LOD transition distance multiplier
// Higher = more aggressive LOD (better performance)
// Default: 2.0f
// Range: 0.5 - 5.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Configuration")
int32 MeshResolution;
// Vertices per side at LOD 0 (highest detail)
// Default: 64
// Range: 16 - 256

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering")
UMaterialInterface* TerrainMaterial;
// Material to apply to all terrain meshes

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Physics")
bool bEnableCollision;
// Enable collision on terrain meshes
// Default: true

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation")
FTerrainGenerationParams GenerationParams;
// Noise generation parameters

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Performance")
int32 UpdateFrequency;
// Update LOD every N frames (1 = every frame)
// Default: 1
// Range: 1 - 10

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Performance")
bool bEnableAsyncGeneration;
// Use async tasks for mesh generation
// Default: true

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Performance")
int32 MaxMeshGenerationsPerFrame;
// Max number of meshes to generate per frame
// Default: 5
// Range: 1 - 20

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering")
bool bSmoothNormals;
// Calculate smooth normals for better lighting
// Default: true

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Debug")
bool bShowDebugQuadTree;
// Visualize QuadTree structure
// Default: false

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Debug")
bool bShowLODColors;
// Color-code LOD levels for debugging
// Default: false
```

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "Terrain")
void InitializeTerrain();
// Initialize the terrain system
// Call once after setting up parameters
// Must be called before terrain is visible

UFUNCTION(BlueprintCallable, Category = "Terrain")
void UpdateLOD(const FVector& CameraPosition);
// Update LOD based on camera position
// Called automatically each frame if auto-update enabled
// Parameters:
//   - CameraPosition: World position of camera/viewer

UFUNCTION(BlueprintCallable, Category = "Terrain")
void RegenerateTerrain();
// Force complete regeneration of terrain
// Destroys all existing meshes and rebuilds from scratch
// Use when changing fundamental parameters (seed, size, etc.)

UFUNCTION(BlueprintCallable, Category = "Terrain")
void CleanupTerrain();
// Cleanup all terrain resources
// Destroys all mesh components and frees memory
// Call before destroying the component

UFUNCTION(BlueprintCallable, Category = "Terrain")
float GetHeightAtPosition(const FVector& WorldPosition) const;
// Get terrain height at a world position
// Uses noise function for accurate interpolation
// Parameters:
//   - WorldPosition: Position to query (X, Y used, Z ignored)
// Returns:
//   - Height value at position

UFUNCTION(BlueprintCallable, Category = "Terrain")
FVector GetNormalAtPosition(const FVector& WorldPosition) const;
// Get terrain normal at a world position
// Uses finite differences for smooth normals
// Parameters:
//   - WorldPosition: Position to query
// Returns:
//   - Normalized normal vector

UFUNCTION(BlueprintCallable, Category = "Terrain")
FLODStatistics GetLODStatistics() const;
// Get current LOD and rendering statistics
// Returns:
//   - Statistics structure with node counts, vertices, triangles, etc.
```

#### Usage Example

```cpp
// C++ Example
UQuadTreeTerrain* Terrain = CreateDefaultSubobject<UQuadTreeTerrain>(TEXT("Terrain"));

// Configure
Terrain->TerrainSize = 20000.0f;
Terrain->MaxLODLevel = 5;
Terrain->MeshResolution = 64;
Terrain->LODDistanceMultiplier = 2.5f;

// Set generation parameters
Terrain->GenerationParams.Seed = 12345;
Terrain->GenerationParams.Frequency = 0.005f;
Terrain->GenerationParams.Octaves = 6;
Terrain->GenerationParams.HeightScale = 1000.0f;

// Initialize
Terrain->InitializeTerrain();

// Query height
FVector PlayerPos = GetActorLocation();
float Height = Terrain->GetHeightAtPosition(PlayerPos);
```

```blueprint
// Blueprint Example
BeginPlay:
  Get Component by Class (QuadTreeTerrain) → InitializeTerrain

Tick:
  Get Player Camera Manager → Get Camera Location → UpdateLOD

Custom Function:
  Get Actor Location → GetHeightAtPosition → Print String
```

---

### ULODManager

**Parent**: `UObject`
**Module**: `ProceduralMap` (Runtime)
**Header**: `LODManager.h`

Manages LOD configurations and statistics.

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "LOD")
void SetLODConfiguration(FName BiomeName, const FLODConfiguration& Config);
// Set LOD configuration for a specific biome
// Parameters:
//   - BiomeName: Name of biome
//   - Config: LOD configuration structure

UFUNCTION(BlueprintCallable, Category = "LOD")
FLODConfiguration GetLODConfiguration(FName BiomeName) const;
// Get LOD configuration for a biome
// Parameters:
//   - BiomeName: Name of biome to query
// Returns:
//   - LOD configuration structure

UFUNCTION(BlueprintCallable, Category = "LOD")
int32 CalculateLODLevel(float Distance, float NodeSize) const;
// Calculate appropriate LOD level for given distance
// Parameters:
//   - Distance: Distance from camera
//   - NodeSize: Size of the terrain node
// Returns:
//   - LOD level (0 = highest detail)

UFUNCTION(BlueprintCallable, Category = "LOD")
FLODStatistics GetStatistics() const;
// Get current LOD statistics
// Returns:
//   - Statistics structure
```

---

## 🌿 Biome System

### ABiomeGenerator

**Parent**: `AActor`
**Module**: `ProceduralMap` (Runtime)
**Header**: `BiomeGenerator.h`

Handles climate calculation and biome determination.

#### Properties

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
UBiomeConfigAsset* BiomeConfig;
// Asset defining all biome types and settings

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
FNoiseParameters TemperatureNoise;
// Noise parameters for temperature map

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
FNoiseParameters HumidityNoise;
// Noise parameters for humidity map

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
float TemperatureScale;
// Global temperature multiplier
// Default: 1.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
float HumidityScale;
// Global humidity multiplier
// Default: 1.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
float WorldSizeX;
// World extent in X (for latitude calculation)

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
float WorldSizeY;
// World extent in Y (for latitude calculation)

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
float MaxElevation;
// Maximum elevation for normalization
```

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "Biome|Climate")
float CalculateTemperature(FVector Position, float Elevation);
// Calculate temperature at position
// Parameters:
//   - Position: World position
//   - Elevation: Normalized elevation (0-1)
// Returns:
//   - Temperature value (0-1, where 0=cold, 1=hot)

UFUNCTION(BlueprintCallable, Category = "Biome|Climate")
float CalculateHumidity(FVector Position, float DistanceToWater);
// Calculate humidity at position
// Parameters:
//   - Position: World position
//   - DistanceToWater: Distance to nearest water body
// Returns:
//   - Humidity value (0-1, where 0=dry, 1=wet)

UFUNCTION(BlueprintCallable, Category = "Biome|Generation")
EBiomeType GetBiomeFromClimate(float Temperature, float Humidity, float Elevation);
// Determine biome type from climate parameters
// Uses Whittaker diagram classification
// Parameters:
//   - Temperature: Temperature (0-1)
//   - Humidity: Humidity (0-1)
//   - Elevation: Elevation (0-1)
// Returns:
//   - Biome type enumeration

UFUNCTION(BlueprintCallable, Category = "Biome|Generation")
TArray<FBiomeData> CalculateBiomeWeights(FVector Position, float Elevation,
                                         float DistanceToWater);
// Calculate weighted biome data for smooth transitions
// Parameters:
//   - Position: World position
//   - Elevation: Normalized elevation
//   - DistanceToWater: Distance to water
// Returns:
//   - Array of biome data with weights (sum to 1.0)

UFUNCTION(BlueprintCallable, Category = "Biome|Noise")
float GenerateNoise(float X, float Y, const FNoiseParameters& Params);
// Generate multi-octave noise value
// Parameters:
//   - X, Y: Coordinates
//   - Params: Noise parameters
// Returns:
//   - Noise value (-1 to 1)

UFUNCTION(BlueprintCallable, Category = "Biome|Config")
FBiomeSettings GetBiomeSettings(EBiomeType BiomeType);
// Get settings for specific biome type
// Parameters:
//   - BiomeType: Type of biome
// Returns:
//   - Biome settings structure
```

#### Usage Example

```cpp
// Spawn biome generator
ABiomeGenerator* BiomeGen = GetWorld()->SpawnActor<ABiomeGenerator>();

// Configure
BiomeGen->WorldSizeX = 20000.0f;
BiomeGen->WorldSizeY = 20000.0f;
BiomeGen->TemperatureScale = 1.0f;
BiomeGen->HumidityScale = 1.0f;

// Set noise parameters
BiomeGen->TemperatureNoise.Frequency = 0.001f;
BiomeGen->TemperatureNoise.Octaves = 4;
BiomeGen->HumidityNoise.Frequency = 0.002f;
BiomeGen->HumidityNoise.Octaves = 3;

// Query biome at position
FVector Position(1000, 500, 0);
float Elevation = 0.5f;
float DistToWater = 1000.0f;

float Temp = BiomeGen->CalculateTemperature(Position, Elevation);
float Humidity = BiomeGen->CalculateHumidity(Position, DistToWater);
EBiomeType Biome = BiomeGen->GetBiomeFromClimate(Temp, Humidity, Elevation);

// Or get blended weights for smooth transitions
TArray<FBiomeData> BiomeWeights = BiomeGen->CalculateBiomeWeights(
    Position, Elevation, DistToWater
);
```

---

### UBiomeBlender

**Parent**: `UObject`
**Module**: `ProceduralMap` (Runtime)
**Header**: `BiomeBlender.h`

Handles smooth blending between multiple biomes.

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "Biome|Blending")
TArray<FBiomeData> CalculateBlendedBiomes(FVector Position, float BlendRadius);
// Calculate biome weights in a radius
// Parameters:
//   - Position: Center position
//   - BlendRadius: Sampling radius
// Returns:
//   - Array of biomes with normalized weights

UFUNCTION(BlueprintCallable, Category = "Biome|Blending")
FBiomeSettings BlendBiomeSettings(const TArray<FBiomeData>& BiomeWeights);
// Blend biome settings based on weights
// Parameters:
//   - BiomeWeights: Array of biome data with weights
// Returns:
//   - Blended biome settings
```

---

## 💧 Erosion System

### UHydraulicErosion

**Parent**: `UObject`
**Module**: `ProceduralMap` (Runtime)
**Header**: `HydraulicErosion.h`

Particle-based hydraulic erosion simulation.

#### Properties

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Simulation")
int32 NumIterations;
// Number of droplets to simulate
// Default: 50000
// Range: 1000 - 10000000

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Simulation")
int32 ErosionRadius;
// Radius of erosion brush in cells
// Default: 3
// Range: 1 - 16

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics")
float Inertia;
// Droplet inertia (momentum vs gradient following)
// 0 = follow gradient exactly, 1 = maintain direction
// Default: 0.05f
// Range: 0.0 - 1.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics")
float SedimentCapacityFactor;
// Multiplier for sediment capacity
// Higher = more erosion potential
// Default: 4.0f
// Range: 0.1 - 10.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics")
float MinSedimentCapacity;
// Minimum sediment capacity
// Default: 0.01f
// Range: 0.001 - 1.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics")
float ErodeSpeed;
// How fast terrain erodes
// Default: 0.3f
// Range: 0.0 - 1.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics")
float DepositSpeed;
// How fast sediment deposits
// Default: 0.3f
// Range: 0.0 - 1.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics")
float EvaporateSpeed;
// Water evaporation rate per iteration
// Default: 0.01f
// Range: 0.0 - 1.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Physics")
float Gravity;
// Gravity strength
// Default: 4.0f
// Range: 0.1 - 10.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Simulation")
float InitialWaterVolume;
// Starting water volume per droplet
// Default: 1.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Simulation")
float InitialSpeed;
// Starting speed per droplet
// Default: 1.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Simulation")
int32 MaxDropletLifetime;
// Maximum iterations per droplet
// Default: 30
// Range: 10 - 100

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Settings")
int32 RandomSeed;
// Random seed for reproducible results
// Default: 1337
```

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "Erosion")
void Erode(UPARAM(ref) TArray<float>& HeightMap, int32 MapSize, bool bAsync = false);
// Apply hydraulic erosion to heightmap
// Parameters:
//   - HeightMap: 1D heightmap array (size = MapSize * MapSize)
//   - MapSize: Width/height of heightmap
//   - bAsync: Use async task (non-blocking)

UFUNCTION(BlueprintCallable, Category = "Erosion")
bool IsErosionComplete() const;
// Check if async erosion has finished
// Returns:
//   - true if complete or not running async

UFUNCTION(BlueprintCallable, Category = "Erosion")
float GetProgress() const;
// Get erosion progress (0-1)
// Returns:
//   - Progress percentage

UFUNCTION(BlueprintCallable, Category = "Erosion")
void CancelErosion();
// Cancel async erosion task
```

#### Usage Example

```cpp
// Create erosion system
UHydraulicErosion* Erosion = NewObject<UHydraulicErosion>();

// Configure for realistic erosion
Erosion->NumIterations = 100000;
Erosion->ErosionRadius = 3;
Erosion->Inertia = 0.05f;
Erosion->SedimentCapacityFactor = 4.0f;
Erosion->ErodeSpeed = 0.3f;
Erosion->DepositSpeed = 0.3f;
Erosion->EvaporateSpeed = 0.01f;
Erosion->Gravity = 4.0f;
Erosion->RandomSeed = 12345;

// Apply to heightmap (async)
TArray<float> HeightMap = GetYourHeightMapData();
int32 MapSize = 512;

Erosion->Erode(HeightMap, MapSize, true);  // Async

// Wait for completion
while (!Erosion->IsErosionComplete())
{
    float Progress = Erosion->GetProgress();
    UE_LOG(LogTemp, Log, TEXT("Erosion progress: %.1f%%"), Progress * 100.0f);
    FPlatformProcess::Sleep(0.1f);
}

// Apply modified heightmap back to terrain
```

---

### UThermalErosion

**Parent**: `UObject`
**Module**: `ProceduralMap` (Runtime)
**Header**: `ThermalErosion.h`

Slope-based thermal weathering simulation.

#### Properties

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Thermal")
int32 NumIterations;
// Number of erosion iterations
// Default: 10
// Range: 1 - 1000

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Thermal")
float TalusAngle;
// Angle of repose in degrees
// Material slides when slope exceeds this
// Default: 35.0f
// Range: 0.0 - 90.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Thermal")
float ErosionRate;
// Rate of material transfer
// Default: 0.5f
// Range: 0.0 - 1.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Thermal")
float CellSize;
// World units per heightmap cell
// Used for slope calculation
// Default: 100.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Thermal")
bool bUse8Neighbors;
// Use 8-neighbor vs 4-neighbor algorithm
// 8-neighbor is more realistic
// Default: true
```

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "Erosion")
void Erode(UPARAM(ref) TArray<float>& HeightMap, int32 MapSize, bool bAsync = false);
// Apply thermal erosion to heightmap
// Parameters:
//   - HeightMap: 1D heightmap array
//   - MapSize: Width/height of heightmap
//   - bAsync: Use async task

UFUNCTION(BlueprintCallable, Category = "Erosion")
bool IsComplete() const;
// Check if erosion finished
```

#### Usage Example

```cpp
// Create thermal erosion
UThermalErosion* Thermal = NewObject<UThermalErosion>();

// Configure
Thermal->NumIterations = 15;
Thermal->TalusAngle = 35.0f;  // Typical for rock/soil
Thermal->ErosionRate = 0.5f;
Thermal->CellSize = 100.0f;
Thermal->bUse8Neighbors = true;

// Apply after hydraulic erosion for best results
Thermal->Erode(HeightMap, MapSize, false);
```

---

### FErosionGPUInterface

**Type**: Static Class
**Module**: `ProceduralMap` (Runtime)
**Header**: `ErosionComputeShader.h`

GPU-accelerated erosion using compute shaders.

#### Methods

```cpp
static void RunErosion(
    const TArray<float>& InputHeightMap,
    int32 MapSize,
    TArray<float>& OutputHeightMap,
    int32 NumDroplets,
    const FErosionParameters* Params = nullptr
);
// Run erosion on GPU using compute shader
// Parameters:
//   - InputHeightMap: Input heightmap data
//   - MapSize: Size of heightmap (must be power of 2)
//   - OutputHeightMap: Output eroded heightmap
//   - NumDroplets: Number of droplets to simulate
//   - Params: Optional parameters (uses defaults if null)
// Note: Requires RHI thread synchronization
```

#### Usage Example

```cpp
// Prepare parameters
FErosionParameters Params;
Params.Inertia = 0.05f;
Params.ErodeSpeed = 0.3f;
Params.DepositSpeed = 0.3f;
Params.EvaporateSpeed = 0.01f;
Params.Gravity = 4.0f;
Params.SedimentCapacityFactor = 4.0f;
Params.MinSedimentCapacity = 0.01f;
Params.ErosionRadius = 3;
Params.MaxDropletLifetime = 30;
Params.RandomSeed = 12345;

// Run on GPU (very fast!)
TArray<float> OutputMap;
FErosionGPUInterface::RunErosion(
    InputHeightMap,
    512,  // Map size
    OutputMap,
    1000000,  // 1 million droplets
    &Params
);

// Process results
// Typically completes in 5-15 seconds for 1M droplets on RTX 3080
```

---

## 🎲 WFC System

### UWaveFunctionCollapse

**Parent**: `UObject`
**Module**: `ProceduralMap` (Runtime)
**Header**: `WaveFunctionCollapse.h`

Wave Function Collapse algorithm for constraint-based generation.

#### Properties

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Grid")
int32 GridWidth;
// Width of generation grid
// Default: 32

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Grid")
int32 GridHeight;
// Height of generation grid
// Default: 32

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Tiles")
TArray<FWFCTile> TileSet;
// Set of all available tiles with adjacency rules

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
int32 RandomSeed;
// Seed for deterministic generation
// Default: 42

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
int32 MaxIterations;
// Maximum iterations before giving up
// Default: 10000

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
bool bEnableBacktracking;
// Enable backtracking on contradiction
// Default: true

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Settings")
int32 MaxBacktrackDepth;
// Maximum backtrack attempts
// Default: 100

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
float TileSize;
// Size of each tile in world units
// Default: 100.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC|Visualization")
bool bGenerateMeshes;
// Automatically generate visual meshes
// Default: true
```

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "WFC")
bool Generate();
// Run complete WFC algorithm
// Returns:
//   - true if generation succeeded, false if contradiction

UFUNCTION(BlueprintCallable, Category = "WFC")
void InitializeGrid();
// Initialize grid with all tiles possible in each cell

UFUNCTION(BlueprintCallable, Category = "WFC")
bool FindLowestEntropyCell(int32& OutX, int32& OutY);
// Find cell with lowest entropy (most constrained)
// Parameters:
//   - OutX, OutY: Output coordinates
// Returns:
//   - true if cell found, false if all collapsed

UFUNCTION(BlueprintCallable, Category = "WFC")
bool CollapseCell(int32 X, int32 Y);
// Collapse specific cell to single tile
// Parameters:
//   - X, Y: Cell coordinates
// Returns:
//   - true if collapse succeeded

UFUNCTION(BlueprintCallable, Category = "WFC")
bool Propagate(int32 StartX, int32 StartY);
// Propagate constraints from collapsed cell
// Parameters:
//   - StartX, StartY: Starting cell coordinates
// Returns:
//   - true if propagation succeeded without contradiction

UFUNCTION(BlueprintCallable, Category = "WFC")
FWFCCell* GetCell(int32 X, int32 Y);
// Get cell at grid position
// Parameters:
//   - X, Y: Grid coordinates
// Returns:
//   - Pointer to cell, or nullptr if out of bounds

UFUNCTION(BlueprintCallable, Category = "WFC")
bool IsFullyCollapsed() const;
// Check if all cells are collapsed
// Returns:
//   - true if generation complete

UFUNCTION(BlueprintCallable, Category = "WFC")
void ResetGrid();
// Reset grid to initial state

UFUNCTION(BlueprintCallable, Category = "WFC")
TArray<UStaticMeshComponent*> GenerateMeshes(UWorld* World, const FVector& BaseLocation,
                                             AActor* Owner);
// Generate visual meshes for collapsed grid
// Parameters:
//   - World: World to spawn in
//   - BaseLocation: Base position for grid
//   - Owner: Owner actor for components
// Returns:
//   - Array of spawned mesh components

UFUNCTION(BlueprintCallable, Category = "WFC")
void GetStatistics(int32& OutTotalCells, int32& OutCollapsedCells,
                   float& OutAverageEntropy) const;
// Get generation statistics
// Parameters:
//   - OutTotalCells: Total cell count
//   - OutCollapsedCells: Number of collapsed cells
//   - OutAverageEntropy: Average entropy of uncollapsed cells
```

#### Usage Example

```cpp
// Create WFC system
UWaveFunctionCollapse* WFC = NewObject<UWaveFunctionCollapse>();

// Configure
WFC->GridWidth = 50;
WFC->GridHeight = 50;
WFC->RandomSeed = 12345;
WFC->MaxIterations = 10000;
WFC->bEnableBacktracking = true;
WFC->TileSize = 100.0f;

// Define tiles
FWFCTile FloorTile;
FloorTile.TileID = 0;
FloorTile.TileName = TEXT("Floor");
FloorTile.Mesh = FloorMesh;
FloorTile.Weight = 10.0f;
FloorTile.ValidNorth = {0, 1};  // Can be next to floor or wall
FloorTile.ValidEast = {0, 1};
FloorTile.ValidSouth = {0, 1};
FloorTile.ValidWest = {0, 1};

FWFCTile WallTile;
WallTile.TileID = 1;
WallTile.TileName = TEXT("Wall");
WallTile.Mesh = WallMesh;
WallTile.Weight = 5.0f;
WallTile.ValidNorth = {0, 1};
WallTile.ValidEast = {0, 1};
WallTile.ValidSouth = {0, 1};
WallTile.ValidWest = {0, 1};

WFC->TileSet.Add(FloorTile);
WFC->TileSet.Add(WallTile);

// Generate
bool bSuccess = WFC->Generate();

if (bSuccess)
{
    // Create visual representation
    TArray<UStaticMeshComponent*> Meshes = WFC->GenerateMeshes(
        GetWorld(),
        FVector::ZeroVector,
        this
    );

    // Get statistics
    int32 TotalCells, CollapsedCells;
    float AvgEntropy;
    WFC->GetStatistics(TotalCells, CollapsedCells, AvgEntropy);
    UE_LOG(LogTemp, Log, TEXT("Generated %d/%d cells"), CollapsedCells, TotalCells);
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("WFC generation failed - contradiction!"));
}
```

---

## 🌊 River & Road Generation

### ARiverGenerator

**Parent**: `AActor`
**Module**: `ProceduralMap` (Runtime)
**Header**: `RiverGenerator.h`

Procedural river generation with pathfinding and landscape modification.

#### Properties

```cpp
// River Parameters
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters")
float Width;
// Width of river in cm
// Default: 500.0f
// Range: 100.0 - 10000.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters")
float Depth;
// Depth to carve into landscape in cm
// Default: 100.0f
// Range: 0.0 - 1000.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters")
float FlowSpeed;
// Flow speed of water
// Default: 100.0f
// Range: 0.0 - 1000.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters")
float Curvature;
// Curvature amount (0=straight, 1=very curvy)
// Default: 0.5f
// Range: 0.0 - 1.0

// Generation Settings
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
FVector SourceLocation;
// Starting point of river

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
FVector DestinationLocation;
// Ending point of river

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
bool bAutoFindDestination;
// Automatically find lowest point for destination
// Default: false

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
float PathfindingStepSize;
// Step size for pathfinding in cm
// Default: 500.0f
// Range: 100.0 - 5000.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
float DownhillBias;
// Prefer downhill paths (higher=stronger preference)
// Default: 5.0f
// Range: 0.0 - 10.0

// Curl Noise
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Noise")
bool bApplyCurlNoise;
// Enable curl noise distortion
// Default: true

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Noise")
float NoiseFrequency;
// Frequency of curl noise
// Default: 0.001f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Noise")
float NoiseAmplitude;
// Amplitude of curl noise displacement
// Default: 200.0f

// Landscape Modification
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Landscape")
ALandscape* TargetLandscape;
// Reference to landscape to modify

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Landscape")
bool bModifyLandscape;
// Enable landscape carving
// Default: true

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Landscape")
float SmoothingRadius;
// Smoothing radius around river
// Default: 1000.0f

// Water System
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Water")
bool bCreateWaterBody;
// Enable UE5 Water System integration
// Default: false

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Water")
TSubclassOf<AWaterBodyRiver> WaterBodyClass;
// Water body class to spawn
```

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "River|Generation")
void GenerateRiver();
// Generate complete river system
// Runs pathfinding, creates spline, applies modifications

UFUNCTION(BlueprintCallable, Category = "River|Generation")
TArray<FVector> FindLowestPath(const FVector& Start, const FVector& End);
// Find lowest path using A* or gradient descent
// Parameters:
//   - Start: Starting position
//   - End: Ending position
// Returns:
//   - Array of path points

UFUNCTION(BlueprintCallable, Category = "River|Generation")
void GenerateRiverSpline(const TArray<FVector>& PathPoints);
// Generate river spline from path points
// Parameters:
//   - PathPoints: Array of positions along river

UFUNCTION(BlueprintCallable, Category = "River|Generation")
void ApplyCurlNoise();
// Apply curl noise distortion to spline
// Creates natural meandering

UFUNCTION(BlueprintCallable, Category = "River|Generation")
void ModifyLandscape();
// Modify landscape to carve river bed

UFUNCTION(BlueprintCallable, Category = "River|Generation")
void CreateWaterBody();
// Create UE5 Water Body actor

UFUNCTION(BlueprintCallable, Category = "River|Generation")
void GenerateTributaries();
// Generate all tributary rivers

UFUNCTION(BlueprintCallable, Category = "River|Generation")
void ClearRiver();
// Clear generated river data

UFUNCTION(BlueprintCallable, Category = "River|Utilities")
float GetTerrainHeight(const FVector& WorldPosition) const;
// Get terrain height at position
// Parameters:
//   - WorldPosition: Position to query
// Returns:
//   - Height at position

UFUNCTION(BlueprintCallable, Category = "River|Utilities")
FVector GetTerrainNormal(const FVector& WorldPosition) const;
// Get terrain normal at position
// Parameters:
//   - WorldPosition: Position to query
// Returns:
//   - Normal vector at position

UFUNCTION(BlueprintCallable, Category = "River|Utilities")
float CalculateSlope(const FVector& Point1, const FVector& Point2) const;
// Calculate slope between two points
// Parameters:
//   - Point1, Point2: Points to measure between
// Returns:
//   - Slope in degrees
```

#### Usage Example

```cpp
// Spawn river generator
ARiverGenerator* River = GetWorld()->SpawnActor<ARiverGenerator>();

// Configure river
River->SourceLocation = FVector(0, 0, 1000);
River->bAutoFindDestination = true;  // Find lowest point automatically
River->DestinationSearchRadius = 10000.0f;

// River parameters
River->Width = 500.0f;
River->Depth = 100.0f;
River->FlowSpeed = 150.0f;
River->Curvature = 0.6f;

// Pathfinding
River->PathfindingStepSize = 500.0f;
River->DownhillBias = 5.0f;  // Strong downhill preference

// Noise for natural meandering
River->bApplyCurlNoise = true;
River->NoiseFrequency = 0.001f;
River->NoiseAmplitude = 200.0f;

// Landscape modification
River->TargetLandscape = MyLandscape;
River->bModifyLandscape = true;
River->SmoothingRadius = 1000.0f;

// Generate!
River->GenerateRiver();

// Optional: Add tributaries
FTributaryConfig Tributary;
Tributary.SourceLocation = FVector(5000, 5000, 800);
Tributary.WidthMultiplier = 0.5f;
Tributary.FlowSpeedMultiplier = 0.7f;
River->Tributaries.Add(Tributary);
River->GenerateTributaries();
```

---

## 🕳️ Cave Generation

### UCellularAutomata

**Parent**: `UObject`
**Module**: `ProceduralMap` (Runtime)
**Header**: `CellularAutomata.h`

Cellular automata for organic cave generation.

#### Properties

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave|Grid")
int32 GridWidth;
// Width of cave grid
// Default: 100

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave|Grid")
int32 GridHeight;
// Height of cave grid
// Default: 100

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave|Generation")
float FillPercent;
// Initial fill percentage (0-1)
// Higher = more walls, smaller caves
// Default: 0.45f
// Range: 0.3 - 0.7

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave|Generation")
int32 NumIterations;
// Number of cellular automata iterations
// More iterations = smoother caves
// Default: 5
// Range: 1 - 20

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave|Generation")
int32 Seed;
// Random seed
// Default: 42

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave|Processing")
bool bRemoveSmallRegions;
// Remove small disconnected regions
// Default: true

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave|Processing")
int32 MinRegionSize;
// Minimum region size to keep
// Default: 50

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave|Processing")
bool bConnectRegions;
// Connect separate cave regions
// Default: true
```

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "Cave")
void Generate();
// Generate cave using cellular automata

UFUNCTION(BlueprintCallable, Category = "Cave")
void InitializeGrid();
// Initialize grid with random cells

UFUNCTION(BlueprintCallable, Category = "Cave")
void ApplyCellularAutomataRules();
// Apply one iteration of CA rules

UFUNCTION(BlueprintCallable, Category = "Cave")
int32 CountWallNeighbors(int32 X, int32 Y) const;
// Count wall neighbors for a cell
// Parameters:
//   - X, Y: Cell coordinates
// Returns:
//   - Number of wall neighbors (0-8)

UFUNCTION(BlueprintCallable, Category = "Cave")
int32 GetCellValue(int32 X, int32 Y) const;
// Get cell value (0=floor, 1=wall)
// Parameters:
//   - X, Y: Cell coordinates
// Returns:
//   - Cell value

UFUNCTION(BlueprintCallable, Category = "Cave")
void SetCellValue(int32 X, int32 Y, int32 Value);
// Set cell value
// Parameters:
//   - X, Y: Cell coordinates
//   - Value: New value (0 or 1)
```

---

### ACaveGenerator

**Parent**: `AActor`
**Module**: `ProceduralMap` (Runtime)
**Header**: `CaveGenerator.h`

3D cave mesh generation actor.

#### Properties

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
UCellularAutomata* CellularAutomata;
// Cellular automata component

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
ECaveMeshMode MeshMode;
// Mesh generation mode
// - MarchingSquares2D: 2D dungeon-style
// - MarchingCubes3D: Full 3D caves
// - SimpleBlocks: Minecraft-style voxels
// Default: MarchingCubes3D

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
float VoxelSize;
// Size of each voxel in cm
// Default: 100.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
float WallHeight;
// Height of walls for 2D mode
// Default: 400.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Generation")
bool bGenerateCollision;
// Generate collision mesh
// Default: true

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Materials")
UMaterialInterface* CaveMaterial;
// Material for cave walls

// Stalactites/Stalagmites
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
bool bGenerateStalactites;
// Generate stalactites
// Default: false

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
float StalactiteChance;
// Probability per ceiling cell
// Default: 0.1f
// Range: 0.0 - 1.0

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
float StalactiteMinHeight;
// Minimum stalactite height
// Default: 50.0f

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Decorations")
float StalactiteMaxHeight;
// Maximum stalactite height
// Default: 200.0f

// Similar properties for stalagmites...

// Resources
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Resources")
TArray<FResourcePlacement> ResourceTypes;
// Array of resource types to spawn

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generator|Resources")
bool bSpawnResources;
// Enable resource spawning
// Default: false
```

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "Cave Generator")
void GenerateCave();
// Generate complete cave system
// Runs cellular automata + mesh generation + decorations

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
void GenerateCaveMesh();
// Generate cave mesh from cellular automata data

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
void GenerateMarchingSquares2D();
// Generate 2D cave mesh using marching squares

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
void GenerateMarchingCubes3D();
// Generate 3D cave mesh using marching cubes

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
void GenerateSimpleBlockMesh();
// Generate simple voxel block mesh

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
void GenerateSpeleothems();
// Generate stalactites and stalagmites

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
void SpawnResources();
// Spawn resources in cave

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
void ClearCave();
// Clear all generated geometry

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
FVector GetWorldPosition(int32 X, int32 Y, int32 Z = 0) const;
// Get world position from grid coordinates
// Parameters:
//   - X, Y, Z: Grid coordinates
// Returns:
//   - World position

UFUNCTION(BlueprintCallable, Category = "Cave Generator")
FIntVector GetGridPosition(FVector WorldPosition) const;
// Get grid coordinates from world position
// Parameters:
//   - WorldPosition: World position
// Returns:
//   - Grid coordinates
```

#### Usage Example

```cpp
// Spawn cave generator
ACaveGenerator* Cave = GetWorld()->SpawnActor<ACaveGenerator>();

// Create cellular automata component
Cave->CellularAutomata = NewObject<UCellularAutomata>(Cave);
Cave->CellularAutomata->GridWidth = 100;
Cave->CellularAutomata->GridHeight = 100;
Cave->CellularAutomata->FillPercent = 0.45f;
Cave->CellularAutomata->NumIterations = 5;
Cave->CellularAutomata->bRemoveSmallRegions = true;
Cave->CellularAutomata->MinRegionSize = 50;
Cave->CellularAutomata->bConnectRegions = true;

// Configure cave
Cave->MeshMode = ECaveMeshMode::MarchingCubes3D;
Cave->VoxelSize = 100.0f;
Cave->bGenerateCollision = true;
Cave->CaveMaterial = CaveMat;

// Decorations
Cave->bGenerateStalactites = true;
Cave->StalactiteChance = 0.1f;
Cave->StalactiteMinHeight = 50.0f;
Cave->StalactiteMaxHeight = 200.0f;

Cave->bGenerateStalagmites = true;
Cave->StalagmiteChance = 0.15f;

// Resources
FResourcePlacement GoldOre;
GoldOre.ResourceClass = AGoldOreActor::StaticClass();
GoldOre.SpawnChance = 0.3f;
GoldOre.MinPerRoom = 1;
GoldOre.MaxPerRoom = 3;
GoldOre.MinDistanceBetween = 300.0f;
Cave->ResourceTypes.Add(GoldOre);

Cave->bSpawnResources = true;

// Generate!
Cave->GenerateCave();
```

---

## 🌳 Foliage System

### UProceduralFoliageSpawner

**Parent**: `UObject`
**Module**: `ProceduralMap` (Runtime)
**Header**: `ProceduralFoliageSpawner.h`

Procedural vegetation placement system.

#### Methods

```cpp
UFUNCTION(BlueprintCallable, Category = "Foliage")
void SpawnFoliage(AActor* TargetTerrain, const TArray<FVegetationLayer>& Layers);
// Spawn foliage on terrain
// Parameters:
//   - TargetTerrain: Actor to spawn foliage on
//   - Layers: Array of vegetation layers

UFUNCTION(BlueprintCallable, Category = "Foliage")
void ClearFoliage();
// Remove all spawned foliage

UFUNCTION(BlueprintCallable, Category = "Foliage")
void UpdateFoliage(const FVector& ViewerLocation);
// Update foliage visibility based on viewer
// Parameters:
//   - ViewerLocation: Camera/player position

UFUNCTION(BlueprintCallable, Category = "Foliage")
int32 GetInstanceCount() const;
// Get total number of foliage instances
// Returns:
//   - Total instance count
```

---

## 📊 Data Structures

### FTerrainGenerationParams

Noise generation parameters.

```cpp
USTRUCT(BlueprintType)
struct FTerrainGenerationParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 Seed;
    // Noise seed for reproducibility
    // Default: 1337

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Frequency;
    // Base noise frequency
    // Default: 0.005f
    // Range: 0.001 - 0.1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 Octaves;
    // Number of fractal octaves
    // Default: 6
    // Range: 1 - 8

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Lacunarity;
    // Frequency multiplier per octave
    // Default: 2.0f
    // Range: 1.5 - 3.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Gain;
    // Amplitude multiplier per octave
    // Default: 0.5f
    // Range: 0.3 - 0.7

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale;
    // Vertical scale multiplier
    // Default: 500.0f
    // Range: 100.0 - 5000.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightOffset;
    // Base height offset
    // Default: 0.0f
    // Range: -1000.0 - 1000.0
};
```

---

### FLODConfiguration

LOD configuration per biome.

```cpp
USTRUCT(BlueprintType)
struct FLODConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> LODDistances;
    // Distance thresholds for each LOD level
    // Example: {0, 2000, 5000, 10000, 20000}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> MeshResolutions;
    // Mesh resolution at each LOD
    // Example: {128, 64, 32, 16, 8}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> CollisionEnabled;
    // Enable collision at each LOD
    // Example: {true, true, false, false, false}
};
```

---

### FLODStatistics

Runtime LOD statistics.

```cpp
USTRUCT(BlueprintType)
struct FLODStatistics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalNodes;
    // Total QuadTree nodes

    UPROPERTY(BlueprintReadOnly)
    int32 VisibleNodes;
    // Currently visible nodes

    UPROPERTY(BlueprintReadOnly)
    int32 TotalVertices;
    // Total vertices rendered

    UPROPERTY(BlueprintReadOnly)
    int32 TotalTriangles;
    // Total triangles rendered

    UPROPERTY(BlueprintReadOnly)
    TMap<int32, int32> NodesPerLOD;
    // Node count per LOD level

    UPROPERTY(BlueprintReadOnly)
    float UpdateTime;
    // Last update duration (milliseconds)

    UPROPERTY(BlueprintReadOnly)
    int32 MeshUpdatesThisFrame;
    // Meshes generated this frame
};
```

---

### FBiomeSettings

Biome configuration.

```cpp
USTRUCT(BlueprintType)
struct FBiomeSettings
{
    GENERATED_BODY()

    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor BaseColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* AlbedoTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* NormalTexture;

    // Climate
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinHumidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHumidity;

    // Terrain
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeightMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Roughness;

    // Vegetation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVegetationType> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VegetationDensity;
};
```

---

### FBiomeData

Biome with weight for blending.

```cpp
USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EBiomeType BiomeType;
    // Type of biome

    UPROPERTY(BlueprintReadWrite)
    float Weight;
    // Blend weight (0-1)

    UPROPERTY(BlueprintReadWrite)
    FBiomeSettings Settings;
    // Biome settings
};
```

---

### FVegetationLayer

Vegetation layer configuration.

```cpp
USTRUCT(BlueprintType)
struct FVegetationLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMesh*> Meshes;
    // Meshes to spawn (random selection)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DensityThreshold;
    // Minimum density to spawn
    // Range: 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InstancesPerCell;
    // Instances per grid cell

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GridCellSize;
    // Size of spawning grid cells

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSlope;
    // Minimum slope angle (degrees)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSlope;
    // Maximum slope angle (degrees)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinHeight;
    // Minimum spawn height

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHeight;
    // Maximum spawn height

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinScale;
    // Minimum instance scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxScale;
    // Maximum instance scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRandomYaw;
    // Randomize yaw rotation

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseInstancedMesh;
    // Use HISM for performance

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;
    // Random seed

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DensityNoiseFrequency;
    // Noise frequency for density variation
};
```

---

### FWFCTile

Wave Function Collapse tile definition.

```cpp
USTRUCT(BlueprintType)
struct FWFCTile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TileID;
    // Unique tile identifier

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TileName;
    // Human-readable name

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;
    // Visual representation

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight;
    // Weight for random selection
    // Higher = more common

    // Adjacency rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<int32> ValidNorth;
    // Tile IDs that can be north neighbors

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<int32> ValidEast;
    // Tile IDs that can be east neighbors

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<int32> ValidSouth;
    // Tile IDs that can be south neighbors

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<int32> ValidWest;
    // Tile IDs that can be west neighbors
};
```

---

### FWFCCell

WFC grid cell.

```cpp
USTRUCT(BlueprintType)
struct FWFCCell
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TSet<int32> PossibleTiles;
    // Set of possible tile IDs

    UPROPERTY(BlueprintReadWrite)
    bool bCollapsed;
    // True if collapsed to single tile

    int32 GetEntropy() const
    {
        return PossibleTiles.Num();
    }
};
```

---

## 🎨 Enumerations

### EBiomeType

Biome types based on Whittaker diagram.

```cpp
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Ocean               UMETA(DisplayName = "Ocean"),
    Beach               UMETA(DisplayName = "Beach"),
    Desert              UMETA(DisplayName = "Desert"),
    Grassland           UMETA(DisplayName = "Grassland"),
    Shrubland           UMETA(DisplayName = "Shrubland"),
    TemperateForest     UMETA(DisplayName = "Temperate Forest"),
    TropicalRainforest  UMETA(DisplayName = "Tropical Rainforest"),
    Savanna             UMETA(DisplayName = "Savanna"),
    Tundra              UMETA(DisplayName = "Tundra"),
    Taiga               UMETA(DisplayName = "Taiga"),
    Alpine              UMETA(DisplayName = "Alpine"),
    Snow                UMETA(DisplayName = "Snow")
};
```

---

### ECaveMeshMode

Cave mesh generation mode.

```cpp
UENUM(BlueprintType)
enum class ECaveMeshMode : uint8
{
    MarchingSquares2D   UMETA(DisplayName = "Marching Squares (2D)"),
    MarchingCubes3D     UMETA(DisplayName = "Marching Cubes (3D)"),
    SimpleBlocks        UMETA(DisplayName = "Simple Voxel Blocks")
};
```

---

### EWFCDirection

Tile adjacency directions.

```cpp
UENUM(BlueprintType)
enum class EWFCDirection : uint8
{
    North   UMETA(DisplayName = "North"),
    East    UMETA(DisplayName = "East"),
    South   UMETA(DisplayName = "South"),
    West    UMETA(DisplayName = "West")
};
```

---

### ETerrainTool

Editor mode tools.

```cpp
UENUM(BlueprintType)
enum class ETerrainTool : uint8
{
    Paint   UMETA(DisplayName = "Paint"),
    Sculpt  UMETA(DisplayName = "Sculpt"),
    Smooth  UMETA(DisplayName = "Smooth"),
    Flatten UMETA(DisplayName = "Flatten"),
    Generate UMETA(DisplayName = "Generate")
};
```

---

<div align="center">

**For implementation details, see [DOCUMENTATION.md](./DOCUMENTATION.md)**

[⬆ Back to Top](#-api-reference) | [README](./README.md) | [Quick Start](./QUICK_START.md)

</div>
