# 📚 Ultimate Procedural Map - Technical Documentation

> **Complete technical reference for developers**

---

## 📑 Table of Contents

1. [Architecture Overview](#-architecture-overview)
2. [Core Systems](#-core-systems)
3. [Biome System](#-biome-system)
4. [Erosion System](#-erosion-system)
5. [Wave Function Collapse](#-wave-function-collapse)
6. [River & Road Generation](#-river--road-generation)
7. [Cave Generation](#-cave-generation)
8. [Foliage System](#-foliage-system)
9. [GPU Compute Shaders](#-gpu-compute-shaders)
10. [Material System](#-material-system)
11. [Editor Tools](#-editor-tools)
12. [API Reference](#-api-reference)

---

## 🏗️ Architecture Overview

### System Design Philosophy

The Ultimate Procedural Map system follows these core principles:

1. **Modularity** - Each system can work independently or together
2. **Performance** - GPU acceleration where possible, async execution
3. **Flexibility** - Full Blueprint and C++ support
4. **Scalability** - From small scenes to massive open worlds
5. **Quality** - Production-ready code with extensive error handling

### Module Structure

```
ProceduralMap/
├── Source/
│   ├── ProceduralMap/              # Runtime Module
│   │   ├── Public/                 # Public headers
│   │   │   ├── QuadTreeTerrain.h   # Core terrain system
│   │   │   ├── LODManager.h        # LOD management
│   │   │   ├── BiomeGenerator.h    # Biome generation
│   │   │   ├── HydraulicErosion.h  # Erosion systems
│   │   │   ├── WaveFunctionCollapse.h
│   │   │   ├── RiverGenerator.h
│   │   │   ├── CaveGenerator.h
│   │   │   └── ProceduralFoliageSpawner.h
│   │   └── Private/                # Implementation files
│   │
│   ├── ProceduralMapEditor/        # Editor-Only Module
│   │   ├── Public/
│   │   │   ├── ProceduralMapEditorMode.h
│   │   │   ├── TerrainPreviewWidget.h
│   │   │   ├── ProceduralMapSettings.h
│   │   │   └── TerrainPreset.h
│   │   └── Private/
│   │
│   └── ThirdParty/
│       └── FastNoiseLite/          # Noise generation library
│
├── Shaders/Private/                # HLSL Compute Shaders
│   ├── TerrainComputeShader.usf
│   ├── ErosionComputeShader.usf
│   ├── BiomeComputeShader.usf
│   ├── NormalComputeShader.usf
│   ├── TriplanarProjection.ush
│   ├── HeightBlending.ush
│   └── SlopeMapping.ush
│
└── Content/
    ├── Materials/
    ├── Textures/
    ├── Blueprints/
    └── Examples/
```

### Dependencies

**Runtime Dependencies:**
- `CoreUObject` - UE core object system
- `Engine` - Unreal Engine runtime
- `ProceduralMeshComponent` - Dynamic mesh generation
- `RenderCore` - GPU compute shader support
- `RHI` - Rendering Hardware Interface

**Editor Dependencies:**
- `UnrealEd` - Editor framework
- `Slate` & `SlateCore` - UI framework
- `PropertyEditor` - Details panel customization
- `AssetTools` - Asset management
- `Blutility` - Editor utility widgets

---

## 🎮 Core Systems

### QuadTree Terrain System

The foundation of the procedural map, providing infinite terrain with dynamic LOD.

#### Class: `UQuadTreeTerrain`

**Purpose**: Main terrain component managing QuadTree structure and mesh generation.

**Key Features:**
- Adaptive QuadTree subdivision based on camera distance
- Dynamic LOD with configurable levels (0-8)
- Async mesh generation for smooth performance
- Frustum culling for optimal rendering
- Memory pooling for mesh components

#### Data Structures

##### `FQuadTreeNode`
```cpp
struct FQuadTreeNode
{
    FVector2D Center;           // Center position in 2D
    float Size;                 // Node size (world units)
    int32 LODLevel;             // Current LOD level (0 = highest detail)

    TSharedPtr<FQuadTreeNode> Children[4];  // NW, NE, SW, SE children
    UProceduralMeshComponent* MeshComponent; // Generated mesh

    bool bIsLeaf;               // True if has no children
    bool bNeedsMeshUpdate;      // Flag for regeneration
    float LastUpdateTime;       // For temporal coherence
};
```

##### `FTerrainGenerationParams`
```cpp
USTRUCT(BlueprintType)
struct FTerrainGenerationParams
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 Seed;                 // Noise seed for reproducibility

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Frequency;            // Base noise frequency (0.001-0.1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 Octaves;              // Fractal octaves (1-8)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Lacunarity;           // Frequency multiplier per octave (1.5-3.0)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Gain;                 // Amplitude multiplier per octave (0.3-0.7)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale;          // Vertical scale (100-5000)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightOffset;         // Base height offset
};
```

#### Algorithm Flow

1. **Initialization**
   ```
   InitializeTerrain()
   ├── Create root QuadTree node
   ├── Initialize FastNoiseLite with parameters
   ├── Setup LOD manager
   └── Create initial mesh
   ```

2. **Update Loop (Every Frame)**
   ```
   TickComponent()
   ├── Get camera position
   ├── Check update frequency throttle
   ├── UpdateLOD(CameraPosition)
   │   ├── Recursive node evaluation
   │   ├── Split nodes if too close
   │   ├── Merge nodes if too far
   │   └── Queue mesh updates
   └── ProcessUpdateQueue()
       └── Generate meshes (limited per frame)
   ```

3. **LOD Calculation**
   ```cpp
   float distance = FVector2D::Distance(NodeCenter, CameraPosition2D);
   float lodThreshold = NodeSize * LODDistanceMultiplier * (1 << LODLevel);

   if (distance < lodThreshold && LODLevel < MaxLODLevel)
   {
       SplitNode();  // Increase detail
   }
   else if (distance > lodThreshold * 2.0f && LODLevel > 0)
   {
       MergeNode();  // Decrease detail
   }
   ```

#### Mesh Generation

**Vertex Grid Creation:**
```cpp
void UQuadTreeTerrain::GenerateMeshForNode(TSharedPtr<FQuadTreeNode> Node)
{
    // Calculate resolution based on LOD
    int32 Resolution = MeshResolution >> Node->LODLevel;

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;

    // Generate vertex grid
    float Step = Node->Size / Resolution;
    for (int32 Y = 0; Y <= Resolution; ++Y)
    {
        for (int32 X = 0; X <= Resolution; ++X)
        {
            FVector2D WorldPos = Node->Center - FVector2D(Node->Size * 0.5f)
                                 + FVector2D(X * Step, Y * Step);

            float Height = CalculateHeightAt(WorldPos);
            Vertices.Add(FVector(WorldPos.X, WorldPos.Y, Height));

            // Calculate normal
            FVector Normal = CalculateNormalAt(WorldPos);
            Normals.Add(Normal);

            // Generate UVs
            UVs.Add(FVector2D(X / (float)Resolution, Y / (float)Resolution));
        }
    }

    // Generate triangles (quad strip)
    for (int32 Y = 0; Y < Resolution; ++Y)
    {
        for (int32 X = 0; X < Resolution; ++X)
        {
            int32 i = Y * (Resolution + 1) + X;

            // Triangle 1
            Triangles.Add(i);
            Triangles.Add(i + Resolution + 1);
            Triangles.Add(i + 1);

            // Triangle 2
            Triangles.Add(i + 1);
            Triangles.Add(i + Resolution + 1);
            Triangles.Add(i + Resolution + 2);
        }
    }

    // Create procedural mesh
    Node->MeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles,
        Normals, UVs, TArray<FLinearColor>(), TArray<FProcMeshTangent>(),
        bEnableCollision);
}
```

#### Performance Optimizations

**Frustum Culling:**
```cpp
bool UQuadTreeTerrain::IsNodeVisible(TSharedPtr<FQuadTreeNode> Node) const
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return true;

    FVector Center3D(Node->Center.X, Node->Center.Y, 0.0f);
    float Radius = Node->Size * 0.707f; // Diagonal radius

    return PC->PlayerCameraManager->GetCameraCachePOV().Location
        .DistSquared(Center3D) < FMath::Square(ViewDistance + Radius);
}
```

**Mesh Pooling:**
```cpp
// Reuse mesh components instead of creating new ones
UProceduralMeshComponent* UQuadTreeTerrain::GetOrCreateMeshComponent(
    TSharedPtr<FQuadTreeNode> Node)
{
    if (!MeshComponentPool.IsEmpty())
    {
        UProceduralMeshComponent* Mesh = MeshComponentPool.Pop();
        Mesh->SetVisibility(true);
        return Mesh;
    }

    return CreateMeshComponent(Node);
}
```

---

### LOD Manager

Centralized LOD configuration and statistics tracking.

#### Class: `ULODManager`

**Purpose**: Manage LOD settings per biome and track performance statistics.

#### LOD Configuration

```cpp
USTRUCT(BlueprintType)
struct FLODConfiguration
{
    // Distance thresholds for each LOD level
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> LODDistances;

    // Mesh resolution at each LOD level
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> MeshResolutions;

    // Whether to enable collision at this LOD
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> CollisionEnabled;
};
```

#### Statistics Tracking

```cpp
USTRUCT(BlueprintType)
struct FLODStatistics
{
    UPROPERTY(BlueprintReadOnly)
    int32 TotalNodes;           // Total QuadTree nodes

    UPROPERTY(BlueprintReadOnly)
    int32 VisibleNodes;         // Currently visible nodes

    UPROPERTY(BlueprintReadOnly)
    int32 TotalVertices;        // Total vertices rendered

    UPROPERTY(BlueprintReadOnly)
    int32 TotalTriangles;       // Total triangles rendered

    UPROPERTY(BlueprintReadOnly)
    TMap<int32, int32> NodesPerLOD;  // Node count per LOD level

    UPROPERTY(BlueprintReadOnly)
    float UpdateTime;           // Last update duration (ms)

    UPROPERTY(BlueprintReadOnly)
    int32 MeshUpdatesThisFrame; // Meshes generated this frame
};
```

---

## 🌿 Biome System

Whittaker diagram-based biome generation with smooth transitions.

### Architecture

```
BiomeSystem
├── BiomeGenerator (Climate Calculation)
│   ├── Temperature Maps
│   ├── Humidity Maps
│   └── Whittaker Classification
│
├── BiomeBlender (Smooth Transitions)
│   ├── Multi-biome Sampling
│   ├── Weight Calculation
│   └── Parameter Blending
│
└── BiomeConfig (Biome Definitions)
    ├── Biome Settings
    ├── Material Parameters
    └── Vegetation Rules
```

### Class: `ABiomeGenerator`

#### Climate Calculation

**Temperature Formula:**
```cpp
float ABiomeGenerator::CalculateTemperature(FVector Position, float Elevation)
{
    // Base temperature from latitude (0 at poles, 1 at equator)
    float LatitudeFactor = GetLatitudeFactor(Position);

    // Elevation effect (-6.5°C per 1000m in real world)
    float ElevationFactor = FMath::Clamp(1.0f - (Elevation * 0.5f), 0.0f, 1.0f);

    // Noise variation for realism
    float NoiseValue = GenerateNoise(
        Position.X,
        Position.Y,
        TemperatureNoise
    );

    // Combine factors
    float Temperature = (LatitudeFactor * 0.7f +
                        ElevationFactor * 0.3f +
                        NoiseValue * 0.1f);

    return FMath::Clamp(Temperature * TemperatureScale, 0.0f, 1.0f);
}
```

**Humidity Formula:**
```cpp
float ABiomeGenerator::CalculateHumidity(FVector Position, float DistanceToWater)
{
    // Distance to water effect (exponential falloff)
    float WaterFactor = FMath::Exp(-DistanceToWater / 5000.0f);

    // Noise for precipitation variation
    float NoiseValue = GenerateNoise(
        Position.X,
        Position.Y,
        HumidityNoise
    );

    // Combine
    float Humidity = (WaterFactor * 0.6f +
                     (NoiseValue * 0.5f + 0.5f) * 0.4f);

    return FMath::Clamp(Humidity * HumidityScale, 0.0f, 1.0f);
}
```

#### Whittaker Diagram Classification

**Biome Types:**
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

**Classification Algorithm:**
```cpp
EBiomeType ABiomeGenerator::GetBiomeFromClimate(
    float Temperature,
    float Humidity,
    float Elevation)
{
    // Elevation-based overrides
    if (Elevation > 0.9f) return EBiomeType::Snow;
    if (Elevation > 0.75f) return EBiomeType::Alpine;
    if (Elevation < 0.1f) return EBiomeType::Ocean;
    if (Elevation < 0.15f) return EBiomeType::Beach;

    // Whittaker diagram logic
    if (Temperature < 0.2f)  // Cold
    {
        return (Humidity > 0.5f) ? EBiomeType::Tundra : EBiomeType::Snow;
    }
    else if (Temperature < 0.5f)  // Temperate
    {
        if (Humidity < 0.2f) return EBiomeType::Desert;
        if (Humidity < 0.5f) return EBiomeType::Grassland;
        if (Humidity < 0.7f) return EBiomeType::Shrubland;
        return EBiomeType::TemperateForest;
    }
    else  // Hot
    {
        if (Humidity < 0.2f) return EBiomeType::Desert;
        if (Humidity < 0.5f) return EBiomeType::Savanna;
        return EBiomeType::TropicalRainforest;
    }
}
```

### Class: `UBiomeBlender`

**Multi-Biome Sampling:**
```cpp
TArray<FBiomeData> UBiomeBlender::CalculateBlendedBiomes(
    FVector Position,
    float BlendRadius)
{
    TArray<FBiomeData> BiomeWeights;

    // Sample biomes in a grid around the position
    const int32 SampleCount = 5;
    for (int32 X = -SampleCount/2; X <= SampleCount/2; ++X)
    {
        for (int32 Y = -SampleCount/2; Y <= SampleCount/2; ++Y)
        {
            FVector SamplePos = Position + FVector(
                X * BlendRadius,
                Y * BlendRadius,
                0.0f
            );

            // Get biome at sample position
            EBiomeType Biome = BiomeGenerator->GetBiomeAt(SamplePos);

            // Calculate weight based on distance
            float Distance = FVector2D(X, Y).Size();
            float Weight = 1.0f - FMath::Clamp(Distance / SampleCount, 0.0f, 1.0f);

            // Add or accumulate weight
            AddBiomeWeight(BiomeWeights, Biome, Weight);
        }
    }

    // Normalize weights
    NormalizeWeights(BiomeWeights);

    return BiomeWeights;
}
```

### Biome Configuration Asset

```cpp
USTRUCT(BlueprintType)
struct FBiomeSettings
{
    // Visual properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor BaseColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UTexture2D* AlbedoTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UTexture2D* NormalTexture;

    // Climate parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MinTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MaxTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MinHumidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MaxHumidity;

    // Terrain modification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Roughness;

    // Vegetation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationType> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity;
};
```

---

## 💧 Erosion System

Realistic terrain erosion using hydraulic and thermal simulations.

### Hydraulic Erosion

Based on particle-based droplet simulation.

#### Class: `UHydraulicErosion`

**Algorithm Overview:**
1. Spawn droplet at random position
2. Calculate height and gradient
3. Move droplet downhill
4. Erode or deposit sediment
5. Evaporate water
6. Repeat until water depleted

**Core Simulation Loop:**
```cpp
void UHydraulicErosion::SimulateDroplet(
    TArray<float>& HeightMap,
    int32 MapSize,
    FRandomStream& Random)
{
    // Initialize droplet
    FVector2D Position(
        Random.FRandRange(0.0f, MapSize - 1.0f),
        Random.FRandRange(0.0f, MapSize - 1.0f)
    );

    FVector2D Direction = FVector2D::ZeroVector;
    float Speed = InitialSpeed;
    float Water = InitialWater;
    float Sediment = 0.0f;

    for (int32 Lifetime = 0; Lifetime < MaxDropletLifetime; ++Lifetime)
    {
        // Get height and gradient at position
        float Height;
        FVector2D Gradient;
        InterpolateHeightAndGradient(HeightMap, MapSize, Position, Height, Gradient);

        // Update direction (mix gradient with inertia)
        Direction = Direction * Inertia - Gradient * (1.0f - Inertia);
        Direction.Normalize();

        // Update position
        FVector2D NewPosition = Position + Direction;

        // Check if still in bounds
        if (!IsInBounds(NewPosition, MapSize))
            break;

        // Calculate height difference
        float NewHeight = InterpolateHeight(HeightMap, MapSize, NewPosition);
        float DeltaHeight = NewHeight - Height;

        // Calculate sediment capacity
        float Capacity = FMath::Max(-DeltaHeight, MinSlope) * Speed * Water
                        * SedimentCapacityFactor;

        // Erode or deposit
        if (Sediment > Capacity || DeltaHeight > 0)
        {
            // Deposit sediment
            float AmountToDeposit = (DeltaHeight > 0) ?
                FMath::Min(DeltaHeight, Sediment) :
                (Sediment - Capacity) * DepositSpeed;

            Sediment -= AmountToDeposit;
            DepositSediment(HeightMap, MapSize, Position, AmountToDeposit);
        }
        else
        {
            // Erode terrain
            float AmountToErode = FMath::Min(
                (Capacity - Sediment) * ErodeSpeed,
                -DeltaHeight
            );

            ErodeTerrain(HeightMap, MapSize, Position, AmountToErode);
            Sediment += AmountToErode;
        }

        // Update speed and water
        Speed = FMath::Sqrt(Speed * Speed + DeltaHeight * Gravity);
        Water *= (1.0f - EvaporateSpeed);

        // Move to new position
        Position = NewPosition;

        // Stop if no water left
        if (Water < 0.01f)
            break;
    }
}
```

**Erosion Brush:**
```cpp
void UHydraulicErosion::ErodeTerrain(
    TArray<float>& HeightMap,
    int32 MapSize,
    FVector2D Position,
    float Amount)
{
    int32 CenterX = FMath::RoundToInt(Position.X);
    int32 CenterY = FMath::RoundToInt(Position.Y);

    // Apply erosion in circular brush pattern
    for (const FBrushIndex& Brush : BrushIndices)
    {
        int32 X = CenterX + Brush.X;
        int32 Y = CenterY + Brush.Y;

        if (IsInBounds(X, Y, MapSize))
        {
            int32 Index = Y * MapSize + X;
            HeightMap[Index] -= Amount * Brush.Weight;
        }
    }
}
```

**Brush Initialization:**
```cpp
void UHydraulicErosion::InitializeBrushIndices()
{
    BrushIndices.Empty();

    for (int32 Y = -ErosionRadius; Y <= ErosionRadius; ++Y)
    {
        for (int32 X = -ErosionRadius; X <= ErosionRadius; ++X)
        {
            float Distance = FMath::Sqrt(X * X + Y * Y);

            if (Distance <= ErosionRadius)
            {
                FBrushIndex Brush;
                Brush.X = X;
                Brush.Y = Y;

                // Gaussian falloff
                Brush.Weight = FMath::Exp(-Distance * Distance /
                              (2.0f * ErosionRadius * ErosionRadius));

                BrushIndices.Add(Brush);
            }
        }
    }
}
```

### GPU Erosion

**Compute Shader Implementation:**

File: `ErosionComputeShader.usf`
```hlsl
// Erosion parameters
cbuffer ErosionParams : register(b0)
{
    uint MapSize;
    uint NumDroplets;
    float Inertia;
    float SedimentCapacityFactor;
    float MinSedimentCapacity;
    float ErodeSpeed;
    float DepositSpeed;
    float EvaporateSpeed;
    float Gravity;
    int ErosionRadius;
    int MaxDropletLifetime;
    uint RandomSeed;
};

// Heightmap textures
RWTexture2D<float> HeightMap : register(u0);
RWTexture2D<float> SedimentMap : register(u1);

// Random number generation (PCG)
uint PCGHash(uint seed)
{
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float RandomFloat(inout uint seed)
{
    seed = PCGHash(seed);
    return seed / 4294967295.0;
}

[numthreads(8, 8, 1)]
void ErosionSimulation(uint3 ThreadID : SV_DispatchThreadID)
{
    uint DropletIndex = ThreadID.x + ThreadID.y * 8;
    if (DropletIndex >= NumDroplets)
        return;

    // Initialize random seed per droplet
    uint Seed = RandomSeed + DropletIndex;

    // Random starting position
    float2 Position = float2(
        RandomFloat(Seed) * (MapSize - 1),
        RandomFloat(Seed) * (MapSize - 1)
    );

    float2 Direction = float2(0, 0);
    float Speed = 1.0;
    float Water = 1.0;
    float Sediment = 0.0;

    for (int Lifetime = 0; Lifetime < MaxDropletLifetime; ++Lifetime)
    {
        // Sample height and gradient
        int2 Coord = int2(Position);
        float Height = HeightMap[Coord];

        // Calculate gradient using finite differences
        float2 Gradient = float2(
            HeightMap[Coord + int2(1, 0)] - HeightMap[Coord - int2(1, 0)],
            HeightMap[Coord + int2(0, 1)] - HeightMap[Coord - int2(0, 1)]
        ) * 0.5;

        // Update direction with inertia
        Direction = Direction * Inertia - Gradient * (1.0 - Inertia);
        Direction = normalize(Direction);

        // Move droplet
        float2 NewPosition = Position + Direction;

        // Boundary check
        if (any(NewPosition < 0) || any(NewPosition >= MapSize - 1))
            break;

        // Sample new height
        float NewHeight = HeightMap[int2(NewPosition)];
        float DeltaHeight = NewHeight - Height;

        // Calculate sediment capacity
        float Capacity = max(-DeltaHeight, 0.01) * Speed * Water
                        * SedimentCapacityFactor;

        // Erosion or deposition
        if (Sediment > Capacity || DeltaHeight > 0)
        {
            float Deposit = (DeltaHeight > 0) ?
                min(DeltaHeight, Sediment) :
                (Sediment - Capacity) * DepositSpeed;

            Sediment -= Deposit;

            // Atomic add for thread safety
            InterlockedAdd(HeightMap[Coord], Deposit);
        }
        else
        {
            float Erode = min(
                (Capacity - Sediment) * ErodeSpeed,
                -DeltaHeight
            );

            Sediment += Erode;

            // Atomic subtract for thread safety
            InterlockedAdd(HeightMap[Coord], -Erode);
        }

        // Update physics
        Speed = sqrt(Speed * Speed + max(DeltaHeight * Gravity, 0));
        Water *= (1.0 - EvaporateSpeed);

        Position = NewPosition;

        if (Water < 0.01)
            break;
    }
}
```

### Thermal Erosion

Slope-based material transfer for realistic scree formation.

#### Class: `UThermalErosion`

**Algorithm:**
```cpp
void UThermalErosion::Erode(TArray<float>& HeightMap, int32 MapSize, bool bAsync)
{
    TArray<float> DeltaMap;
    DeltaMap.SetNumZeroed(MapSize * MapSize);

    for (int32 Iteration = 0; Iteration < NumIterations; ++Iteration)
    {
        // Calculate material transfer
        for (int32 Y = 1; Y < MapSize - 1; ++Y)
        {
            for (int32 X = 1; X < MapSize - 1; ++X)
            {
                int32 Index = Y * MapSize + X;
                float Height = HeightMap[Index];
                float TotalDelta = 0.0f;

                // Check all neighbors
                TArray<int32> Neighbors = bUse8Neighbors ?
                    Get8Neighbors(X, Y, MapSize) :
                    Get4Neighbors(X, Y, MapSize);

                for (int32 NeighborIndex : Neighbors)
                {
                    float NeighborHeight = HeightMap[NeighborIndex];
                    float HeightDiff = Height - NeighborHeight;
                    float Distance = GetNeighborDistance(Index, NeighborIndex, MapSize);

                    // Calculate slope angle
                    float Angle = FMath::Atan(HeightDiff / (Distance * CellSize))
                                * 180.0f / PI;

                    // If slope exceeds talus angle, move material
                    if (Angle > TalusAngle)
                    {
                        float AmountToMove = HeightDiff * ErosionRate;
                        TotalDelta -= AmountToMove;
                        DeltaMap[NeighborIndex] += AmountToMove / Neighbors.Num();
                    }
                }

                DeltaMap[Index] += TotalDelta;
            }
        }

        // Apply changes
        for (int32 i = 0; i < HeightMap.Num(); ++i)
        {
            HeightMap[i] += DeltaMap[i];
            DeltaMap[i] = 0.0f;  // Reset for next iteration
        }
    }
}
```

---

## 🎲 Wave Function Collapse

Constraint-based procedural generation for structures.

### Class: `UWaveFunctionCollapse`

#### Core Algorithm

**1. Initialization:**
```cpp
void UWaveFunctionCollapse::InitializeGrid()
{
    Grid.SetNum(GridWidth * GridHeight);

    for (FWFCCell& Cell : Grid)
    {
        Cell.PossibleTiles.Empty();
        Cell.bCollapsed = false;

        // All tiles are possible initially
        for (const FWFCTile& Tile : TileSet)
        {
            Cell.PossibleTiles.Add(Tile.TileID);
        }
    }
}
```

**2. Find Lowest Entropy:**
```cpp
bool UWaveFunctionCollapse::FindLowestEntropyCell(int32& OutX, int32& OutY)
{
    int32 MinEntropy = TileSet.Num() + 1;
    TArray<FIntPoint> MinEntropyCells;

    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            FWFCCell* Cell = GetCell(X, Y);

            if (!Cell->bCollapsed)
            {
                int32 Entropy = Cell->PossibleTiles.Num();

                if (Entropy < MinEntropy && Entropy > 0)
                {
                    MinEntropy = Entropy;
                    MinEntropyCells.Empty();
                    MinEntropyCells.Add(FIntPoint(X, Y));
                }
                else if (Entropy == MinEntropy)
                {
                    MinEntropyCells.Add(FIntPoint(X, Y));
                }
            }
        }
    }

    if (MinEntropyCells.IsEmpty())
        return false;

    // Random selection from min entropy cells
    FIntPoint Selected = MinEntropyCells[RandomStream.RandRange(0, MinEntropyCells.Num() - 1)];
    OutX = Selected.X;
    OutY = Selected.Y;

    return true;
}
```

**3. Collapse Cell:**
```cpp
bool UWaveFunctionCollapse::CollapseCell(int32 X, int32 Y)
{
    FWFCCell* Cell = GetCell(X, Y);
    if (!Cell || Cell->bCollapsed || Cell->PossibleTiles.IsEmpty())
        return false;

    // Weighted random selection
    int32 SelectedTileID = WeightedRandomSelect(Cell->PossibleTiles);

    // Collapse to single tile
    Cell->PossibleTiles.Empty();
    Cell->PossibleTiles.Add(SelectedTileID);
    Cell->bCollapsed = true;

    return true;
}
```

**4. Propagate Constraints:**
```cpp
bool UWaveFunctionCollapse::Propagate(int32 StartX, int32 StartY)
{
    TQueue<FIntPoint> PropagationQueue;
    PropagationQueue.Enqueue(FIntPoint(StartX, StartY));

    while (!PropagationQueue.IsEmpty())
    {
        FIntPoint Current;
        PropagationQueue.Dequeue(Current);

        // Check all 4 directions
        for (EWFCDirection Dir = EWFCDirection::North;
             Dir <= EWFCDirection::West;
             Dir = (EWFCDirection)((int32)Dir + 1))
        {
            int32 NeighborX, NeighborY;
            if (GetNeighborPosition(Current.X, Current.Y, Dir, NeighborX, NeighborY))
            {
                FWFCCell* Neighbor = GetCell(NeighborX, NeighborY);

                if (!Neighbor->bCollapsed)
                {
                    int32 PreviousCount = Neighbor->PossibleTiles.Num();

                    // Constrain neighbor based on current cell
                    if (!ConstrainNeighbor(NeighborX, NeighborY))
                        return false;  // Contradiction!

                    // If possibilities changed, add to queue
                    if (Neighbor->PossibleTiles.Num() < PreviousCount)
                    {
                        PropagationQueue.Enqueue(FIntPoint(NeighborX, NeighborY));
                    }
                }
            }
        }
    }

    return true;
}
```

**5. Constrain Neighbor:**
```cpp
bool UWaveFunctionCollapse::ConstrainNeighbor(int32 X, int32 Y)
{
    FWFCCell* Cell = GetCell(X, Y);
    if (!Cell || Cell->bCollapsed)
        return true;

    TSet<int32> ValidTiles;

    // Collect valid tiles from all neighbors
    for (EWFCDirection Dir = EWFCDirection::North;
         Dir <= EWFCDirection::West;
         Dir = (EWFCDirection)((int32)Dir + 1))
    {
        TSet<int32> ValidFromDirection = GetValidNeighborTiles(X, Y, Dir);

        if (ValidTiles.IsEmpty())
            ValidTiles = ValidFromDirection;
        else
            ValidTiles = ValidTiles.Intersect(ValidFromDirection);
    }

    // Intersect with current possibilities
    Cell->PossibleTiles = Cell->PossibleTiles.Intersect(ValidTiles);

    // Check for contradiction
    return !Cell->PossibleTiles.IsEmpty();
}
```

#### Tile Definition

```cpp
USTRUCT(BlueprintType)
struct FWFCTile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TileID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight;  // For weighted random selection

    // Adjacency rules (which tiles can be neighbors)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<int32> ValidNorth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<int32> ValidEast;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<int32> ValidSouth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<int32> ValidWest;
};
```

#### Backtracking

```cpp
bool UWaveFunctionCollapse::Generate()
{
    InitializeGrid();
    RandomStream.Initialize(RandomSeed);

    CurrentIteration = 0;

    while (!IsFullyCollapsed() && CurrentIteration < MaxIterations)
    {
        // Save state for backtracking
        if (bEnableBacktracking)
            SaveState();

        // Find cell with minimum entropy
        int32 X, Y;
        if (!FindLowestEntropyCell(X, Y))
            break;  // All collapsed

        // Collapse cell
        if (!CollapseCell(X, Y))
        {
            if (bEnableBacktracking && RestoreState())
                continue;  // Try again from previous state

            return false;  // Failed
        }

        // Propagate constraints
        if (!Propagate(X, Y))
        {
            if (bEnableBacktracking && RestoreState())
                continue;  // Backtrack

            return false;  // Contradiction
        }

        CurrentIteration++;
    }

    return IsFullyCollapsed();
}
```

---

## 🌊 River & Road Generation

Spline-based procedural generation with pathfinding.

### Class: `ARiverGenerator`

#### A* Pathfinding with Downhill Bias

```cpp
TArray<FVector> ARiverGenerator::FindPathAStar(const FVector& Start, const FVector& End)
{
    TArray<FRiverPathNode> OpenSet;
    TArray<FRiverPathNode> ClosedSet;

    // Initialize start node
    FRiverPathNode StartNode;
    StartNode.Position = Start;
    StartNode.GCost = 0.0f;
    StartNode.HCost = CalculateHeuristic(Start, End);
    StartNode.ParentIndex = -1;

    OpenSet.Add(StartNode);

    while (!OpenSet.IsEmpty())
    {
        // Find node with lowest F cost
        int32 CurrentIndex = 0;
        float LowestFCost = OpenSet[0].GetFCost();

        for (int32 i = 1; i < OpenSet.Num(); ++i)
        {
            float FCost = OpenSet[i].GetFCost();
            if (FCost < LowestFCost)
            {
                CurrentIndex = i;
                LowestFCost = FCost;
            }
        }

        FRiverPathNode Current = OpenSet[CurrentIndex];
        OpenSet.RemoveAt(CurrentIndex);
        ClosedSet.Add(Current);

        // Check if reached goal
        if (FVector::Dist(Current.Position, End) < PathfindingStepSize)
        {
            return ReconstructPath(ClosedSet, ClosedSet.Num() - 1);
        }

        // Explore neighbors
        TArray<FVector> Neighbors = GetNeighbors(Current.Position);

        for (const FVector& NeighborPos : Neighbors)
        {
            // Skip if in closed set
            if (IsInClosedSet(NeighborPos, ClosedSet))
                continue;

            float NeighborHeight = GetTerrainHeight(NeighborPos);
            float CurrentHeight = GetTerrainHeight(Current.Position);
            float HeightDiff = NeighborHeight - CurrentHeight;

            // Cost calculation (prefer downhill)
            float MoveCost = PathfindingStepSize;

            // Add penalty for uphill movement
            if (HeightDiff > 0)
                MoveCost += HeightDiff * 10.0f * DownhillBias;
            else
                MoveCost += HeightDiff * -0.5f;  // Bonus for downhill

            float NewGCost = Current.GCost + MoveCost;

            // Check if in open set
            int32 ExistingIndex = FindInOpenSet(NeighborPos, OpenSet);

            if (ExistingIndex == -1)
            {
                // Add new node to open set
                FRiverPathNode NewNode;
                NewNode.Position = NeighborPos;
                NewNode.GCost = NewGCost;
                NewNode.HCost = CalculateHeuristic(NeighborPos, End);
                NewNode.ParentIndex = ClosedSet.Num() - 1;

                OpenSet.Add(NewNode);
            }
            else if (NewGCost < OpenSet[ExistingIndex].GCost)
            {
                // Update existing node
                OpenSet[ExistingIndex].GCost = NewGCost;
                OpenSet[ExistingIndex].ParentIndex = ClosedSet.Num() - 1;
            }
        }
    }

    // No path found, return empty
    return TArray<FVector>();
}
```

#### Curl Noise for Natural Meandering

```cpp
void ARiverGenerator::ApplyCurlNoise()
{
    if (!bApplyCurlNoise || !RiverSpline)
        return;

    int32 NumPoints = RiverSpline->GetNumberOfSplinePoints();

    for (int32 i = 1; i < NumPoints - 1; ++i)
    {
        FVector OriginalPos = RiverSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

        // Calculate curl noise displacement
        FVector2D CurlOffset = CalculateCurlNoise(OriginalPos);

        // Apply perpendicular to spline direction
        FVector Tangent = RiverSpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
        FVector Right = FVector::CrossProduct(Tangent, FVector::UpVector).GetSafeNormal();

        FVector Displacement = Right * CurlOffset.X * NoiseAmplitude;

        RiverSpline->SetLocationAtSplinePoint(i, OriginalPos + Displacement,
            ESplineCoordinateSpace::World, false);
    }

    RiverSpline->UpdateSpline();
}

FVector2D ARiverGenerator::CalculateCurlNoise(const FVector& Position) const
{
    float Epsilon = 0.0001f;

    // Calculate partial derivatives for curl
    float DxDy = (GetNoise2D(Position.X + Epsilon, Position.Y) -
                 GetNoise2D(Position.X - Epsilon, Position.Y)) / (2.0f * Epsilon);

    float DyDx = (GetNoise2D(Position.X, Position.Y + Epsilon) -
                 GetNoise2D(Position.X, Position.Y - Epsilon)) / (2.0f * Epsilon);

    // Curl = (dψ/dy, -dψ/dx)
    return FVector2D(DyDx, -DxDy);
}
```

#### Landscape Modification

```cpp
void ARiverGenerator::ModifyLandscape()
{
    if (!bModifyLandscape || !TargetLandscape || !RiverSpline)
        return;

    int32 NumPoints = RiverSpline->GetNumberOfSplinePoints();

    for (int32 i = 0; i < NumPoints - 1; ++i)
    {
        FVector StartPos = RiverSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        FVector EndPos = RiverSpline->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

        float SegmentLength = FVector::Dist(StartPos, EndPos);
        int32 Steps = FMath::CeilToInt(SegmentLength / (Width * 0.5f));

        for (int32 Step = 0; Step <= Steps; ++Step)
        {
            float Alpha = Step / (float)Steps;
            FVector Position = FMath::Lerp(StartPos, EndPos, Alpha);

            // Target height (river bed depth)
            float TargetHeight = Position.Z - Depth;

            // Modify landscape at this point
            ModifyLandscapeAtPoint(Position, Width + SmoothingRadius, TargetHeight);
        }
    }
}

void ARiverGenerator::ModifyLandscapeAtPoint(
    const FVector& Position,
    float Radius,
    float TargetHeight)
{
    // This would interface with UE's landscape editing API
    // Pseudo-code for the concept:

    FLandscapeEditDataInterface LandscapeEdit(TargetLandscape->GetLandscapeInfo());

    int32 MinX = FMath::FloorToInt((Position.X - Radius) / LandscapeEdit.ComponentSizeQuads);
    int32 MaxX = FMath::CeilToInt((Position.X + Radius) / LandscapeEdit.ComponentSizeQuads);
    int32 MinY = FMath::FloorToInt((Position.Y - Radius) / LandscapeEdit.ComponentSizeQuads);
    int32 MaxY = FMath::CeilToInt((Position.Y + Radius) / LandscapeEdit.ComponentSizeQuads);

    for (int32 Y = MinY; Y <= MaxY; ++Y)
    {
        for (int32 X = MinX; X <= MaxX; ++X)
        {
            FVector VertexPos = LandscapeEdit.GetVertexPosition(X, Y);
            float Distance = FVector2D::Distance(
                FVector2D(Position),
                FVector2D(VertexPos)
            );

            if (Distance < Radius)
            {
                float CurrentHeight = VertexPos.Z;

                // Smooth falloff
                float Falloff = FMath::Pow(1.0f - (Distance / Radius), FalloffPower);
                float NewHeight = FMath::Lerp(CurrentHeight, TargetHeight, Falloff);

                LandscapeEdit.SetHeight(X, Y, NewHeight);
            }
        }
    }

    LandscapeEdit.Flush();
}
```

---

## 🕳️ Cave Generation

3D cave systems using cellular automata and marching cubes.

### Class: `UCellularAutomata`

#### Algorithm

```cpp
void UCellularAutomata::Generate()
{
    InitializeGrid();

    for (int32 Iteration = 0; Iteration < NumIterations; ++Iteration)
    {
        ApplyCellularAutomataRules();
    }

    if (bRemoveSmallRegions)
    {
        RemoveSmallRegions(MinRegionSize);
    }

    if (bConnectRegions)
    {
        ConnectClosestRegions();
    }
}

void UCellularAutomata::InitializeGrid()
{
    Grid.SetNum(GridWidth * GridHeight);
    FRandomStream Random(Seed);

    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            int32 Index = Y * GridWidth + X;

            // Border cells are always walls
            if (X == 0 || X == GridWidth - 1 || Y == 0 || Y == GridHeight - 1)
            {
                Grid[Index] = 1;  // Wall
            }
            else
            {
                // Random fill
                Grid[Index] = (Random.FRand() < FillPercent) ? 1 : 0;
            }
        }
    }
}

void UCellularAutomata::ApplyCellularAutomataRules()
{
    TArray<int32> NewGrid = Grid;

    for (int32 Y = 1; Y < GridHeight - 1; ++Y)
    {
        for (int32 X = 1; X < GridWidth - 1; ++X)
        {
            int32 Index = Y * GridWidth + X;
            int32 WallCount = CountWallNeighbors(X, Y);

            // Cellular automata rules
            if (WallCount > 4)
                NewGrid[Index] = 1;  // Become wall
            else if (WallCount < 4)
                NewGrid[Index] = 0;  // Become floor
            // else stay the same
        }
    }

    Grid = NewGrid;
}

int32 UCellularAutomata::CountWallNeighbors(int32 X, int32 Y) const
{
    int32 WallCount = 0;

    for (int32 DY = -1; DY <= 1; ++DY)
    {
        for (int32 DX = -1; DX <= 1; ++DX)
        {
            if (DX == 0 && DY == 0)
                continue;

            int32 NX = X + DX;
            int32 NY = Y + DY;

            if (NX >= 0 && NX < GridWidth && NY >= 0 && NY < GridHeight)
            {
                int32 Index = NY * GridWidth + NX;
                if (Grid[Index] == 1)
                    WallCount++;
            }
            else
            {
                WallCount++;  // Out of bounds counts as wall
            }
        }
    }

    return WallCount;
}
```

### Class: `ACaveGenerator`

#### Marching Cubes Implementation

```cpp
void ACaveGenerator::GenerateMarchingCubes3D()
{
    if (!CellularAutomata)
        return;

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;

    // Process each voxel
    for (int32 Z = 0; Z < CellularAutomata->GridHeight - 1; ++Z)
    {
        for (int32 Y = 0; Y < CellularAutomata->GridHeight - 1; ++Y)
        {
            for (int32 X = 0; X < CellularAutomata->GridWidth - 1; ++X)
            {
                ProcessVoxel(X, Y, Z, Vertices, Triangles, Normals);
            }
        }
    }

    // Calculate smooth normals
    CalculateSmoothNormals(Vertices, Triangles, Normals);

    // Generate UVs
    TArray<FVector2D> UVs;
    GenerateUVs(Vertices, UVs);

    // Create mesh
    CaveMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals,
        UVs, TArray<FLinearColor>(), TArray<FProcMeshTangent>(), bGenerateCollision);

    if (CaveMaterial)
    {
        CaveMesh->SetMaterial(0, CaveMaterial);
    }
}

void ACaveGenerator::ProcessVoxel(
    int32 X, int32 Y, int32 Z,
    TArray<FVector>& Vertices,
    TArray<int32>& Triangles,
    TArray<FVector>& Normals)
{
    // Get 8 corner values (0 = floor, 1 = wall)
    int32 CubeIndex = 0;

    if (GetVoxelValue(X, Y, Z))         CubeIndex |= 1;
    if (GetVoxelValue(X + 1, Y, Z))     CubeIndex |= 2;
    if (GetVoxelValue(X + 1, Y, Z + 1)) CubeIndex |= 4;
    if (GetVoxelValue(X, Y, Z + 1))     CubeIndex |= 8;
    if (GetVoxelValue(X, Y + 1, Z))     CubeIndex |= 16;
    if (GetVoxelValue(X + 1, Y + 1, Z)) CubeIndex |= 32;
    if (GetVoxelValue(X + 1, Y + 1, Z + 1)) CubeIndex |= 64;
    if (GetVoxelValue(X, Y + 1, Z + 1)) CubeIndex |= 128;

    // Lookup triangulation from marching cubes table
    const int32* Triangulation = MarchingCubesTable[CubeIndex];

    // Generate triangles
    for (int32 i = 0; Triangulation[i] != -1; i += 3)
    {
        int32 BaseIndex = Vertices.Num();

        // Get vertex positions from edge table
        for (int32 j = 0; j < 3; ++j)
        {
            int32 EdgeIndex = Triangulation[i + j];
            FVector VertexPos = GetEdgeVertex(X, Y, Z, EdgeIndex);
            Vertices.Add(VertexPos);
        }

        // Add triangle indices
        Triangles.Add(BaseIndex);
        Triangles.Add(BaseIndex + 1);
        Triangles.Add(BaseIndex + 2);
    }
}

FVector ACaveGenerator::GetEdgeVertex(int32 X, int32 Y, int32 Z, int32 EdgeIndex) const
{
    // Marching cubes edge vertices
    static const FVector EdgeVertices[12] = {
        FVector(0.5f, 0.0f, 0.0f),  // Edge 0
        FVector(1.0f, 0.0f, 0.5f),  // Edge 1
        FVector(0.5f, 0.0f, 1.0f),  // Edge 2
        FVector(0.0f, 0.0f, 0.5f),  // Edge 3
        FVector(0.5f, 1.0f, 0.0f),  // Edge 4
        FVector(1.0f, 1.0f, 0.5f),  // Edge 5
        FVector(0.5f, 1.0f, 1.0f),  // Edge 6
        FVector(0.0f, 1.0f, 0.5f),  // Edge 7
        FVector(0.0f, 0.5f, 0.0f),  // Edge 8
        FVector(1.0f, 0.5f, 0.0f),  // Edge 9
        FVector(1.0f, 0.5f, 1.0f),  // Edge 10
        FVector(0.0f, 0.5f, 1.0f)   // Edge 11
    };

    FVector BasePos = FVector(X, Y, Z) * VoxelSize;
    FVector Offset = EdgeVertices[EdgeIndex] * VoxelSize;

    return BasePos + Offset;
}
```

---

## 🌳 Foliage System

Procedural vegetation placement with biome integration.

### Class: `UProceduralFoliageSpawner`

```cpp
void UProceduralFoliageSpawner::SpawnFoliage(
    AActor* TargetTerrain,
    const TArray<FVegetationLayer>& Layers)
{
    for (const FVegetationLayer& Layer : Layers)
    {
        SpawnLayer(TargetTerrain, Layer);
    }
}

void UProceduralFoliageSpawner::SpawnLayer(
    AActor* TargetTerrain,
    const FVegetationLayer& Layer)
{
    // Get terrain bounds
    FBox Bounds = TargetTerrain->GetComponentsBoundingBox();

    // Calculate spawn grid
    int32 GridWidth = FMath::CeilToInt(Bounds.GetSize().X / Layer.GridCellSize);
    int32 GridHeight = FMath::CeilToInt(Bounds.GetSize().Y / Layer.GridCellSize);

    FRandomStream Random(Layer.Seed);

    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            // Cell position
            FVector CellCenter = Bounds.Min + FVector(
                (X + 0.5f) * Layer.GridCellSize,
                (Y + 0.5f) * Layer.GridCellSize,
                0.0f
            );

            // Density check
            float DensityNoise = GetDensityNoise(CellCenter, Layer);
            if (DensityNoise < Layer.DensityThreshold)
                continue;

            // Calculate instances per cell
            int32 InstancesInCell = FMath::RoundToInt(
                Layer.InstancesPerCell * DensityNoise
            );

            for (int32 i = 0; i < InstancesInCell; ++i)
            {
                // Random position within cell
                FVector Position = CellCenter + FVector(
                    Random.FRandRange(-Layer.GridCellSize * 0.5f, Layer.GridCellSize * 0.5f),
                    Random.FRandRange(-Layer.GridCellSize * 0.5f, Layer.GridCellSize * 0.5f),
                    0.0f
                );

                // Get terrain height
                float Height = GetTerrainHeightAt(Position, TargetTerrain);
                Position.Z = Height;

                // Slope filtering
                FVector Normal = GetTerrainNormalAt(Position, TargetTerrain);
                float Slope = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector))
                            * 180.0f / PI;

                if (Slope < Layer.MinSlope || Slope > Layer.MaxSlope)
                    continue;

                // Height filtering
                if (Height < Layer.MinHeight || Height > Layer.MaxHeight)
                    continue;

                // Spawn instance
                SpawnInstance(Position, Normal, Layer, Random);
            }
        }
    }
}

float UProceduralFoliageSpawner::GetDensityNoise(
    const FVector& Position,
    const FVegetationLayer& Layer) const
{
    // Use FastNoiseLite for density variation
    FastNoiseLite Noise;
    Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    Noise.SetFrequency(Layer.DensityNoiseFrequency);
    Noise.SetFractalOctaves(3);
    Noise.SetSeed(Layer.Seed);

    float NoiseValue = Noise.GetNoise(Position.X, Position.Y);

    // Remap from [-1, 1] to [0, 1]
    return (NoiseValue + 1.0f) * 0.5f;
}

void UProceduralFoliageSpawner::SpawnInstance(
    const FVector& Position,
    const FVector& Normal,
    const FVegetationLayer& Layer,
    FRandomStream& Random)
{
    // Random mesh from layer
    UStaticMesh* Mesh = Layer.Meshes[Random.RandRange(0, Layer.Meshes.Num() - 1)];

    // Random scale
    float Scale = Random.FRandRange(Layer.MinScale, Layer.MaxScale);

    // Align to normal
    FRotator Rotation = Normal.Rotation();

    // Random yaw
    if (Layer.bRandomYaw)
    {
        Rotation.Yaw = Random.FRandRange(0.0f, 360.0f);
    }

    // Spawn using instanced static mesh or HISM
    if (Layer.bUseInstancedMesh)
    {
        UHierarchicalInstancedStaticMeshComponent* HISM = GetOrCreateHISM(Mesh, Layer);

        FTransform Transform;
        Transform.SetLocation(Position);
        Transform.SetRotation(Rotation.Quaternion());
        Transform.SetScale3D(FVector(Scale));

        HISM->AddInstance(Transform);
    }
    else
    {
        // Spawn individual actor
        AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(
            Position, Rotation
        );

        Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        Actor->SetActorScale3D(FVector(Scale));
    }
}
```

---

## 🖥️ GPU Compute Shaders

High-performance HLSL compute shaders.

### Terrain Compute Shader

**File**: `TerrainComputeShader.usf`

```hlsl
#include "/Engine/Private/Common.ush"

// Parameters
cbuffer TerrainParams : register(b0)
{
    uint MapSize;
    uint Seed;
    float Frequency;
    uint Octaves;
    float Lacunarity;
    float Gain;
    float HeightScale;
    float HeightOffset;
};

// Output heightmap
RWTexture2D<float> OutputHeightMap : register(u0);

// Permutation table for noise
static const int Perm[512] = {
    // ... standard Perlin permutation table
};

// Gradient vectors
static const float3 Grad3[12] = {
    float3(1,1,0), float3(-1,1,0), float3(1,-1,0), float3(-1,-1,0),
    float3(1,0,1), float3(-1,0,1), float3(1,0,-1), float3(-1,0,-1),
    float3(0,1,1), float3(0,-1,1), float3(0,1,-1), float3(0,-1,-1)
};

// Perlin noise implementation
float PerlinNoise(float2 position)
{
    // Grid cell coordinates
    int2 cell = int2(floor(position));
    float2 frac = frac(position);

    // Smooth interpolation curve
    float2 u = frac * frac * (3.0 - 2.0 * frac);

    // Hash coordinates of 4 corners
    int aa = Perm[Perm[cell.x & 255] + (cell.y & 255)];
    int ab = Perm[Perm[cell.x & 255] + ((cell.y + 1) & 255)];
    int ba = Perm[Perm[(cell.x + 1) & 255] + (cell.y & 255)];
    int bb = Perm[Perm[(cell.x + 1) & 255] + ((cell.y + 1) & 255)];

    // Gradients at corners
    float3 g00 = Grad3[aa % 12];
    float3 g01 = Grad3[ab % 12];
    float3 g10 = Grad3[ba % 12];
    float3 g11 = Grad3[bb % 12];

    // Dot products
    float n00 = dot(g00, float3(frac.x, frac.y, 0));
    float n01 = dot(g01, float3(frac.x, frac.y - 1, 0));
    float n10 = dot(g10, float3(frac.x - 1, frac.y, 0));
    float n11 = dot(g11, float3(frac.x - 1, frac.y - 1, 0));

    // Bilinear interpolation
    float nx0 = lerp(n00, n10, u.x);
    float nx1 = lerp(n01, n11, u.x);
    float nxy = lerp(nx0, nx1, u.y);

    return nxy;
}

// Fractal Brownian Motion
float FBM(float2 position, uint octaves, float lacunarity, float gain)
{
    float total = 0.0;
    float frequency = 1.0;
    float amplitude = 1.0;
    float maxValue = 0.0;

    for (uint i = 0; i < octaves; ++i)
    {
        total += PerlinNoise(position * frequency) * amplitude;

        maxValue += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }

    return total / maxValue;
}

[numthreads(8, 8, 1)]
void GenerateHeightmap(uint3 ThreadID : SV_DispatchThreadID)
{
    if (ThreadID.x >= MapSize || ThreadID.y >= MapSize)
        return;

    // Calculate position
    float2 position = float2(ThreadID.xy) * Frequency;

    // Generate height using FBM
    float height = FBM(position, Octaves, Lacunarity, Gain);

    // Apply scale and offset
    height = height * HeightScale + HeightOffset;

    // Write to output
    OutputHeightMap[ThreadID.xy] = height;
}
```

### Normal Computation Shader

**File**: `NormalComputeShader.usf`

```hlsl
#include "/Engine/Private/Common.ush"

cbuffer NormalParams : register(b0)
{
    uint MapSize;
    float CellSize;
};

Texture2D<float> InputHeightMap : register(t0);
RWTexture2D<float4> OutputNormalMap : register(u0);

[numthreads(8, 8, 1)]
void ComputeNormals(uint3 ThreadID : SV_DispatchThreadID)
{
    if (ThreadID.x >= MapSize || ThreadID.y >= MapSize)
        return;

    uint2 coord = ThreadID.xy;

    // Sample heights using finite differences
    float heightL = InputHeightMap[coord + uint2(-1, 0)];
    float heightR = InputHeightMap[coord + uint2(1, 0)];
    float heightD = InputHeightMap[coord + uint2(0, -1)];
    float heightU = InputHeightMap[coord + uint2(0, 1)];

    // Calculate gradients
    float3 va = float3(2.0 * CellSize, 0.0, heightR - heightL);
    float3 vb = float3(0.0, 2.0 * CellSize, heightU - heightD);

    // Cross product for normal
    float3 normal = normalize(cross(va, vb));

    // Encode normal (remap from [-1,1] to [0,1])
    float4 encodedNormal = float4(normal * 0.5 + 0.5, 1.0);

    OutputNormalMap[coord] = encodedNormal;
}
```

---

## 🎨 Material System

Advanced material functions for realistic terrain rendering.

### Triplanar Projection

**File**: `TriplanarProjection.ush`

```hlsl
// Triplanar mapping function
float4 TriplanarMapping(
    Texture2D Tex,
    SamplerState TexSampler,
    float3 WorldPosition,
    float3 WorldNormal,
    float TilingScale)
{
    // Blend weights based on normal
    float3 blendWeights = abs(WorldNormal);
    blendWeights = blendWeights / (blendWeights.x + blendWeights.y + blendWeights.z);

    // UV coordinates for each axis
    float2 uvX = WorldPosition.yz * TilingScale;
    float2 uvY = WorldPosition.xz * TilingScale;
    float2 uvZ = WorldPosition.xy * TilingScale;

    // Sample textures
    float4 colorX = Tex.Sample(TexSampler, uvX);
    float4 colorY = Tex.Sample(TexSampler, uvY);
    float4 colorZ = Tex.Sample(TexSampler, uvZ);

    // Blend
    return colorX * blendWeights.x +
           colorY * blendWeights.y +
           colorZ * blendWeights.z;
}
```

### Height-Based Blending

**File**: `HeightBlending.ush`

```hlsl
// Height-based terrain layer blending
float4 HeightBlend(
    float4 Layer1Color,
    float Layer1Height,
    float4 Layer2Color,
    float Layer2Height,
    float BlendStrength)
{
    // Height-based blend factor
    float heightDiff = Layer2Height - Layer1Height;
    float blend = saturate((heightDiff + 0.5) * BlendStrength);

    // Smooth interpolation
    blend = smoothstep(0.0, 1.0, blend);

    return lerp(Layer1Color, Layer2Color, blend);
}

// Multi-layer height blending
float4 MultiLayerHeightBlend(
    float4 Colors[4],
    float Heights[4],
    float Weights[4],
    float BlendStrength)
{
    // Find max height
    float maxHeight = max(max(Heights[0], Heights[1]), max(Heights[2], Heights[3]));

    // Calculate blend factors
    float blends[4];
    float totalBlend = 0.0;

    for (int i = 0; i < 4; ++i)
    {
        float heightFactor = saturate((Heights[i] - maxHeight + 1.0) * BlendStrength);
        blends[i] = Weights[i] * heightFactor;
        totalBlend += blends[i];
    }

    // Normalize and blend
    float4 result = float4(0, 0, 0, 0);
    for (int i = 0; i < 4; ++i)
    {
        result += Colors[i] * (blends[i] / totalBlend);
    }

    return result;
}
```

### Slope-Based Material

**File**: `SlopeMapping.ush`

```hlsl
// Calculate slope angle from world normal
float GetSlopeAngle(float3 WorldNormal)
{
    float dotProduct = dot(WorldNormal, float3(0, 0, 1));
    return acos(saturate(dotProduct)) * (180.0 / 3.14159265);
}

// Slope-based material selection
float4 SlopeBasedMaterial(
    float3 WorldNormal,
    Texture2D FlatTexture,
    Texture2D SteepTexture,
    SamplerState TexSampler,
    float2 UV,
    float SlopeThreshold,
    float BlendRange)
{
    float slope = GetSlopeAngle(WorldNormal);

    // Blend factor
    float blend = saturate((slope - SlopeThreshold) / BlendRange);
    blend = smoothstep(0.0, 1.0, blend);

    // Sample textures
    float4 flatColor = FlatTexture.Sample(TexSampler, UV);
    float4 steepColor = SteepTexture.Sample(TexSampler, UV);

    return lerp(flatColor, steepColor, blend);
}
```

---

## 🛠️ Editor Tools

Professional editor integration.

### Custom Editor Mode

**Class**: `FProceduralMapEditorMode`

```cpp
// Editor mode implementation
void FProceduralMapEditorMode::Enter()
{
    FEdMode::Enter();

    // Initialize tools
    CurrentTool = ETerrainTool::Paint;
    BrushSize = 500.0f;
    BrushStrength = 0.5f;

    // Register input handlers
    GEditor->GetEditorWorldContext().AddRef();
}

bool FProceduralMapEditorMode::HandleClick(
    FEditorViewportClient* ViewportClient,
    HHitProxy* HitProxy,
    const FViewportClick& Click)
{
    if (Click.GetKey() == EKeys::LeftMouseButton)
    {
        FVector HitLocation;
        if (GetHitLocation(ViewportClient, Click, HitLocation))
        {
            ApplyTool(HitLocation);
            return true;
        }
    }

    return false;
}

void FProceduralMapEditorMode::ApplyTool(const FVector& Location)
{
    switch (CurrentTool)
    {
        case ETerrainTool::Paint:
            PaintHeight(Location);
            break;
        case ETerrainTool::Sculpt:
            SculptTerrain(Location);
            break;
        case ETerrainTool::Smooth:
            SmoothTerrain(Location);
            break;
        case ETerrainTool::Flatten:
            FlattenTerrain(Location);
            break;
        case ETerrainTool::Generate:
            GenerateNoise(Location);
            break;
    }
}

void FProceduralMapEditorMode::Render(const FSceneView* View, FViewport* Viewport,
    FPrimitiveDrawInterface* PDI)
{
    FEdMode::Render(View, Viewport, PDI);

    if (bShowBrush && LastBrushLocation.IsSet())
    {
        // Draw brush circle
        FColor BrushColor = GetBrushColor();
        DrawCircle(PDI, LastBrushLocation.GetValue(), FVector::UpVector,
            FVector::RightVector, BrushColor, BrushSize, 32, SDPG_Foreground);
    }
}
```

---

## 📖 API Reference

### Core Classes Quick Reference

#### UQuadTreeTerrain
- `void InitializeTerrain()` - Initialize the terrain system
- `void UpdateLOD(const FVector& CameraPosition)` - Update LOD based on camera
- `float GetHeightAtPosition(const FVector& WorldPosition)` - Query height
- `FVector GetNormalAtPosition(const FVector& WorldPosition)` - Query normal
- `void RegenerateTerrain()` - Force complete regeneration

#### ABiomeGenerator
- `float CalculateTemperature(FVector Position, float Elevation)` - Get temperature
- `float CalculateHumidity(FVector Position, float DistanceToWater)` - Get humidity
- `EBiomeType GetBiomeFromClimate(float Temp, float Humidity, float Elevation)` - Determine biome
- `TArray<FBiomeData> CalculateBiomeWeights(FVector Position, ...)` - Multi-biome blending

#### UHydraulicErosion
- `void Erode(TArray<float>& HeightMap, int32 MapSize, bool bAsync)` - Apply erosion
- `bool IsErosionComplete()` - Check if async erosion finished
- `void SetParameters(...)` - Configure erosion settings

#### UWaveFunctionCollapse
- `bool Generate()` - Run WFC algorithm
- `void InitializeGrid()` - Setup grid
- `bool FindLowestEntropyCell(int32& X, int32& Y)` - Find next cell
- `bool CollapseCell(int32 X, int32 Y)` - Collapse cell
- `bool Propagate(int32 StartX, int32 StartY)` - Propagate constraints

#### ARiverGenerator
- `void GenerateRiver()` - Generate complete river system
- `TArray<FVector> FindLowestPath(const FVector& Start, const FVector& End)` - Pathfinding
- `void ModifyLandscape()` - Carve river into landscape
- `void CreateWaterBody()` - Spawn UE5 water body

#### ACaveGenerator
- `void GenerateCave()` - Generate complete cave system
- `void GenerateMarchingCubes3D()` - 3D mesh generation
- `void GenerateMarchingSquares2D()` - 2D mesh generation
- `void SpawnResources()` - Place resources in cave

---

## 🎯 Best Practices

### Performance
1. Use GPU compute shaders for large terrains (>512x512)
2. Enable async generation for runtime
3. Tune LOD distances based on target platform
4. Use mesh pooling to reduce allocations
5. Implement frustum culling for chunks

### Quality
1. Combine erosion types (hydraulic + thermal) for realism
2. Use multi-biome blending at transition zones
3. Apply curl noise to rivers for natural meandering
4. Use height-based material blending
5. Generate smooth normals for better lighting

### Workflow
1. Start with terrain presets
2. Tweak parameters in real-time preview
3. Save successful configurations as presets
4. Use editor mode for fine-tuning
5. Profile regularly with statistics window

---

<div align="center">

**For more information, see [README.md](./README.md) | [Quick Start](./QUICK_START.md) | [API Reference](./API_REFERENCE.md)**

</div>
