# Quick Start Guide - Hydraulic Erosion System

## 1. Simple C++ Example

```cpp
#include "HydraulicErosion.h"
#include "ThermalErosion.h"

void ApplyErosionToTerrain(TArray<float>& HeightMap, int32 MapSize)
{
    // Create hydraulic erosion
    UHydraulicErosion* HydraulicErosion = NewObject<UHydraulicErosion>();

    // Configure (using recommended defaults)
    HydraulicErosion->NumIterations = 50000;
    HydraulicErosion->ErosionRadius = 3;
    HydraulicErosion->Inertia = 0.05f;
    HydraulicErosion->SedimentCapacityFactor = 4.0f;
    HydraulicErosion->ErodeSpeed = 0.3f;
    HydraulicErosion->DepositSpeed = 0.3f;
    HydraulicErosion->EvaporateSpeed = 0.01f;
    HydraulicErosion->Gravity = 4.0f;

    // Apply erosion
    HydraulicErosion->Erode(HeightMap, MapSize);

    // Optional: Apply thermal erosion for weathering
    UThermalErosion* ThermalErosion = NewObject<UThermalErosion>();
    ThermalErosion->NumIterations = 10;
    ThermalErosion->TalusAngle = 35.0f;
    ThermalErosion->ErosionRate = 0.5f;
    ThermalErosion->Erode(HeightMap, MapSize);
}
```

## 2. Simple Blueprint Example

1. Create a Blueprint Actor
2. Add these components in Event Graph:

```
Event BeginPlay
    |
    V
[Create Object: HydraulicErosion]
    |
    V
[Set NumIterations = 50000]
    |
    V
[Erode (HeightMap, MapSize, bAsync = false)]
    |
    V
[Print "Erosion Complete!"]
```

## 3. GPU-Accelerated Example

```cpp
#include "ErosionComputeShader.h"

void ApplyGPUErosion(const TArray<float>& InputHeightMap, int32 MapSize)
{
    // Setup parameters
    FErosionParameters Params;
    Params.Inertia = 0.05f;
    Params.ErodeSpeed = 0.3f;
    Params.DepositSpeed = 0.3f;
    Params.Gravity = 4.0f;
    // ... (configure other params)

    // Run on GPU (1 million droplets)
    TArray<float> OutputHeightMap;
    FErosionGPUInterface::RunErosion(
        InputHeightMap,
        MapSize,
        OutputHeightMap,
        1000000,  // 1M droplets
        &Params
    );

    // OutputHeightMap now contains eroded terrain
}
```

## 4. Async Execution Example

```cpp
void ApplyAsyncErosion(TArray<float>& HeightMap, int32 MapSize)
{
    UHydraulicErosion* Erosion = NewObject<UHydraulicErosion>();
    Erosion->NumIterations = 100000;

    // Start async erosion
    Erosion->Erode(HeightMap, MapSize, true);

    // In Tick() or timer callback:
    if (Erosion->IsErosionComplete())
    {
        UE_LOG(LogTemp, Log, TEXT("Erosion finished!"));
        // Use eroded heightmap
    }
}
```

## 5. Using the Example Actor

### In Editor:
1. Open your level
2. From Place Actors panel, search "ErosionExample"
3. Drag into level
4. In Details panel:
   - Set MapSize (e.g., 256)
   - Configure HydraulicErosion parameters
   - Configure ThermalErosion parameters
   - Enable bAutoApplyOnBeginPlay
5. Press Play

### From Blueprint:
```
[Spawn Actor: ErosionExample]
    |
    V
[Call: GenerateTestHeightmap]
    |
    V
[Call: ApplyErosion (bUseGPU = false, bAsync = true)]
    |
    V
[Delay 1.0 second]
    |
    V
[Is Erosion Complete?]
    |
    V
[Print "Done!"]
```

## 6. Integration with Existing Terrain

```cpp
// Example: Apply erosion to TerrainChunk
void ErodeTerrainChunk(ATerrainChunk* Chunk)
{
    // Get heightmap from chunk (implementation specific)
    TArray<float> HeightMap = GetHeightMapFromChunk(Chunk);
    int32 MapSize = Chunk->VerticesPerSide;

    // Apply erosion
    UHydraulicErosion* Erosion = NewObject<UHydraulicErosion>();
    Erosion->NumIterations = 50000;
    Erosion->Erode(HeightMap, MapSize);

    // Apply back to chunk
    ApplyHeightMapToChunk(Chunk, HeightMap);
}
```

## 7. Parameter Presets

### Gentle Erosion (Smooth Landscape)
```cpp
Erosion->NumIterations = 30000;
Erosion->ErodeSpeed = 0.1f;
Erosion->DepositSpeed = 0.4f;
Erosion->Inertia = 0.1f;
```

### Aggressive Erosion (Deep Valleys)
```cpp
Erosion->NumIterations = 100000;
Erosion->ErodeSpeed = 0.5f;
Erosion->DepositSpeed = 0.2f;
Erosion->Inertia = 0.01f;
```

