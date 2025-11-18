# 🔧 Guide de Résolution - Erreur de Compilation

## Erreur : "ProceduralMap could not be compiled. Try rebuilding from source manually."

Cette erreur peut avoir plusieurs causes. Voici les solutions dans l'ordre :

---

## ✅ SOLUTION 1 : Vérifier les Prérequis (COMMENCEZ ICI)

### 1. Version d'Unreal Engine
- **Requis** : Unreal Engine 5.4 ou supérieur
- **Vérifier** : Ouvrir Epic Games Launcher > Library
- Si vous avez 5.0-5.3, il faut mettre à jour

### 2. Visual Studio (Windows)
- **Requis** : Visual Studio 2022 avec "Desktop development with C++"
- **Ou** : Visual Studio 2019 (v16.11.5+)
- **Vérifier** : Ouvrir Visual Studio Installer

### 3. Xcode (Mac)
- **Requis** : Xcode 14+ avec Command Line Tools
- Vérifier : `xcode-select --version`

---

## ✅ SOLUTION 2 : Nettoyer et Régénérer le Projet

```bash
cd /home/user/ProceduralMap

# 1. Supprimer les fichiers temporaires
rm -rf Binaries/
rm -rf Intermediate/
rm -rf Saved/
rm -rf .vs/
rm -rf *.sln

# 2. Supprimer les fichiers de compilation des plugins
rm -rf Plugins/*/Binaries/
rm -rf Plugins/*/Intermediate/

# 3. Régénérer les fichiers projet
# Windows (right-click sur .uproject)
# > Generate Visual Studio project files

# Linux/Mac
/path/to/UnrealEngine/Engine/Build/BatchFiles/GenerateProjectFiles.sh ProceduralMap.uproject
```

---

## ✅ SOLUTION 3 : Vérifier la Structure des Fichiers

La structure DOIT être exactement :

```
ProceduralMap/
├── ProceduralMap.uproject
├── Source/
│   ├── ProceduralMap.Target.cs
│   ├── ProceduralMapEditor.Target.cs
│   ├── ProceduralMap/
│   │   ├── ProceduralMap.Build.cs
│   │   ├── ProceduralMap.h
│   │   ├── ProceduralMap.cpp
│   │   ├── Public/ (tous les .h)
│   │   └── Private/ (tous les .cpp)
│   └── ProceduralMapEditor/
│       ├── ProceduralMapEditor.Build.cs
│       ├── Public/
│       └── Private/
├── Shaders/
│   └── Private/ (fichiers .usf et .ush)
└── Config/
    ├── DefaultEngine.ini
    └── DefaultGame.ini
```

**Vérification rapide** :
```bash
# Ces commandes doivent toutes retourner des fichiers
ls Source/ProceduralMap/ProceduralMap.Build.cs
ls Source/ProceduralMap/Public/*.h
ls Source/ProceduralMap/Private/*.cpp
```

---

## ✅ SOLUTION 4 : Activer les Plugins Requis

Ouvrez `ProceduralMap.uproject` dans un éditeur de texte et vérifiez :

```json
{
  "Plugins": [
    {
      "Name": "ProceduralMeshComponent",
      "Enabled": true
    },
    {
      "Name": "PCG",
      "Enabled": true
    },
    {
      "Name": "Water",
      "Enabled": true
    },
    {
      "Name": "Niagara",
      "Enabled": true
    }
  ]
}
```

---

## ✅ SOLUTION 5 : Compilation Manuelle (Détection d'Erreurs)

### Windows :
```bash
# Ouvrir Developer Command Prompt for VS 2022
cd C:\Path\To\ProceduralMap

# Compiler
"C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" ProceduralMapEditor Win64 Development -Project="C:\Path\To\ProceduralMap\ProceduralMap.uproject" -WaitMutex -FromMsBuild
```

### Linux/Mac :
```bash
cd /home/user/ProceduralMap

# Compiler
/path/to/UnrealEngine/Engine/Build/BatchFiles/Build.sh ProceduralMapEditor Linux Development -Project="/home/user/ProceduralMap/ProceduralMap.uproject"
```

**Les erreurs de compilation apparaîtront ici !** Copiez-les et cherchez la cause.

---

## ✅ SOLUTION 6 : Problèmes Courants Spécifiques

### Erreur : "Cannot open include file: 'ProceduralMap.h'"
**Cause** : Fichier manquant ou mal placé
**Solution** :
```bash
# Vérifier que le fichier existe
ls Source/ProceduralMap/ProceduralMap.h
```

### Erreur : "Module 'ProceduralMeshComponent' not found"
**Cause** : Plugin non activé
**Solution** : Éditer le .uproject et ajouter dans "Plugins" :
```json
{"Name": "ProceduralMeshComponent", "Enabled": true}
```

