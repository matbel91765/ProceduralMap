#!/bin/bash
echo "=== DIAGNOSTIC SIMPLE ===" 
echo ""
echo "1. Modules requis dans Build.cs:"
grep -A 20 "PublicDependencyModuleNames" Source/ProceduralMap/ProceduralMap.Build.cs | grep '"'
echo ""
echo "2. FastNoiseLite:"
ls -lh Source/ProceduralMap/ThirdParty/FastNoiseLite/FastNoiseLite.h
echo ""
echo "3. Fichiers .Target.cs:"
ls -1 Source/*.Target.cs
echo ""
echo "4. Version Unreal dans .uproject:"
grep "EngineAssociation" ProceduralMap.uproject
