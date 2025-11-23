#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Screen.h"
#include "widgets/Button.h"

class SettingsScreen : public Screen
{
public:
    using NavigateFn    = std::function<void(const std::string&)>;
    using ToggleMusicFn = std::function<void()>;   // 🆕 callback bật/tắt nhạc

    explicit SettingsScreen(NavigateFn onNavigate,
                            ToggleMusicFn onToggleMusic);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Hàm riêng để bố trí lại layout khi cửa sổ đổi size
    void layout(const sf::Vector2u& winSize);

private:
    NavigateFn    navigate;
    ToggleMusicFn toggleMusic;   // 🆕 lưu callback

    // Background
    sf::Texture                         bgTexture;
    std::unique_ptr<sf::Sprite>         bgSprite;   // dùng unique_ptr giống IconButton

    // UI
    sf::Font            font;
    sf::Text            title;
    Button              btnReturn;
    std::vector<Button> options;

    bool layoutDone;
};
