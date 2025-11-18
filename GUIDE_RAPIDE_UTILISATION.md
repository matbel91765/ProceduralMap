# 🚀 Guide Rapide d'Utilisation

## Vous avez 2 options :

---

## ✅ OPTION 1 : Projet Standalone (Le Plus Simple)

### Étapes :

1. **Aller dans le dossier téléchargé**
   ```
   /home/user/ProceduralMap/
   ```

2. **Double-cliquer sur ce fichier :**
   ```
   ProceduralMap.uproject
   ```

3. **Quand Unreal demande :**
   ```
   "The following modules are missing... Would you like to rebuild them now?"
   ```
   → **Cliquer YES**

4. **Attendre la compilation** (5-15 minutes)

5. **C'est prêt !** 🎉

### ⚠️ NE PAS :
- ❌ Faire File > Import dans Unreal
- ❌ Copier les fichiers .h/.cpp dans Content/
- ❌ Essayer d'importer comme DataTable
- ❌ Créer un nouveau projet et copier dedans

### ✅ JUSTE :
- ✅ Double-cliquer sur ProceduralMap.uproject
- ✅ Dire YES à la compilation
- ✅ Attendre

---

## ✅ OPTION 2 : Plugin dans Projet Existant

### Si vous voulez utiliser dans VOTRE projet :

#### Méthode Automatique :
```bash
cd /home/user
./INSTALLATION_FACILE.sh
# Suivre les instructions
```

#### Méthode Manuelle :

1. **Copier vers votre projet :**
   ```bash
   # Remplacez par VOTRE chemin
   VOTRE_PROJET="/chemin/vers/VotreProjet"
   
   # Créer dossier plugin
   mkdir -p "$VOTRE_PROJET/Plugins/ProceduralMap"
   
   # Copier
   cp -r /home/user/ProceduralMap/Source/ProceduralMap \
         "$VOTRE_PROJET/Plugins/ProceduralMap/Source/"
   cp -r /home/user/ProceduralMap/Source/ProceduralMapEditor \
         "$VOTRE_PROJET/Plugins/ProceduralMap/Source/"
   cp -r /home/user/ProceduralMap/Shaders \
         "$VOTRE_PROJET/Plugins/ProceduralMap/"
   cp /home/user/ProceduralMapPlugin/ProceduralMap/ProceduralMap.uplugin \
      "$VOTRE_PROJET/Plugins/ProceduralMap/"
   ```

2. **Ouvrir votre projet**

3. **Edit > Plugins**

4. **Chercher "Procedural Map"**

5. **Cocher "Enabled"**

6. **Redémarrer**

---

## 🐛 Si Vous Voyez "DataTable Options"

Cela signifie que vous essayez d'**importer** au lieu d'**ouvrir**.

### Solution :
1. **Fermer Unreal**
2. **NE PAS** faire File > Import
3. **Double-cliquer** directement sur `ProceduralMap.uproject`
4. **OU** utiliser File > Open Project et sélectionner le .uproject

---

## 📝 Après Ouverture du Projet

Une fois le projet ouvert avec succès :

1. **Lire le Quick Start :**
   ```
   /home/user/ProceduralMap/QUICK_START.md
   ```

2. **Essayer les exemples :**
   - Créer un Blueprint Actor
   - Ajouter component "QuadTree Terrain"
   - Configurer les paramètres
   - Play !

3. **Explorer la documentation :**
   - README.md
   - DOCUMENTATION.md
   - API_REFERENCE.md

---

## ❓ Questions Fréquentes

### Q : "Pourquoi DataTable Options apparaît ?"
**R :** Vous essayez d'importer les fichiers source (.h/.cpp) comme des assets. Ce sont des fichiers de CODE, pas des assets. Il faut ouvrir le .uproject.

### Q : "Combien de temps la compilation ?"
**R :** 5-15 minutes la première fois. Ensuite ~30 secondes pour les petits changements.

### Q : "Le projet est vide après compilation ?"
**R :** Normal ! C'est un système de code C++. Créez vos terrains en utilisant les classes fournies (voir QUICK_START.md).

### Q : "Puis-je l'utiliser dans mon projet existant ?"
**R :** Oui ! Utilisez OPTION 2 ci-dessus (plugin).

---

## 🎯 Résumé Ultra-Rapide

```
1. Double-cliquer ProceduralMap.uproject
2. Dire YES à "rebuild modules"
3. Attendre 5-15 minutes
4. Utiliser !
```

**C'est vraiment aussi simple que ça !** 😊

