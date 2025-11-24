#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

#include "ScreenManager.h"

class GameScreen; // forward declare

class App
{
public:
    App();
    void run();

private:
    sf::RenderWindow window;
    ScreenManager    screens;

    GameScreen*      gameScreen;   // trỏ tới màn Game

    // Nhạc nền
    sf::Music        bgMusic;
    bool             musicEnabled;

    void handleGlobalEvent(const sf::Event& e);

    // helpers cho nhạc
    void startMusic();
    void stopMusic();
    void toggleMusic();
};
