#include "App.h"

#include <fstream>
#include <iostream>
#include <algorithm>

#include "screens/MenuScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/PreGameScreen.h"
#include "screens/GameScreen.h"

namespace
{
    constexpr const char* SAVE_PATH            = "savegame.txt";
    constexpr const char* PREGAME_CONFIG_PATH  = "pregame_tmp.txt";
    constexpr const char* BG_MUSIC_PATH        = "assets/sfx/bg_music.ogg";
}

App::App()
    : window()
    , screens()
    , gameScreen(nullptr)
    , bgMusic()
    , musicEnabled(false)
    , musicVolume(40.f)
{
    window.create(sf::VideoMode({1280u, 720u}), "Go Game");

    // Load background music (loop forever)
    if (!bgMusic.openFromFile(BG_MUSIC_PATH))
    {
        std::cerr << "[App] Failed to load bg music: " << BG_MUSIC_PATH << "\n";
    }
    else
    {
        bgMusic.setLooping(true);
        bgMusic.setVolume(musicVolume);
        musicEnabled = true;
        bgMusic.play();
    }

    auto navigate = [this](const std::string& name) mutable
    {
        if (name == "Quit")
        {
            window.close();
            return;
        }

        if (name == "Game")
        {
            int boardSize = 9;
            int mode      = 0;

            std::ifstream in(PREGAME_CONFIG_PATH);
            if (in)
            {
                in >> boardSize >> mode;
                in.close();
            }

            if (boardSize != 9 && boardSize != 13 && boardSize != 19)
                boardSize = 9;

            if (gameScreen)
            {
                gameScreen->setBoardSize(boardSize);
                gameScreen->setCurrentPlayer(0); // Black first
            }
        }

        screens.switchTo(name);
    };

    // Menu
    screens.addScreen("Menu", std::make_unique<MenuScreen>(navigate));

    // Pre-game
    screens.addScreen("PreGame", std::make_unique<PreGameScreen>(navigate));

    // Game
    {
        auto gs = std::make_unique<GameScreen>(navigate);
        gameScreen = gs.get();
        screens.addScreen("Game", std::move(gs));
    }

    // Settings (volume slider + board skins)
    screens.addScreen("Settings",
        std::make_unique<SettingsScreen>(
            navigate,
            [this](float v) { setMusicVolume(v); },
            [this]() -> float { return getMusicVolume(); }
        )
    );

    screens.switchTo("Menu");
}

void App::handleGlobalEvent(const sf::Event& e)
{
    if (auto closed = e.getIf<sf::Event::Closed>())
    {
        (void)closed;
        window.close();
        return;
    }

    if (auto key = e.getIf<sf::Event::KeyPressed>())
    {
        if (key->scancode == sf::Keyboard::Scancode::Escape)
            window.close();
    }
}

void App::startMusic()
{
    if (bgMusic.getStatus() != sf::SoundSource::Status::Playing)
        bgMusic.play();
    musicEnabled = true;
}

void App::stopMusic()
{
    if (bgMusic.getStatus() != sf::SoundSource::Status::Stopped)
        bgMusic.stop();
    musicEnabled = false;
}

void App::toggleMusic()
{
    if (musicEnabled) stopMusic();
    else startMusic();
}

void App::setMusicVolume(float v)
{
    musicVolume = std::clamp(v, 0.f, 100.f);
    bgMusic.setVolume(musicVolume);

    // If user raises volume from 0 while stopped, keep your current behavior:
    // (we do NOT auto-start; only volume changes)
}

float App::getMusicVolume() const
{
    return musicVolume;
}

void App::run()
{
    sf::Clock clock;

    while (window.isOpen())
    {
        while (auto opt = window.pollEvent())
        {
            const sf::Event& e = *opt;

            handleGlobalEvent(e);
            if (!window.isOpen())
                break;

            screens.handleEvent(e);
        }

        float dt = clock.restart().asSeconds();
        screens.update(dt);

        window.clear(sf::Color::Black);
        screens.draw(window);
        window.display();
    }
}