### Mountain Erosion (Natural Peaks)
```cpp
Erosion->NumIterations = 75000;
Erosion->ErodeSpeed = 0.3f;
Erosion->DepositSpeed = 0.3f;
Erosion->Gravity = 6.0f;
ThermalErosion->TalusAngle = 40.0f;
```

### Desert/Badlands
```cpp
Erosion->NumIterations = 50000;
Erosion->ErodeSpeed = 0.4f;
Erosion->EvaporateSpeed = 0.05f;  // Fast evaporation
ThermalErosion->TalusAngle = 30.0f;  // More sliding
```

## 8. Performance Tips

### For Real-Time Applications:
- Use lower iteration counts (5,000 - 10,000)
- Enable async execution
- Use GPU acceleration for large terrains
- Cache eroded results

### For Offline Generation:
- Use high iteration counts (100,000+)
- Use GPU for terrains > 512x512
- Combine multiple erosion passes
- Save eroded heightmaps to disk

### Memory Optimization:
```cpp
// Process in chunks for large terrains
void ErodeInChunks(TArray<float>& FullHeightMap, int32 FullSize, int32 ChunkSize)
{
    for (int32 Y = 0; Y < FullSize; Y += ChunkSize)
    {
        for (int32 X = 0; X < FullSize; X += ChunkSize)
        {
            TArray<float> ChunkHeightMap = ExtractChunk(FullHeightMap, X, Y, ChunkSize);

            UHydraulicErosion* Erosion = NewObject<UHydraulicErosion>();
            Erosion->NumIterations = 25000;
            Erosion->Erode(ChunkHeightMap, ChunkSize);

            MergeChunk(FullHeightMap, ChunkHeightMap, X, Y);
        }
    }
}
```

## 9. Troubleshooting

### Problem: Erosion too strong
**Solution:**
```cpp
Erosion->ErodeSpeed = 0.1f;  // Reduce from 0.3
Erosion->NumIterations = 25000;  // Reduce iterations
```

### Problem: Erosion not visible
**Solution:**
```cpp
Erosion->ErodeSpeed = 0.5f;  // Increase from 0.3
Erosion->NumIterations = 100000;  // Increase iterations
Erosion->ErosionRadius = 5;  // Increase radius
```

### Problem: Droplets get stuck
**Solution:**
```cpp
Erosion->EvaporateSpeed = 0.05f;  // Increase evaporation
Erosion->MaxDropletLifetime = 20;  // Reduce lifetime
```

### Problem: Unnatural patterns
**Solution:**
```cpp
Erosion->Inertia = 0.1f;  // Increase inertia (straighter paths)
Erosion->RandomSeed = NewRandomSeed;  // Change seed
// Then apply thermal erosion:
ThermalErosion->Erode(HeightMap, MapSize);
```

## 10. Complete Pipeline Example

```cpp
void CompleteErosionPipeline(TArray<float>& HeightMap, int32 MapSize)
{
    // Stage 1: Major hydraulic erosion
    UHydraulicErosion* MainErosion = NewObject<UHydraulicErosion>();
    MainErosion->NumIterations = 100000;
    MainErosion->ErodeSpeed = 0.3f;
    MainErosion->Erode(HeightMap, MapSize);

    // Stage 2: Thermal weathering
    UThermalErosion* Weathering = NewObject<UThermalErosion>();
    Weathering->NumIterations = 10;
    Weathering->TalusAngle = 35.0f;
    Weathering->Erode(HeightMap, MapSize);

    // Stage 3: Light hydraulic polish
    UHydraulicErosion* PolishErosion = NewObject<UHydraulicErosion>();
    PolishErosion->NumIterations = 10000;
    PolishErosion->ErodeSpeed = 0.1f;  // Gentle
    PolishErosion->Erode(HeightMap, MapSize);

    UE_LOG(LogTemp, Log, TEXT("Complete erosion pipeline finished!"));
}
```

## Files Reference

### Headers (Include these):
```cpp
#include "HydraulicErosion.h"          // CPU hydraulic erosion
#include "ThermalErosion.h"            // Thermal weathering
#include "ErosionComputeShader.h"      // GPU erosion
#include "ErosionExample.h"            // Example actor
```

### File Locations:
```
Source/ProceduralMap/
├── Public/
│   ├── HydraulicErosion.h
│   ├── ThermalErosion.h
│   ├── ErosionComputeShader.h
│   └── ErosionExample.h
└── Private/
    ├── HydraulicErosion.cpp
    ├── ThermalErosion.cpp
    ├── ErosionComputeShader.cpp
    └── ErosionExample.cpp

Shaders/Private/
└── ErosionComputeShader.usf
```

## Next Steps

1. Read **EROSION_SYSTEM_README.md** for detailed documentation
2. Check **EROSION_FEATURES_SUMMARY.md** for feature list
3. Experiment with different parameters
4. Integrate with your terrain system
5. Profile performance for your use case

---

**Ready to use!** Start with the simple examples above and adjust parameters to suit your needs.