### Erreur : "ShaderCore.h not found"
**Cause** : Module manquant dans Build.cs
**Solution** : Déjà inclus dans notre Build.cs ("RenderCore", "Renderer")

### Erreur : Multiples erreurs C2039, C2065 dans FastNoiseLite
**Cause** : FastNoiseLite.h mal placé
**Solution** :
```bash
# Vérifier
ls Source/ProceduralMap/ThirdParty/FastNoiseLite/FastNoiseLite.h

# Si absent, télécharger
curl -L -o Source/ProceduralMap/ThirdParty/FastNoiseLite/FastNoiseLite.h \
  "https://raw.githubusercontent.com/Auburn/FastNoiseLite/master/Cpp/FastNoiseLite.h"
```

### Erreur : "IMPLEMENT_PRIMARY_GAME_MODULE redefined"
**Cause** : Conflit avec d'autres modules
**Solution** : Dans ProceduralMap.cpp, ligne 26 devrait être :
```cpp
IMPLEMENT_PRIMARY_GAME_MODULE(FProceduralMapModule, ProceduralMap, "ProceduralMap");
```

---

## ✅ SOLUTION 7 : Compilation Minimale (Test)

Si tout échoue, créons une version minimale pour tester :

### 1. Créer un fichier de test minimal

Créez `Source/ProceduralMap/Private/TestCompilation.cpp` :
```cpp
// Test de compilation minimal
#include "CoreMinimal.h"

class FTestCompilation
{
public:
    FTestCompilation() { }
    ~FTestCompilation() { }
};
```

### 2. Compiler juste ce fichier
Si même ce fichier minimal ne compile pas, le problème est avec votre installation Unreal/Visual Studio.

---

## ✅ SOLUTION 8 : Vérifier les Logs Détaillés

Les logs de compilation sont dans :
```
ProceduralMap/Saved/Logs/
```

Ouvrez le fichier le plus récent et cherchez :
- `error C...` (erreurs C++)
- `LNK...` (erreurs de linkage)
- `Fatal error` (erreurs fatales)

**Copiez ces erreurs et envoyez-les moi !**

---

## 🆘 SOLUTION DE DERNIER RECOURS : Projet Minimal

Si rien ne fonctionne, créons un projet minimal qui compile :

```bash
cd /home/user

# Créer nouveau projet minimal
mkdir ProceduralMapMinimal
cd ProceduralMapMinimal

# Copier seulement les fichiers essentiels
cp /home/user/ProceduralMap/ProceduralMap.uproject .
cp -r /home/user/ProceduralMap/Source .
cp -r /home/user/ProceduralMap/Config .

# Nettoyer
rm -rf Binaries Intermediate Saved

# Ouvrir avec Unreal
```

---

## 📝 CHECKLIST DE DIAGNOSTIC

Cochez ce qui est OK :

- [ ] Unreal Engine 5.4+ installé
- [ ] Visual Studio 2022 avec C++ installé (Windows)
- [ ] ProceduralMap.uproject existe
- [ ] Source/ProceduralMap/ProceduralMap.Build.cs existe
- [ ] Source/ProceduralMap/ProceduralMap.h existe
- [ ] Source/ProceduralMap/ProceduralMap.cpp existe
- [ ] Source/ProceduralMap/Public/ contient des .h
- [ ] Source/ProceduralMap/Private/ contient des .cpp
- [ ] FastNoiseLite.h est dans ThirdParty/
- [ ] Plugins requis activés dans .uproject

---

## 🔍 COMMANDES DE DIAGNOSTIC

Exécutez ces commandes et envoyez-moi les résultats :

```bash
# 1. Structure
find Source -type f -name "*.Build.cs"

# 2. Fichiers principaux
ls -la Source/ProceduralMap/

# 3. Nombre de fichiers
find Source/ProceduralMap/Public -name "*.h" | wc -l
find Source/ProceduralMap/Private -name "*.cpp" | wc -l

# 4. FastNoiseLite
ls -la Source/ProceduralMap/ThirdParty/FastNoiseLite/

# 5. Version Unreal (si accessible)
cat ProceduralMap.uproject | grep EngineAssociation
```

---

## 💡 ASTUCE : Compilation Ligne par Ligne

Si vous avez des erreurs C++, cherchez :
1. Le **premier** message d'erreur (pas le dernier)
2. Le nom du fichier problématique
3. Le numéro de ligne

Exemple :
```
TerrainChunk.cpp(45): error C2065: 'FastNoiseLite' undeclared identifier
```
→ Problème : FastNoiseLite.h pas inclus dans TerrainChunk.cpp

**Envoyez-moi la première erreur et je vous dirai exactement quoi faire !**

