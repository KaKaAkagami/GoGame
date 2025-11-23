// src/screens/PreGameScreen.cpp
#include "screens/PreGameScreen.h"
#include <iostream>
#include <algorithm>
#include <fstream>

namespace
{
    constexpr const char* FONT_UI_PATH  = "assets/fonts/Inter_28pt-SemiBold.ttf";
    constexpr const char* BG_IMAGE_PATH = "assets/img/menu_bg.jpg";

    // file tạm để truyền lựa chọn sang GameScreen
    constexpr const char* PREGAME_CONFIG_PATH = "pregame_tmp.txt";

    void centerOrigin(sf::Text& t)
    {
        sf::FloatRect b = t.getLocalBounds();
        float cx = b.position.x + b.size.x * 0.5f;
        float cy = b.position.y + b.size.y * 0.5f;
        t.setOrigin(sf::Vector2f{cx, cy});
    }
} // namespace

PreGameScreen::PreGameScreen(NavigateFn onNavigate)
    : navigate(std::move(onNavigate))
    , bgTexture()
    , bgSprite(nullptr)
    , font()
    , labelBoard(font, "", 28U)
    , labelMode(font, "", 28U)
    , boardButtons()
    , btn2P(font, "2 Players Mode", 28U)
    , btnEasy(font, "Easy AI", 28U)
    , btnMedium(font, "Medium AI", 28U)
    , btnHard(font, "Hard AI", 28U)
    , btnStart(font, "Start the game", 28U)
    , btnReturn(font, "Return", 28U)
    , layoutDone(false)
    , selectedBoard(-1)
    , selectedMode(-1)
    , boardBtnPositions(3)
    , modeBtnPositions{}
{
    // Font
    if (!font.openFromFile(FONT_UI_PATH))
        std::cerr << "[PreGameScreen] Failed to load font\n";

    // Background
    if (!bgTexture.loadFromFile(BG_IMAGE_PATH))
        std::cerr << "[PreGameScreen] Failed to load background\n";
    else
    {
        bgTexture.setSmooth(true);
        bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    }

    // Labels
    labelBoard.setString("Select board size:");
    labelBoard.setFillColor(sf::Color::Black);

    labelMode.setString("Select game mode:");
    labelMode.setFillColor(sf::Color::Black);

    // Board size buttons
    boardButtons.emplace_back(font, "9 x 9", 28U);
    boardButtons.emplace_back(font, "13 x 13", 28U);
    boardButtons.emplace_back(font, "19 x 19", 28U);

    // 0 = 9x9, 1 = 13x13, 2 = 19x19
    boardButtons[0].setOnClick([this]()
    {
        selectedBoard = 0;
        std::cout << "[PreGameScreen] Selected board: 9x9\n";
    });
    boardButtons[1].setOnClick([this]()
    {
        selectedBoard = 1;
        std::cout << "[PreGameScreen] Selected board: 13x13\n";
    });
    boardButtons[2].setOnClick([this]()
    {
        selectedBoard = 2;
        std::cout << "[PreGameScreen] Selected board: 19x19\n";
    });

    // Game mode buttons
    btn2P.setOnClick([this]()
    {
        selectedMode = 0;
        std::cout << "[PreGameScreen] Mode: 2 Players\n";
    });
    btnEasy.setOnClick([this]()
    {
        selectedMode = 1;
        std::cout << "[PreGameScreen] Mode: Easy AI\n";
    });
    btnMedium.setOnClick([this]()
    {
        selectedMode = 2;
        std::cout << "[PreGameScreen] Mode: Medium AI\n";
    });
    btnHard.setOnClick([this]()
    {
        selectedMode = 3;
        std::cout << "[PreGameScreen] Mode: Hard AI\n";
    });

    // Start game
    btnStart.setOnClick([this]()
    {
        if (selectedBoard == -1 || selectedMode == -1)
        {
            std::cerr << "[PreGameScreen] Start clicked but selection incomplete\n";
            return;
        }

        int boardSize = 9;
        switch (selectedBoard)
        {
        case 0: boardSize = 9;  break;
        case 1: boardSize = 13; break;
        case 2: boardSize = 19; break;
        }

        std::ofstream out(PREGAME_CONFIG_PATH);
        if (out)
        {
            // Format: boardSize selectedMode
            out << boardSize << " " << selectedMode << "\n";
            std::cout << "[PreGameScreen] Wrote config: boardSize="
                      << boardSize << ", mode=" << selectedMode << "\n";
        }
        else
        {
            std::cerr << "[PreGameScreen] Cannot open "
                      << PREGAME_CONFIG_PATH << " for writing\n";
        }

        if (navigate)
            navigate("Game");
    });

    // Return về Menu
    btnReturn.setOnClick([this]()
    {
        if (navigate) navigate("Menu");
    });
}

