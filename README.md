# Jeu du Serpent en C

Un jeu de serpent développé en C utilisant la bibliothèque ncurses avec de nombreuses fonctionnalités avancées.

## Fonctionnalités

- 🎮 Interface graphique complète avec ncurses
- 🎨 Couleurs et design soigné
- 📊 Système de score et de niveaux
- 🏆 Meilleur score sauvegardé
- 📋 Menu principal interactif
- ⏸️ Pause (touche P)
- ⚡ Vitesse progressive qui augmente avec les niveaux
- 🎯 Collision detection (murs et corps)
- 🌟 Croissance du serpent
- ⌨️ Contrôles multiples (flèches ou WASD)

## Prérequis

- GCC (GNU Compiler Collection)
- Bibliothèque ncurses (`libncurses-dev` sur Debian/Ubuntu, `ncurses-devel` sur Fedora)

### Installation des dépendances

**Debian/Ubuntu:**
```bash
sudo apt-get install build-essential libncurses-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc ncurses-devel
```

**macOS:**
```bash
brew install ncurses
```

## Compilation

```bash
make
```

Ou manuellement:
```bash
gcc -Wall -Wextra -std=c11 -O2 -o snake snake.c -lncurses
```

## Utilisation

### Lancer le jeu
```bash
./snake
```

### Contrôles

**Navigation dans les menus:**
- Flèches haut/bas pour naviguer
- Entrée pour sélectionner
- Q pour quitter

**Dans le jeu:**
- Flèches directionnelles ou WASD pour diriger le serpent
- P pour mettre en pause/reprendre
- Q pour quitter

### Règles du jeu

1. Dirigez le serpent avec les flèches ou WASD
2. Mangez la nourriture (★) pour grandir et gagner des points
3. Évitez les murs et votre propre corps
4. Le jeu accélère à chaque niveau (tous les 50 points)
5. Le meilleur score est automatiquement sauvegardé

### Score

- +10 points par nourriture mangée
- Niveau augmente tous les 50 points
- Vitesse augmente avec chaque niveau
- Le meilleur score est sauvegardé dans `.snake_best_score`

## Nettoyage

Pour supprimer les fichiers compilés et le fichier de meilleur score:
```bash
make clean
```

## Structure du code

- `snake.c` - Code source principal du jeu
- `Makefile` - Fichier de compilation
- `.snake_best_score` - Fichier de sauvegarde du meilleur score (créé automatiquement)

## Développement

Le jeu est entièrement écrit en C standard (C11) et utilise:
- ncurses pour l'interface graphique
- Structures de données pour le serpent et le jeu
- Gestion des entrées clavier en temps réel
- Système de pause et de menus interactifs

## Auteur

Jeu développé en C avec toutes les fonctionnalités modernes d'un jeu de serpent complet.
