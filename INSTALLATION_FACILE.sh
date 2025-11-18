#!/bin/bash

echo "🎮 Installation Facile - ProceduralMap Plugin"
echo "=============================================="
echo ""
echo "Quel est le chemin vers VOTRE projet Unreal ?"
echo "(Exemple: /chemin/vers/MonProjet)"
read -p "Chemin : " PROJECT_PATH

if [ ! -d "$PROJECT_PATH" ]; then
    echo "❌ Erreur: Le dossier n'existe pas: $PROJECT_PATH"
    exit 1
fi

if [ ! -f "$PROJECT_PATH"/*.uproject ]; then
    echo "❌ Erreur: Aucun fichier .uproject trouvé dans $PROJECT_PATH"
    exit 1
fi

PLUGIN_DIR="$PROJECT_PATH/Plugins/ProceduralMap"

echo ""
echo "📦 Installation dans: $PLUGIN_DIR"
echo ""

# Créer structure
mkdir -p "$PLUGIN_DIR/Source/ProceduralMap"
mkdir -p "$PLUGIN_DIR/Source/ProceduralMapEditor"
mkdir -p "$PLUGIN_DIR/Shaders/Private"
mkdir -p "$PLUGIN_DIR/Resources"
mkdir -p "$PLUGIN_DIR/Content"

# Copier fichiers
echo "📋 Copie des fichiers..."
cp -r /home/user/ProceduralMap/Source/ProceduralMap/* "$PLUGIN_DIR/Source/ProceduralMap/"
cp -r /home/user/ProceduralMap/Source/ProceduralMapEditor/* "$PLUGIN_DIR/Source/ProceduralMapEditor/"
cp -r /home/user/ProceduralMap/Shaders/Private/* "$PLUGIN_DIR/Shaders/Private/"
cp /home/user/ProceduralMapPlugin/ProceduralMap/ProceduralMap.uplugin "$PLUGIN_DIR/"

# Nettoyer fichiers Target.cs (pas besoin dans plugin)
rm -f "$PLUGIN_DIR/Source/ProceduralMap.Target.cs" 2>/dev/null
rm -f "$PLUGIN_DIR/Source/ProceduralMapEditor.Target.cs" 2>/dev/null

echo ""
echo "✅ Installation terminée !"
echo ""
echo "📝 Étapes suivantes:"
echo "1. Ouvrir votre projet dans Unreal Engine"
echo "2. Edit > Plugins"
echo "3. Chercher 'Procedural Map'"
echo "4. Cocher 'Enabled'"
echo "5. Redémarrer l'éditeur"
echo "6. Le plugin compilera automatiquement"
echo ""
echo "🎉 Profitez !"