void PreGameScreen::layout(const sf::Vector2u& winSize)
{
    float winW = static_cast<float>(winSize.x);
    float winH = static_cast<float>(winSize.y);

    // Background full màn
    if (bgSprite)
    {
        auto tex = bgTexture.getSize();
        float sx = winW / static_cast<float>(tex.x);
        float sy = winH / static_cast<float>(tex.y);
        bgSprite->setScale(sf::Vector2f{sx, sy});
        bgSprite->setPosition(sf::Vector2f{0.f, 0.f});
    }

    const float leftX = winW * 0.25f;
    const float topY  = winH * 0.20f;

    // Label 1
    labelBoard.setPosition(sf::Vector2f{leftX, topY});

    // ===== 3 nút board size trên 1 hàng =====
    const float centerY = topY + 90.f;

    Button& baseBtn = boardButtons[2];
    const float baseW = baseBtn.textWidth() + 60.f;
    const float baseH = baseBtn.height() + 8.f;

    for (auto& b : boardButtons)
        b.setSize(sf::Vector2f{baseW, baseH});

    float totalWidth = baseW * 3.f + 40.f * 2.f;
    float startX     = (winW - totalWidth) * 0.5f;

    for (int i = 0; i < 3; ++i)
    {
        float x = startX + i * (baseW + 40.f);
        boardButtons[i].setPosition(sf::Vector2f{x, centerY});
        boardBtnPositions[i] = sf::Vector2f{x, centerY};
    }

    // ===== Label 2 =====
    float label2Y = centerY + baseH + 80.f;
    labelMode.setPosition(sf::Vector2f{leftX, label2Y});

    // ===== 4 nút game mode =====
    float maxTextW = btn2P.textWidth();
    maxTextW = std::max(maxTextW, btnEasy.textWidth());
    maxTextW = std::max(maxTextW, btnMedium.textWidth());
    maxTextW = std::max(maxTextW, btnHard.textWidth());

    const float gmW = std::max(maxTextW + 60.f, baseW);
    const float gmH = baseH;

    const float gmGapX   = 40.f;
    const float gmTotalW = gmW * 4.f + gmGapX * 3.f;
    const float gmStartX = (winW - gmTotalW) * 0.5f;
    const float gmY      = label2Y + 70.f;

    btn2P.setSize(sf::Vector2f{gmW, gmH});
    btnEasy.setSize(sf::Vector2f{gmW, gmH});
    btnMedium.setSize(sf::Vector2f{gmW, gmH});
    btnHard.setSize(sf::Vector2f{gmW, gmH});

    modeBtnPositions[0] = sf::Vector2f{gmStartX + 0.f * (gmW + gmGapX), gmY};
    modeBtnPositions[1] = sf::Vector2f{gmStartX + 1.f * (gmW + gmGapX), gmY};
    modeBtnPositions[2] = sf::Vector2f{gmStartX + 2.f * (gmW + gmGapX), gmY};
    modeBtnPositions[3] = sf::Vector2f{gmStartX + 3.f * (gmW + gmGapX), gmY};

    btn2P.setPosition(modeBtnPositions[0]);
    btnEasy.setPosition(modeBtnPositions[1]);
    btnMedium.setPosition(modeBtnPositions[2]);
    btnHard.setPosition(modeBtnPositions[3]);

    // ===== Nút Start =====
    const float startBtnY = gmY + gmH + 70.f;

    btnStart.setSize(sf::Vector2f{gmW, gmH});
    btnStart.setPosition(sf::Vector2f{
        winW * 0.5f - gmW * 0.5f,
        startBtnY
    });

    // ===== Nút Return =====
    const float margin   = 16.f;
    const float paddingX = 48.f;
    const float paddingY = 18.f;

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

void PreGameScreen::handleEvent(const sf::Event& e)
{
    if (!layoutDone) return;

    for (auto& b : boardButtons)
        b.handleEvent(e);

    btn2P.handleEvent(e);
    btnEasy.handleEvent(e);
    btnMedium.handleEvent(e);
    btnHard.handleEvent(e);
    btnStart.handleEvent(e);

    btnReturn.handleEvent(e);
}

void PreGameScreen::update(float) {}

void PreGameScreen::draw(sf::RenderWindow& window)
{
    if (!layoutDone)
        layout(window.getSize());

    if (bgSprite)
        window.draw(*bgSprite);

    // Label board size
    window.draw(labelBoard);

    // Highlight board size
    if (selectedBoard >= 0 && selectedBoard < static_cast<int>(boardButtons.size()))
    {
        sf::RectangleShape rect;
        const float w = boardButtons[selectedBoard].width();
        const float h = boardButtons[selectedBoard].height();

        rect.setSize(sf::Vector2f{w + 6.f, h + 6.f});
        rect.setPosition(boardBtnPositions[selectedBoard] - sf::Vector2f{3.f, 3.f});
        rect.setFillColor(sf::Color(255, 255, 255, 30));
        rect.setOutlineColor(sf::Color::Yellow);
        rect.setOutlineThickness(3.f);
        window.draw(rect);
    }

    for (auto& b : boardButtons)
        b.draw(window);

    // Label game mode
    window.draw(labelMode);

    // Highlight game mode
    if (selectedMode >= 0 && selectedMode < 4)
    {
        sf::RectangleShape rect;
        Button* btn = nullptr;

        switch (selectedMode)
        {
        case 0: btn = &btn2P;     break;
        case 1: btn = &btnEasy;   break;
        case 2: btn = &btnMedium; break;
        case 3: btn = &btnHard;   break;
        }

        if (btn)
        {
            float w = btn->width();
            float h = btn->height();

            rect.setSize(sf::Vector2f{w + 6.f, h + 6.f});
            rect.setPosition(modeBtnPositions[selectedMode] - sf::Vector2f{3.f, 3.f});
            rect.setFillColor(sf::Color(255, 255, 255, 30));
            rect.setOutlineColor(sf::Color::Yellow);
            rect.setOutlineThickness(3.f);
            window.draw(rect);
        }
    }

    btn2P.draw(window);
    btnEasy.draw(window);
    btnMedium.draw(window);
    btnHard.draw(window);

    btnStart.draw(window);
    btnReturn.draw(window);
}
