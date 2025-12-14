# GoGame 

## Developed by
**Nguyen Minh Khang - 25125018 & Dang Xuan Phat - 25125062**
Faculty of Information Technology
University of Science - VNUHCM

---

## 1. Overview

This project is a Go game application written in C++ using SFML 3.0.2.
It supports (Up to November 24):

- Player vs Player
- Board size: 9x9, 13x13, 19x19
- Saving and loading
- Background music toggle 
- Interactive UI with multiple screens

The game includes essential Go rules (based on Weiqi or Chinese Go Rule):
- Liberties and capturing
- Suicide prevention
- Ko rule
- Pass + end-game scoring based on the number of captured stones

---

## 2. Project structure

```
Go/
|──.vscode/
| |──c_cpp_properties.json
| |──launch.json
| |──settings.json
| |──tasks.json
|
|──assets/
| |──fonts/
| | |──FZShuTi.ttf
| | |──Inter_28pt-SemiBold.ttf
| |──img/
| | |──stone/
| | | |──Pirate/
| | | | |──Pirate_black.png
| | | | |──Pirate_white.png
| | | |
| | | |──Weapon/
| | | | |──Weapon_black.png
| | | | |──Weapon_white.png
| | | |
| | |──menu_bg.jpg
| | |──Myth.jpg
| | |──Pirate.jpg
| | | 
| |──sfx/
| | |──bg_music.ogg
|
|──include/
| |──screens/
| | |──GameScreen.h
| | |──MenuScreen.h
| | |──PreGameScreen.h
| | |──SettingsScreen.h
| |
| |──widgets/
| | |──Button.h
| | |──IconButton.h
| |──AI.h
| |──App.h
| |──BoardTheme.h
| |──Config.h
| |──ConfigManager.h
| |──GameLogic.h
| |──Screen.h
| |──ScreenManager.h
| 
|──src/
| |──screens/
| | |──GameScreen.cpp
| | |──MenuScreen.cpp
| | |──PreGameScreen.cpp
| | |──SettingsScreen.cpp
| |widgets/
| | |──Button.cpp
| | |──IconButton.cpp
| |──AI.cpp
| |──App.cpp
| |──ConfigManager.cpp
| |──GameLogic.cpp
| |──main.cpp
| |──ScreenManager.cpp
| 
|GoGame.exe
|
|README.md
|
|save_ai.txt
| 
|savegame.txt
|
|settings.cfg
```

## 3. How to play

### Menu
- Play  
- Continue game (load saved game both AI or PvP and the last saved skin)
- Settings (music toggle, changing stones's style, boards's style)
- Exit 

### Pre-game Selection
- Choose board size (9/13/19)
- Choose game mode (2P , Easy AI, Medium AI, Hard AI)
- Start the game

### Ingame
- Click intersections to place stones
- Pass (skip a turn), if both players passed, change to mark-dead mode to discuss what stones are dead and remove them + change the point
- Finish and Score. After mark-dead mode, finish the game and calculate score.
- Save game (storing progress)
- Back to menu

> Note: If you want to reset the game even after saving it, just press the ‘Back to Menu’ button to return to the main menu, and then select ‘PLAY’ instead of ‘Continue Game’. This will start a brand-new level for you.

two consecutive passes to mark-dead mode and press 'Finish and Score' to end the game.
Winner = who has the highest score

## 4. Build & Run Instruction 

### Requirements
- C++17 
- SFML 3.0.2
- g++ (MSYS2 UCRT64)

### Build (We build in terminal's UCRT64)
g++ -std=c++17 -I include src/main.cpp src/App.cpp src/ScreenManager.cpp src/ConfigManager.cpp src/GameLogic.cpp src/AI.cpp src/widgets/Button.cpp src/widgets/IconButton.cpp src/screens/MenuScreen.cpp src/screens/PreGameScreen.cpp src/screens/SettingsScreen.cpp src/screens/GameScreen.cpp -o GoGame.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

### Run
./GoGame.exe

Demo video:
https://drive.google.com/file/d/1mbQ4Ace68Z3dHjK_28rAxmB2zIoa-zhr/view?usp=sharing
(This is the last video, we have a new one for the newest update)
https://drive.google.com/drive/folders/1nUomDgRLmoa58Wd63UZW14alhYXzWohX?usp=sharing
