# Interface Graphique du Jeu du Serpent

## 📺 Vue d'ensemble

Le jeu utilise **ncurses** (New CURSes) pour fournir une interface graphique complète dans le terminal. L'interface graphique est entièrement intégrée dans le code source et fonctionne sur tous les systèmes Unix/Linux.

## 🎨 Architecture de l'Interface Graphique

### Initialisation

L'interface graphique est initialisée dans la fonction `main()` :

```c
int main() {
    initscr();              // Initialise ncurses
    noecho();               // Désactive l'écho des caractères
    curs_set(0);            // Cache le curseur
    init_colors();          // Initialise le système de couleurs
    init_theme_colors(THEME_CLASSIC);  // Applique le thème
    
    // ... code du jeu ...
    
    endwin();               // Nettoie et restaure le terminal
    return 0;
}
```

### Composants Graphiques

#### 1. **Système de Couleurs**

Le jeu utilise 14 paires de couleurs différentes :
- `COLOR_SNAKE1_HEAD` / `COLOR_SNAKE1_BODY` : Couleurs du serpent joueur 1
- `COLOR_SNAKE2_HEAD` / `COLOR_SNAKE2_BODY` : Couleurs du serpent joueur 2
- `COLOR_FOOD_NORMAL` / `COLOR_FOOD_GOLDEN` / etc. : Couleurs des différents types de nourriture
- `COLOR_POWERUP` : Couleurs des power-ups
- `COLOR_OBSTACLE` / `COLOR_PORTAL` : Couleurs des obstacles
- `COLOR_BORDER` : Couleurs des bordures
- `COLOR_TEXT` : Couleurs du texte

#### 2. **Fenêtres (Windows)**

Le jeu utilise des fenêtres ncurses pour chaque écran :

- **Fenêtre principale du jeu** (`game->win`) : Affiche le terrain de jeu
- **Fenêtre du menu principal** : Menu de sélection
- **Fenêtre de sélection du mode** : Choix du mode de jeu
- **Fenêtre de difficulté** : Choix de la difficulté
- **Fenêtre Game Over** : Écran de fin de partie
- **Fenêtre des meilleurs scores** : Affichage du classement

#### 3. **Fonctions d'Affichage**

##### `draw_game(Game *game)`
Affiche tous les éléments du jeu :
- Bordure du terrain
- Serpent(s) avec leurs couleurs
- Nourriture avec animations (pulsation)
- Power-ups actifs
- Obstacles et téléporteurs
- Informations (score, niveau, vies)
- Messages de pause

##### `show_main_menu()`
Menu principal avec navigation au clavier :
- Options sélectionnables
- Surbrillance de l'option sélectionnée
- Instructions pour l'utilisateur

##### `show_game_mode_menu()`
Sélection du mode de jeu (Classique, Arcade, Défi, Libre)

##### `show_difficulty_menu()`
Sélection de la difficulté (Facile, Moyen, Difficile, Extrême)

##### `show_game_over(Game *game)`
Écran de fin de partie avec :
- Score final
- Statistiques
- Options (Rejouer, Menu, Quitter)

##### `show_top_scores(Game *game)`
Affichage du classement des meilleurs scores

## 🎮 Contrôles de l'Interface

### Navigation dans les Menus
- **Flèches Haut/Bas** : Naviguer dans les options
- **Entrée** : Sélectionner l'option
- **ESC/Q** : Quitter/Retour

### Contrôles en Jeu
- **Joueur 1** : Flèches directionnelles ou WASD
- **Joueur 2** (multijoueur) : Flèches directionnelles uniquement
- **P** : Pause/Reprendre
- **Q** : Quitter la partie

## 🌈 Thèmes de Couleurs

Le jeu supporte 4 thèmes pré-configurés :

1. **THEME_CLASSIC** : Thème classique (vert, jaune, rouge)
2. **THEME_NEON** : Thème néon avec couleurs vives
3. **THEME_RETRO** : Style rétro arcade
4. **THEME_DARK** : Thème sombre

Les couleurs sont initialisées via `init_theme_colors(Theme theme)`.

## 🎨 Effets Visuels

### Animations
- **Pulsation de la nourriture** : Les symboles de nourriture pulsent (A_BOLD activé/désactivé)
- **Invincibilité** : Le serpent clignote (A_BLINK) quand invincible
- **Power-ups actifs** : Affichés en gras (A_BOLD)

