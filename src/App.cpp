#include "App.h"

#include <fstream>
#include <iostream>

#include "screens/MenuScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/PreGameScreen.h"
#include "screens/GameScreen.h"

namespace
{
    constexpr const char* SAVE_PATH = "savegame.txt";

    // giống PreGameScreen
    constexpr const char* PREGAME_CONFIG_PATH = "pregame_tmp.txt";

    // đường dẫn nhạc nền
    constexpr const char* BG_MUSIC_PATH = "assets/sfx/bg_music.ogg";
}

App::App()
    : window()
    , screens()
    , gameScreen(nullptr)
    , bgMusic()
    , musicEnabled(false)
{
    // SFML 3: VideoMode từ Vector2u
    window.create(sf::VideoMode({1280u, 720u}), "Go Game");

    //  Load background music 
    if (!bgMusic.openFromFile(BG_MUSIC_PATH))
    {
        std::cerr << "[App] Failed to load bg music: "
                  << BG_MUSIC_PATH << "\n";
    }
    else
    {
        bgMusic.setLooping(true);// phát lặp vô hạn
        bgMusic.setVolume(40.f);    
        musicEnabled = true;
        bgMusic.play();
    }

    //navigate callback cho tất cả Screen
    auto navigate = [this](const std::string& name) mutable
    {
        if (name == "Quit")
        {
            window.close();
            return;
        }

        // Trước khi vào Game, đọc lựa chọn từ PreGame
        if (name == "Game")
        {
            int boardSize = 9;
            int mode      = 0;

            std::ifstream in(PREGAME_CONFIG_PATH);
            if (in)
                in >> boardSize >> mode;
                in.close();

            if (boardSize != 9 && boardSize != 13 && boardSize != 19)
                boardSize = 9;

            if (gameScreen)
            {
                gameScreen->setBoardSize(boardSize);
                gameScreen->setCurrentPlayer(0); // Black đi trước
            }
            // std::remove(PREGAME_CONFIG_PATH);
            // std::remove(SAVE_PATH);
        }

        screens.switchTo(name);
    };


    // Menu
    screens.addScreen("Menu",
        std::make_unique<MenuScreen>(navigate));

    // Pre-game (chọn board size + mode)
    screens.addScreen("PreGame",
        std::make_unique<PreGameScreen>(navigate));

    // Gameplay screen – lưu lại con trỏ gameScreen
    {
        auto gs = std::make_unique<GameScreen>(navigate);
        gameScreen = gs.get();
        screens.addScreen("Game", std::move(gs));
    }

    // Settings – truyền callback toggleMusic
    screens.addScreen("Settings",
        std::make_unique<SettingsScreen>(
            navigate,
            [this]() { toggleMusic(); }
        ));


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

// cho nhạc 
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
    if (musicEnabled)
        stopMusic();
    else
        startMusic();
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
