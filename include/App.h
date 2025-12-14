#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "ScreenManager.h"
#include "screens/GameScreen.h"

class App
{
public:
    App();
    void run();

private:
    void handleGlobalEvent(const sf::Event& e);

    // music
    void startMusic();
    void stopMusic();
    void toggleMusic();

    void setMusicVolume(float v);
    float getMusicVolume() const;

private:
    sf::RenderWindow window;
    ScreenManager    screens;

    GameScreen*      gameScreen;

    sf::Music        bgMusic;
    bool             musicEnabled;
    float            musicVolume; // 0..100
};