### Bordures
- Toutes les fenêtres ont des bordures (fonction `box()`)
- Couleurs personnalisables selon le thème

### Formatage du Texte
- **A_BOLD** : Texte en gras pour les titres et éléments importants
- **A_BLINK** : Clignotement pour l'invincibilité
- Combinaisons d'attributs pour les effets

## 📐 Gestion des Dimensions

### Fenêtre de Jeu
La fenêtre de jeu est créée avec des dimensions adaptées :
- Largeur : `grid_width + 2` (terrain + bordures)
- Hauteur : `grid_height + 2` (terrain + bordures)
- Position : Centrée à l'écran
  ```c
  game->win = newwin(game->grid_height + 2, game->grid_width + 2,
                     (LINES - game->grid_height) / 2 - 1,
                     (COLS - game->grid_width) / 2 - 1);
  ```

### Fenêtres de Menu
Toutes les fenêtres de menu sont centrées à l'écran :
```c
WINDOW *menu_win = newwin(height, width, 
                          (LINES - height) / 2, 
                          (COLS - width) / 2);
```

## 🔄 Boucle de Rendu

La boucle principale du jeu (`game_loop`) :
1. Gère les entrées utilisateur
2. Met à jour la logique du jeu
3. Appelle `draw_game()` pour rafraîchir l'affichage
4. Utilise `usleep(10000)` pour limiter le framerate (~100 FPS max)

## 🛠️ Fonctions Utilitaires Graphiques

### `init_colors()`
- Vérifie si le terminal supporte les couleurs
- Initialise le système de couleurs ncurses
- Utilise `use_default_colors()` pour la transparence

### `init_theme_colors(Theme theme)`
- Définit les paires de couleurs selon le thème
- Utilise `init_pair()` pour créer les combinaisons couleur/arrière-plan

### Gestion des Caractères
- `get_food_char(FoodType type)` : Retourne le caractère pour chaque type de nourriture
- `get_powerup_char(PowerUpType type)` : Retourne le caractère pour chaque power-up

## 📱 Compatibilité

### Terminaux Supportés
- Terminal Linux standard (xterm, gnome-terminal, konsole, etc.)
- Terminal macOS (Terminal.app, iTerm2)
- SSH sessions avec support couleur
- Terminaux dans les IDE (VS Code, etc.)

### Prérequis
- Terminal avec support ANSI/VT100
- Support des couleurs (la plupart des terminaux modernes)
- Taille minimale recommandée : 80x24 caractères

### Détection Automatique
Le jeu détecte automatiquement :
- Support des couleurs (`has_colors()`)
- Dimensions du terminal (`LINES`, `COLS`)
- Si les couleurs ne sont pas disponibles, le jeu fonctionne en mode texte simple

## 🐛 Gestion d'Erreurs

- Si le terminal est trop petit, les fenêtres peuvent être coupées
- Si les couleurs ne sont pas supportées, le jeu fonctionne en mode texte
- Les fenêtres sont automatiquement nettoyées avec `delwin()` et `endwin()`

## 🎯 Points d'Intégration

L'interface graphique est intégrée à plusieurs niveaux :

1. **Initialisation** : `main()` → `init_colors()` → `init_theme_colors()`
2. **Création des fenêtres** : `init_game()` crée la fenêtre de jeu
3. **Rendu** : `game_loop()` → `draw_game()` à chaque frame
4. **Menus** : Fonctions `show_*_menu()` pour la navigation
5. **Nettoyage** : `endwin()` à la fin du programme

## 📝 Notes Techniques

- **Performance** : Utilisation de `nodelay()` pour les entrées non-bloquantes
- **Rafraîchissement** : `wrefresh()` après chaque mise à jour de fenêtre
- **Mémoire** : Chaque fenêtre est allouée dynamiquement et libérée avec `delwin()`
- **Thread Safety** : ncurses n'est pas thread-safe, utilisation mono-thread uniquement

## 🚀 Améliorations Possibles

- Support de la redimensionnement dynamique du terminal (SIGWINCH)
- Animation plus fluide avec interpolation
- Effets de transition entre écrans
- Support de plus de thèmes personnalisés
- Interface en mode texte améliorée pour terminaux sans couleur

