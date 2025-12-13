
#include "screens/SettingsScreen.h"
#include <iostream>

namespace
{
    constexpr const char* FONT_UI_PATH  = "assets/fonts/Inter_28pt-SemiBold.ttf";
    constexpr const char* BG_IMAGE_PATH = "assets/img/menu_bg.jpg";

    // Căn origin text vào giữa
    void centerOrigin(sf::Text& t)
    {
        sf::FloatRect b = t.getLocalBounds(); 
        const float cx = b.position.x + b.size.x * 0.5f;
        const float cy = b.position.y + b.size.y * 0.5f;
        t.setOrigin(sf::Vector2f{cx, cy});
    }
} // namespace

SettingsScreen::SettingsScreen(NavigateFn onNavigate,
                               ToggleMusicFn onToggleMusic)
    : navigate(std::move(onNavigate))
    , toggleMusic(std::move(onToggleMusic))
    , bgTexture()
    , bgSprite(nullptr)
    , font()
    , title(font, "Settings", 40U)
    , btnReturn(font, "Return", 28U)
    , options()
    , layoutDone(false)
{
    // Font
    if (!font.openFromFile(FONT_UI_PATH))
    {
        std::cerr << "[SettingsScreen] Failed to load font: "
                  << FONT_UI_PATH << "\n";
    }

    // Style title
    title.setFillColor(sf::Color::Black);// màu đen

    // Background
    if (!bgTexture.loadFromFile(BG_IMAGE_PATH))
    {
        std::cerr << "[SettingsScreen] Failed to load background: "
                  << BG_IMAGE_PATH << "\n";
    }
    else
    {
        bgTexture.setSmooth(true);
        bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    }

    // Options
    options.emplace_back(font, "Board color",      28U);
    options.emplace_back(font, "Stone design",     28U);
    options.emplace_back(font, "Sound effects",    28U);
    options.emplace_back(font, "Background music", 28U); // nút cuối là toggle nhạc

    // Callback nút Return
    btnReturn.setOnClick([this]()
    {
        if (navigate)
            navigate("Menu");
    });

    for (std::size_t i = 0; i < options.size(); ++i)
    {
        if (i == options.size() - 1)
        {
            options[i].setOnClick([this]()
            {
                if (toggleMusic)
                    toggleMusic();// bật / tắt nhạc
            });
        }
        else
        {
            
            options[i].setOnClick([]()
            {
                
            });
        }
    }
}

void SettingsScreen::layout(const sf::Vector2u& winSize)
{
    const float winW = static_cast<float>(winSize.x);
    const float winH = static_cast<float>(winSize.y);

    if (bgSprite)
    {
        const sf::Vector2u texSize = bgTexture.getSize();
        if (texSize.x != 0 && texSize.y != 0)
        {
            const float sx = winW / static_cast<float>(texSize.x);
            const float sy = winH / static_cast<float>(texSize.y);
            bgSprite->setScale(sf::Vector2f{sx, sy});
        }
        bgSprite->setPosition(sf::Vector2f{0.f, 0.f});
    }

    centerOrigin(title);
    const float titleY = winH * 0.28f;   
    title.setPosition(sf::Vector2f{winW * 0.5f, titleY});

    const float startY = winH * 0.38f;  
    const float gapY   = 70.f;
    const float cx     = winW * 0.5f;

    if (!options.empty())
    {
        Button& baseBtn   = options.back();
        const float baseTextW = baseBtn.textWidth();
        const float baseH     = baseBtn.height();
        const float paddingX  = 40.f;
        const float baseW     = baseTextW + paddingX;

        for (std::size_t i = 0; i < options.size(); ++i)
        {
            Button& b = options[i];
            b.setSize(sf::Vector2f{baseW, baseH});

            const float x = cx - baseW * 0.5f;
            const float y = startY + gapY * static_cast<float>(i);
            b.setPosition(sf::Vector2f{x, y});
        }
    }

    const float margin   = 16.f;
    const float paddingX = 48.f;    // rộng hơn
    const float paddingY = 18.f;    // cao hơn

    const float textW = btnReturn.textWidth();
    const float textH = btnReturn.height();

    const float btnW = textW + paddingX;
    const float btnH = textH + paddingY;

    btnReturn.setSize(sf::Vector2f{btnW, btnH});

    const float posX = winW - btnReturn.width()  - margin;
    const float posY = winH - btnReturn.height() - margin;

    btnReturn.setPosition(sf::Vector2f{posX, posY});

    layoutDone = true;
}

void SettingsScreen::handleEvent(const sf::Event& e)
{
    if (!layoutDone)
        return;

    for (auto& b : options)
        b.handleEvent(e);

    btnReturn.handleEvent(e);
}

void SettingsScreen::update(float )
{
}

void SettingsScreen::draw(sf::RenderWindow& window)
{
    if (!layoutDone)
        layout(window.getSize());

    if (bgSprite)
        window.draw(*bgSprite);

    window.draw(title);

    for (const auto& b : options)
        b.draw(window);

    btnReturn.draw(window);
}
