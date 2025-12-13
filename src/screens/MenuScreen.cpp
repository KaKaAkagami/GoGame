
#include "screens/MenuScreen.h"
#include <iostream>

namespace
{
    constexpr const char* FONT_UI_PATH    = "assets/fonts/Inter_28pt-SemiBold.ttf";
    constexpr const char* FONT_TITLE_PATH = "assets/fonts/FZShuTi.ttf";
    constexpr const char* BG_IMAGE_PATH   = "assets/img/menu_bg.jpg";

    void centerOrigin(sf::Text& t)
    {
        sf::FloatRect b = t.getLocalBounds();
        const float cx = b.position.x + b.size.x * 0.5f;
        const float cy = b.position.y + b.size.y * 0.5f;
        t.setOrigin(sf::Vector2f{cx, cy});
    }
} // namespace

MenuScreen::MenuScreen(NavigateFn onNavigate)
    : navigate(std::move(onNavigate))
    , bgTexture()
    , bgSprite(nullptr)
    , fontUI()
    , fontTitle()
    , title(fontTitle, "Go Game", 64U)
    , madeBy(fontUI, "Made by: Nguyen Minh Khang and Dang Xuan Phat - CS160", 18U)
    , buttons()
    , layoutDone(false)
{
    // Font
    if (!fontUI.openFromFile(FONT_UI_PATH) ||
        !fontTitle.openFromFile(FONT_TITLE_PATH))
    {
        std::cerr << "[MenuScreen] Failed to load fonts\n";
    }

    title.setFillColor(sf::Color::Black);
    madeBy.setFillColor(sf::Color(0x3B, 0x2F, 0x2F));

    // Background
    if (!bgTexture.loadFromFile(BG_IMAGE_PATH))
    {
        std::cerr << "[MenuScreen] Failed to load background\n";
    }
    else
    {
        bgTexture.setSmooth(true);
        bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    }

    // 3 nút trung tâm + 1 nút Exit ở góc:
    // 0: PLAY
    // 1: Continue game
    // 2: Settings
    // 3: Exit
    buttons.emplace_back(fontUI, "PLAY", 28U);// 0
    buttons.emplace_back(fontUI, "Continue game", 28U);// 1
    buttons.emplace_back(fontUI, "Settings", 28U);// 2
    buttons.emplace_back(fontUI, "Exit", 28U);// 3

    buttons[0].setOnClick([this]() {
        if (navigate) navigate("PreGame");
    });

    buttons[1].setOnClick([this]() {
        if (navigate) navigate("Game");
    });

    buttons[2].setOnClick([this]() {
        if (navigate) navigate("Settings");
    });

    buttons[3].setOnClick([this]() {
        if (navigate) navigate("Quit");
    });
}

void MenuScreen::layout(const sf::Vector2u& winSize)
{
    const float winW = static_cast<float>(winSize.x);
    const float winH = static_cast<float>(winSize.y);

    // Background full màn
    if (bgSprite)
    {
        const sf::Vector2u tex = bgTexture.getSize();
        if (tex.x && tex.y)
        {
            float sx = winW / static_cast<float>(tex.x);
            float sy = winH / static_cast<float>(tex.y);
            bgSprite->setScale(sf::Vector2f{sx, sy});
        }
        bgSprite->setPosition(sf::Vector2f{0.f, 0.f});
    }

    // Title
    centerOrigin(title);
    title.setPosition(sf::Vector2f{winW * 0.5f, winH * 0.20f});

    centerOrigin(madeBy);
    madeBy.setPosition(sf::Vector2f{winW * 0.5f, winH * 0.20f + 40.f});

    //3 nút trung tâm: PLAY, Continue, Settings 
    const float startY = winH * 0.40f;
    const float gapY   = 70.f;

    // Lấy width lớn nhất để 3 nút đều nhau
    float maxTextW = 0.f;
    float baseH    = 0.f;

    for (int i = 0; i < 3; ++i)
    {
        maxTextW = std::max(maxTextW, buttons[i].textWidth());
        baseH    = buttons[i].height(); //cùng font size nên giống nhau
    }

    float baseW = maxTextW + 40.f;

    for (int i = 0; i < 3; ++i)
    {
        buttons[i].setSize(sf::Vector2f{baseW, baseH});

        float x = winW * 0.5f - baseW * 0.5f;
        float y = startY + gapY * static_cast<float>(i);

        buttons[i].setPosition(sf::Vector2f{x, y});
    }

    //nút Exit bên phải dưới 
    {
        Button& e = buttons[3];
        e.setSize(sf::Vector2f{
            e.textWidth() + 40.f,
            e.height()
        });

        float x = winW - e.width() - 16.f;
        float y = winH - e.height() - 16.f;
        e.setPosition(sf::Vector2f{x, y});
    }

    layoutDone = true;
}

void MenuScreen::handleEvent(const sf::Event& e)
{
    if (!layoutDone) return;

    for (auto& b : buttons)
        b.handleEvent(e);
}

void MenuScreen::update(float) {}

void MenuScreen::draw(sf::RenderWindow& window)
{
    if (!layoutDone)
        layout(window.getSize());

    if (bgSprite)
        window.draw(*bgSprite);

    window.draw(title);
    window.draw(madeBy);

    for (auto& b : buttons)
        b.draw(window);
}
