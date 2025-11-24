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
|
|──assets/
| |──fonts/
| | |──FZShuTi.ttf
| | |──Inter_28pt-SemiBold.ttf
| |──img/
| | |──menu_bg.jpg
| |──sfx/
| | |──bg_music.ogg
|
|──include/
| |──screens/
| | |──GameScreen.h
| | |──MenuScreen.h
| | |──PreGameScreen.h
| | |──SettingsScreen.h
| |──widgets/
| | |──Button.h
| | |──IconButton.h
| |──App.h
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
| |──App.cpp
| |──main.cpp
| |──ScreenManager.cpp
| 
|GoGame.exe
|
|README.md
| 
|savegame.txt
```

## 3. How to play

### Menu
- Play  
- Continue game (load saved game)
- Settings (music toggle - available, changing stones's style, boards's style)
- Exit 

### Pre-game Selection
- Choose board size (9/13/19)
- Choose game mode (2P - available, Easy AI, Medium AI, Hard AI)
- Start the game

### Ingame
- Click intersections to place stones
- Pass (skip a turn)
- Save game (storing progress)
- Back to menu

> Note: If you want to reset the game even after saving it, just press the ‘Back to Menu’ button to return to the main menu, and then select ‘PLAY’ instead of ‘Continue Game’. This will start a brand-new level for you.

two consecutive passes end the game.
Winner = player with more captured stones

## 4. Build & Run Instruction 

### Requirements
- C++17 
- SFML 3.0.2
- g++ (MSYS2 UCRT64)

### Build (We build in terminal's UCRT64)
g++ -std=c++17 -Iinclude src/main.cpp src/App.cpp src/ScreenManager.cpp src/widgets/Button.cpp src/widgets/IconButton.cpp src/screens/MenuScreen.cpp src/screens/SettingsScreen.cpp src/screens/PreGameScreen.cpp src/screens/GameScreen.cpp -o GoGame.exe -LC:/msys64/ucrt64/lib -IC:/msys64/ucrt64/include -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

### Run
./GoGame.exe


