#include "App.h"

#include <fstream>
#include <iostream>

#include "screens/MenuScreen.h"
#include "screens/BlankScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/PreGameScreen.h"
#include "screens/GameScreen.h"

namespace
{
    // Giống với PreGameScreen
    constexpr const char* PREGAME_CONFIG_PATH = "pregame_tmp.txt";

    // Đường dẫn nhạc nền
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

    // ---- Nhạc nền ----
    if (!bgMusic.openFromFile(BG_MUSIC_PATH))
    {
        std::cerr << "[App] Failed to load bg music: "
                  << BG_MUSIC_PATH << "\n";
    }
    else
    {
        bgMusic.setLooping(true);   // phát lặp vô hạn
        bgMusic.setVolume(40.f);    // 0–100, tuỳ bạn chỉnh
        bgMusic.play();             // phát ngay khi mở game
        musicEnabled = true;
    }

    // ==== Toggle nhạc cho SettingsScreen ====
    auto toggleMusic = [this]()
    {
        if (musicEnabled)
        {
            // Đang bật -> TẮT
            bgMusic.stop();
            musicEnabled = false;
        }
        else
        {
            // Đang tắt -> BẬT LẠI TỪ ĐẦU
            bgMusic.stop();                          // đảm bảo dừng hẳn
            bgMusic.setPlayingOffset(sf::Time::Zero); // tua về 0
            bgMusic.play();                          // phát lại
            musicEnabled = true;
        }
    };

    // ==== navigate callback cho tất cả Screen ====
    auto navigate = [this, toggleMusic](const std::string& name) mutable
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
            {
                in >> boardSize >> mode;
            }

            if (boardSize != 9 && boardSize != 13 && boardSize != 19)
                boardSize = 9;

            if (gameScreen)
            {
                gameScreen->setBoardSize(boardSize);
                gameScreen->setCurrentPlayer(0); // luôn để Black đi trước
            }
        }

        screens.switchTo(name);
    };

    // ==== Đăng ký các màn hình ====
    screens.addScreen("Menu",
        std::make_unique<MenuScreen>(navigate));

    screens.addScreen("PreGame",
        std::make_unique<PreGameScreen>(navigate));

    // Gameplay screen mới – lưu lại con trỏ
    {
        auto gs = std::make_unique<GameScreen>(navigate);
        gameScreen = gs.get();
        screens.addScreen("Game", std::move(gs));
    }

    // Settings: truyền thêm toggleMusic
    screens.addScreen("Settings",
        std::make_unique<SettingsScreen>(navigate, toggleMusic));

    // Các màn hình khác
    screens.addScreen("2P",
        std::make_unique<BlankScreen>("2 Players Mode - coming soon"));
    screens.addScreen("PracticeAI",
        std::make_unique<BlankScreen>("Practice with AI - coming soon"));
    screens.addScreen("Challenge",
        std::make_unique<BlankScreen>("Challenge - coming soon"));

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
