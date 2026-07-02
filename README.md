# SunsetCraft V2

> Prototype voxel C++/OpenGL inspiré de Minecraft, développé pour expérimenter la génération de monde, le rendu et l'intégration progressive avec SunsetEngine.

![Status](https://img.shields.io/badge/status-prototype-orange)
![Language](https://img.shields.io/badge/C%2B%2B-20-blue)
![Build](https://img.shields.io/badge/build-CMake-informational)
![Graphics](https://img.shields.io/badge/graphics-OpenGL-success)

---

![Game screenshot](Resources/ScreenShot/Game_10.06.26.png)
![Game screenshot](Resources/ScreenShot/Game_18.06.26.png)
![Game screenshot](Resources/ScreenShot/Game_19.06.26.png)
![Game screenshot](Resources/ScreenShot/Game_20.06.26.png)
![Game screenshot](Resources/ScreenShot/Game_21.06.26.png)

---

## Présentation

SunsetCraft V2 est une réécriture de [SunsetCraft](https://github.com/SunvyWasTaken/SunsetCraft). Le but est de repartir d'une base plus propre sans modifier la première version, puis de tester :

- une génération de monde plus expressive, inspirée de la conférence de Henrik Kniberg [Reinventing Minecraft world generation](https://youtu.be/ob3VwY4JyzE) ;
- un éditeur ImGui pour régler les paramètres de génération directement en jeu ;
- un rendu voxel simple avec ciel, eau animée, inventaire et interaction bloc par bloc ;
- une base réseau locale avec ENet, encore expérimentale ;
- l'intégration progressive avec [`SunsetEngine`](https://github.com/SunvyWasTaken/SunsetEngine), utilisé comme sous-module Git.

---

## Fonctionnalités actuelles

- Menu principal avec les actions `Play` et `Quit`.
- Lancement d'une partie locale depuis le menu, avec hébergement ENet sur le port `7777`.
- Exécutable serveur headless `SunsetCraftV2_Server` disponible via CMake.
- Génération de chunks autour de la position du joueur, avec chargement/déchargement selon la distance de rendu.
- Génération asynchrone des chunks sur plusieurs threads.
- Génération de terrain à partir de plusieurs couches `FastNoiseSIMD`.
- Passes de génération pour le relief, la surface, l'eau, les minerais et les caves.
- Blocs et items chargés depuis les fichiers JSON du dossier `Save`.
- Rendu OpenGL des chunks, du ciel, de l'eau transparente/animée et de l'interface.
- Inventaire et barre rapide, avec sélection à la molette.
- Placement et destruction de blocs avec le raycast caméra.
- Éditeur ImGui `Parameter` pour :
  - changer ou randomiser la seed ;
  - modifier la distance de rendu ;
  - ajouter et sélectionner des couches de bruit ;
  - choisir le type de bruit ;
  - régler les octaves, la fréquence et l'amplitude ;
  - éditer une courbe de remapping des valeurs de bruit ;
  - sauvegarder et charger des presets de bruit JSON.
- Ancienne couche de test réseau avec chat ImGui conservée dans le code, mais non branchée sur le flux de jeu principal.

---

## État du projet

Le projet est un prototype en développement actif. Une partie peut être lancée depuis le menu et le monde se génère autour du joueur, mais plusieurs systèmes restent incomplets.

À savoir :

- le bouton `Play` démarre une session locale et charge directement le jeu ;
- il n'y a plus de boutons `Start Server` / `Join Server` dans le menu principal ;
- la réplication complète du monde en multijoueur n'est pas encore implémentée ;
- l'exécutable headless héberge une session locale, mais le gameplay reste orienté prototype ;
- les arbres sont présents dans le code de génération, mais leur placement est actuellement désactivé ;
- le fichier de bruit chargé par défaut est `NoiseData.json` dans le chemin de sauvegarde configuré par SunsetEngine.

---

## Technologies

- C++20
- CMake 3.28+
- OpenGL
- GLFW / GLAD
- GLM
- ImGui avec docking expérimental
- ENet
- EnTT
- nlohmann-json
- spdlog
- assimp
- stb, via SunsetEngine
- vcpkg en mode manifest
- [`SunsetEngine`](https://github.com/SunvyWasTaken/SunsetEngine) comme sous-module Git

---

## Prérequis

Avant de compiler, installez :

- un compilateur compatible C++20 ;
- CMake `3.28` ou plus récent ;
- Git ;
- vcpkg ;
- les dépendances graphiques nécessaires à OpenGL sur votre système ;
- Visual Studio, CLion ou un autre IDE CMake si vous préférez travailler avec une interface graphique.

> Sur Windows, Visual Studio 2022 avec la charge de travail C++ est recommandé. CLion fonctionne aussi correctement avec CMake et vcpkg.

---

## Installation

### 1. Cloner le projet avec ses sous-modules

```bash
git clone --recurse-submodules <repo-url> SunsetCraft_V2
cd SunsetCraft_V2
```

Si le projet a été cloné sans le sous-module [`SunsetEngine`](https://github.com/SunvyWasTaken/SunsetEngine), lancez :

```bash
git submodule update --init --recursive
```

### 2. Installer ou préparer vcpkg

Suivez la documentation officielle Microsoft :
[Get started with vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-vs?pivots=shell-cmd).

Exemple d'installation locale à côté du projet :

```bash
git clone https://github.com/microsoft/vcpkg.git
```

Windows :

```bat
vcpkg\bootstrap-vcpkg.bat
```

Linux / macOS :

```bash
./vcpkg/bootstrap-vcpkg.sh
```

### 3. Configurer CMake

Remplacez le chemin du toolchain vcpkg par celui qui correspond à votre installation.

Windows :

```bat
cmake -S . -B Build -DCMAKE_TOOLCHAIN_FILE=%CD%\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Linux / macOS :

```bash
cmake -S . -B Build -DCMAKE_TOOLCHAIN_FILE=$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### 4. Compiler

```bash
cmake --build Build --config Debug
```

Pour une version Release :

```bash
cmake --build Build --config Release
```

---

## Lancer le jeu

Après compilation, lancez l'exécutable `SunsetCraftV2` généré dans le dossier `Build`.

Depuis le menu principal :

1. cliquez sur `Play` pour démarrer une session locale ;
2. utilisez la fenêtre `Parameter` pour ajuster la seed, la distance de rendu et les paramètres de génération ;
3. utilisez l'inventaire et la barre rapide pour sélectionner un bloc ;
4. cassez un bloc avec le clic gauche et placez le bloc sélectionné avec le clic droit.

Pour lancer le serveur headless généré par CMake, utilisez l'exécutable `SunsetCraftV2_Server`.

---

## Données modifiables

Les fichiers du dossier `Save` servent de configuration de prototype :

- `NoiseData.json` : preset de génération chargé par défaut ;
- `BlockReg.json` : registre des blocs et textures associées ;
- `Item.json` : registre des items ;
- `Inputs.json` / `Input.json` : exemples de mapping d'entrées selon la configuration moteur ;
- `imgui.ini` : état des fenêtres ImGui.

---

## Licence

Ce dépôt inclut un fichier `LICENSE`. Consultez-le avant de distribuer ou réutiliser le jeu.
