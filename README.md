# Jeu du Serpent en C - Version Complète

Un jeu de serpent ultra-développé en C utilisant la bibliothèque ncurses avec de nombreuses fonctionnalités avancées.

## 🎮 Fonctionnalités Principales

### Types de Nourriture
- **Nourriture normale** (★) : +10 points
- **Nourriture dorée** ($) : +50 points (rare)
- **Nourriture poison** (X) : Rétrécit le serpent de 2 segments, -5 points
- **Nourriture rapide** (!) : Augmente temporairement la vitesse
- **Nourriture bonus** (?) : +100 points (très rare)

### Power-ups Spéciaux
- **Ralentissement** (S) : Ralentit le jeu temporairement
- **Invincibilité** (I) : Permet de passer à travers les obstacles et le corps
- **Multiplicateur** (M) : Double les points obtenus
- **Magnétique** (G) : Attire la nourriture vers le serpent

### Modes de Jeu
1. **Classique** : Mode traditionnel avec collisions mortelles
2. **Arcade** : 3 vies, continue après collision
3. **Défi** : Obstacles fixes et téléporteurs sur le terrain
4. **Libre** : Passage à travers les murs (wrap-around)

### Niveaux de Difficulté
- **Facile** : Grande grille (80x30), vitesse lente (200ms)
- **Moyen** : Grille normale (60x20), vitesse moyenne (150ms)
- **Difficile** : Petite grille (50x18), vitesse rapide (100ms)
- **Extrême** : Très petite grille (40x15), vitesse maximale (50ms)

### Multijoueur
- **Mode 2 joueurs** : Deux serpents s'affrontent sur le même terrain
  - Joueur 1 : Contrôles WASD
  - Joueur 2 : Contrôles Flèches directionnelles

### Obstacles
- **Obstacles fixes** : Murs qui bloquent le chemin
- **Téléporteurs** : Portes qui téléportent le serpent

### Statistiques et Classements
- **Top 10** des meilleurs scores sauvegardés
- Statistiques détaillées : niveau atteint, nourriture mangée, temps de jeu
- Affichage des scores après chaque partie

### Personnalisation
- **4 thèmes de couleurs** : Classique, Neon, Rétro, Dark
- Caractères personnalisés pour chaque serpent
- Taille de grille adaptée selon la difficulté

### Effets Visuels
- Animation de pulsation pour la nourriture
- Effets visuels pour les power-ups actifs
- Indicateur de combo visible
- Affichage des vies en mode arcade

### Système de Combo
- Bonus de points si vous mangez rapidement plusieurs nourritures
- Multiplicateur progressif selon le combo

## 🚀 Prérequis

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

## 📦 Compilation

```bash
make
```

Ou manuellement:
```bash
gcc -Wall -Wextra -std=c11 -O2 -o snake snake.c -lncurses
```

## 🎯 Utilisation

### Lancer le jeu
```bash
./snake
```

### Navigation dans les menus
- **Flèches haut/bas** : Naviguer dans les menus
- **Entrée** : Sélectionner
- **ESC/Q** : Quitter/Retour

### Contrôles en jeu

**Joueur 1 (Solo ou Multijoueur) :**
- **Flèches directionnelles** ou **WASD** : Diriger le serpent
- **P** : Mettre en pause/reprendre
- **Q** : Quitter

**Joueur 2 (Multijoueur uniquement) :**
- **Flèches directionnelles** : Diriger le serpent
- **P** : Pause (partagée)
- **Q** : Quitter

### Règles du jeu

1. **Dirigez le serpent** avec les flèches ou WASD
2. **Mangez la nourriture** pour grandir et gagner des points
3. **Évitez** les murs, votre propre corps, les obstacles et l'autre serpent (multijoueur)
4. **Utilisez les power-ups** pour vous aider
5. **Le jeu accélère** à chaque niveau (tous les 100 points)
6. **En mode arcade**, vous avez 3 vies
7. **En mode libre**, vous traversez les murs

### Score et Progression

- Points variables selon le type de nourriture
- Combo system : bonus si vous mangez rapidement
- Multiplicateur de score avec power-up
- Niveau augmente tous les 100 points
- Vitesse augmente avec chaque niveau
- Les meilleurs scores sont sauvegardés dans `.snake_top_scores`

## 🗂️ Structure du Code

- `snake.c` - Code source principal du jeu (1448 lignes)
- `Makefile` - Fichier de compilation
- `.snake_top_scores` - Fichier de sauvegarde des meilleurs scores (créé automatiquement)
- `snake_backup.c` - Sauvegarde de l'ancienne version (481 lignes)

## 🎨 Thèmes Disponibles

1. **Classique** : Couleurs classiques (vert, jaune, rouge)
2. **Neon** : Couleurs vives et fluorescentes
3. **Rétro** : Style rétro arcade
4. **Dark** : Thème sombre

## 📊 Statistiques

Le jeu enregistre :
- Score final
- Niveau atteint
- Nombre de nourritures mangées
- Temps de jeu
- Top 10 des meilleurs scores

## 🔧 Nettoyage

Pour supprimer les fichiers compilés et les fichiers de scores:
```bash
make clean
```

Cela supprime :
- `snake` (exécutable)
- `.snake_best_score` (ancien format)
- `.snake_top_scores` (nouveau format)

## 💡 Fonctionnalités Techniques

Le jeu est entièrement écrit en C standard (C11) et utilise :
- **ncurses** pour l'interface graphique et les couleurs
- **Structures de données** complexes pour gérer le jeu
- **Gestion des entrées clavier** en temps réel
- **Système de pause** et menus interactifs
- **Sauvegarde/chargement** des scores
- **Gestion des collisions** avancée
- **Système de timer** pour les effets temporaires
- **Génération procédurale** d'obstacles et de nourriture

## 🎮 Exemples de Gameplay

### Mode Classique
Jouez traditionnellement avec collisions mortelles.

### Mode Arcade
Avec 3 vies, vous pouvez continuer après une collision. Parfait pour les débutants !

### Mode Défi
Des obstacles parsèment le terrain. Les téléporteurs peuvent être utiles ou dangereux !

### Mode Libre
Passez à travers les murs pour une expérience différente. Le serpent apparaît de l'autre côté.

### Mode Multijoueur
Affrontez un ami ! Le premier à mourir perd.

## 🧪 Tests Unitaires

Le projet inclut une suite complète de tests unitaires pour vérifier la logique du jeu.

### Compiler les tests
```bash
make test
```

### Exécuter les tests
```bash
./test_snake
```

### Tests couverts

Les tests unitaires vérifient :

- ✅ **Génération de positions** : Vérification que les positions aléatoires sont dans les limites
- ✅ **Validation de positions** : Test des collisions avec obstacles, nourriture et serpents
- ✅ **Initialisation du serpent** : Vérification des valeurs initiales et positions
- ✅ **Initialisation du jeu** : Test de tous les modes et difficultés
- ✅ **Système de power-ups** : Vérification des timers et effets
- ✅ **Top scores** : Test de sauvegarde/chargement et tri des scores
- ✅ **Caractères de nourriture** : Vérification des symboles
- ✅ **Caractères de power-ups** : Vérification des symboles
- ✅ **Logique de mouvement** : Test des positions initiales
- ✅ **Modes de jeu** : Vérification de tous les modes (classique, arcade, défi, libre)

**Statistiques des tests :**
- Tests exécutés : 266
- Taux de réussite : 100%
- Couverture : Fonctions logiques principales

## 🐛 Bugs Connus / Améliorations Futures

- Le mode multijoueur utilise le même terminal (contraintes de ncurses)
- Les thèmes sont pré-configurés (pas encore de personnalisation en jeu)
- Le nom du joueur est fixé à "Player" dans les scores (pourrait être personnalisable)

## 📝 Auteur

Jeu développé en C avec toutes les fonctionnalités modernes d'un jeu de serpent complet et avancé.

**Version :** 2.0 (Version complète avec toutes les fonctionnalités)
**Lignes de code :** 1448
**Tests unitaires :** 266 tests (100% de réussite)
**Compilation :** Sans erreurs ni warnings avec `-Wall -Wextra`
