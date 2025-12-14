#include "screens/GameScreen.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <queue>

namespace
{
    constexpr const char* FONT_UI_PATH         = "assets/fonts/Inter_28pt-SemiBold.ttf";
    constexpr const char* SAVE_PATH            = "savegame.txt";
    constexpr const char* PREGAME_CONFIG_PATH  = "pregame_tmp.txt";
    constexpr const char* BG_IMAGE_PATH        = "assets/img/menu_bg.jpg";
    constexpr const char* SETTINGS_PATH        = "settings.cfg";

    constexpr const char* MYTH_JPG   = "assets/img/Myth.jpg";
    constexpr const char* MYTH_PNG   = "assets/img/Myth.png";
    constexpr const char* PIRATE_JPG = "assets/img/Pirate.jpg";
    constexpr const char* PIRATE_PNG = "assets/img/Pirate.png";

    
    constexpr const char* WEAPON_BLACK = "assets/img/stone/Weapon/Weapon_black.png";
    constexpr const char* WEAPON_WHITE = "assets/img/stone/Weapon/Weapon_white.png";
    constexpr const char* STPIRATE_BLACK = "assets/img/stone/Pirate/Pirate_black.png";
    constexpr const char* STPIRATE_WHITE = "assets/img/stone/Pirate/Pirate_white.png";

    static void centerOrigin(sf::Text& t)
    {
        sf::FloatRect b = t.getLocalBounds();
        float cx = b.position.x + b.size.x * 0.5f;
        float cy = b.position.y + b.size.y * 0.5f;
        t.setOrigin(sf::Vector2f{cx, cy});
    }

    static bool tryLoadTexture(sf::Texture& tex, const char* jpg, const char* png)
    {
        if (tex.loadFromFile(jpg)) return true;
        if (tex.loadFromFile(png)) return true;
        return false;
    }

    static sf::Color defaultWoodColor()
    {
        return sf::Color(214, 177, 122);
    }

    static sf::Color darkBlueColor()
    {
        
        return sf::Color(0x28, 0x38, 0x78);
    }

    static std::string trim(const std::string& s)
    {
        auto isSpace = [](unsigned char ch){ return std::isspace(ch) != 0; };
        std::size_t a = 0;
        while (a < s.size() && isSpace((unsigned char)s[a])) ++a;
        std::size_t b = s.size();
        while (b > a && isSpace((unsigned char)s[b - 1])) --b;
        return s.substr(a, b - a);
    }

    static std::string loadBoardThemeOrDefault()
    {
        std::ifstream in(SETTINGS_PATH);
        if (!in) return "Default";

        std::string line;
        while (std::getline(in, line))
        {
            line = trim(line);
            if (line.rfind("boardTheme=", 0) == 0)
            {
                std::string t = trim(line.substr(11));
                if (t == "Default" || t == "DarkBlue" || t == "Myth" || t == "Pirate")
                    return t;
            }
        }
        return "Default";
    }

    static std::string loadStoneThemeOrDefault()
    {
        std::ifstream in(SETTINGS_PATH);
        if (!in) return "Default";

        std::string line;
        while (std::getline(in, line))
        {
            line = trim(line);
            if (line.rfind("stoneTheme=", 0) == 0)
            {
                std::string t = trim(line.substr(11));
                if (t == "Default" || t == "Weapon" || t == "Pirate")
                    return t;
            }
        }
        return "Default";
    }
} // namespace

GameScreen::GameScreen(NavigateFn onNavigate)
    : navigate(std::move(onNavigate))
    , game(9)

    , ai(AIDifficulty::Easy)
    , vsAI(false)
    , aiPlayerIndex(1)
    , pendingAIMove(false)
    , aiThinkTimer(0.f)

    , font()
    , bgTexture()
    , bgSprite(nullptr)
    , titleText(font, "", 32U)
    , boardSizeText(font, "", 24U)
    , turnText(font, "", 24U)
    , blackScoreText(font, "", 24U)
    , whiteScoreText(font, "", 24U)
    , statusText(font, "", 20U)
    , turnPanelRect()
    , turnPanelText(font, "", 28U)
    , btnPass(font, "Pass", 24U)
    , btnSave(font, "Save game", 24U)
    , btnBackMenu(font, "Back to menu", 24U)
    , btnFinishGame(font, "Finish and Score", 24U)
    , btnUndo(font, "Undo", 24U)
    , btnRedo(font, "Redo", 24U)
    , boardRect()

    , boardTheme("Default")
    , boardSkinTexture()
    , boardSkinLoaded(false)

    , stoneTheme("Default")
    , stoneBlackTexture()
    , stoneWhiteTexture()
    , stoneSkinLoaded(false)

    , boardOrigin(0.f, 0.f)
    , boardPixelSize(0.f)
    , cellSize(0.f)
    , layoutDone(false)
    , statusTimer(0.f)
{
    if (!font.openFromFile(FONT_UI_PATH))
        std::cerr << "[GameScreen] Failed to load font: " << FONT_UI_PATH << "\n";

    if (!bgTexture.loadFromFile(BG_IMAGE_PATH))
        std::cerr << "[GameScreen] Failed to load background: " << BG_IMAGE_PATH << "\n";
    else
    {
        bgTexture.setSmooth(true);
        bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    }

    titleText.setString("");
    titleText.setFillColor(sf::Color::White);

    boardSizeText.setFillColor(sf::Color::White);
    turnText.setFillColor(sf::Color::White);
    blackScoreText.setFillColor(sf::Color::White);
    whiteScoreText.setFillColor(sf::Color::White);

    statusText.setFillColor(sf::Color(0, 255, 0));

    turnPanelRect.setFillColor(sf::Color(30, 30, 30, 220));
    turnPanelRect.setOutlineColor(sf::Color::White);
    turnPanelRect.setOutlineThickness(2.f);

    turnPanelText.setFillColor(sf::Color::White);

    btnFinishGame.setOnClick([this]()
    {
        if (!game.isMarkingDead())
        {
            statusText.setString("You can only finish the game after both players pass.");
            statusTimer = 2.0f;
            return;
        }

        auto s = game.finalizeScore();

        auto toStr1 = [](double x) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.1f", x);
            return std::string(buf);
        };

        std::string text;
        text  = "Final result (Japanese scoring)\n";
        text += "Black: " + toStr1(s.blackTotal)
            + "  [Territory " + std::to_string(s.blackTerritory)
            + ", Captures " + std::to_string(s.blackCaptures) + "]\n";
        text += "White: " + toStr1(s.whiteTotal)
            + "  [Territory " + std::to_string(s.whiteTerritory)
            + ", Captures " + std::to_string(s.whiteCaptures)
            + ", Komi " + toStr1(s.komi) + "]\n";

        double diff = s.blackTotal - s.whiteTotal;
        if (diff > 0.0) text += "\nBlack wins by " + toStr1(diff) + " points.";
        else if (diff < 0.0) text += "\nWhite wins by " + toStr1(-diff) + " points.";
        else text += "\nResult: Draw.";

        statusText.setString(text);
        statusTimer = 0.f;

        turnPanelText.setString("Game over");
        centerOrigin(turnPanelText);
    });

    btnPass.setOnClick([this]()
    {
        GoGame::MoveResult res = game.pass();

        if (!res.ok)
        {
            statusText.setString(res.message);
            statusTimer = 2.0f;
            return;
        }

        if (game.isMarkingDead())
        {
            updateScorePreview();
            turnPanelText.setString("Mark dead stones");
            centerOrigin(turnPanelText);
            return;
        }

        statusText.setString(res.message);

        if (game.isGameOver()) statusTimer = 0.f;
        else statusTimer = 2.0f;

        updateTurnText();
        updateTurnPanel();
        updateScoreTexts();
    });

    
    btnSave.setOnClick([this]()
    {
        bool okBoard = game.saveToFile(SAVE_PATH);

        
        bool okMeta = true;
        {
            std::ofstream meta("save_ai.txt");
            if (meta)
            {
                
                int vsFlag = vsAI ? 1 : 0;

                
                int diffInt = static_cast<int>(ai.getDifficulty());

                
                meta << vsFlag << " " << diffInt << " " << aiPlayerIndex << "\n";
            }
            else
            {
                okMeta = false;
            }
        }

        if (okBoard && okMeta)
        {
            statusText.setString("Game saved!");
        }
        else if (okBoard && !okMeta)
        {
            statusText.setString("Board saved, but AI meta save failed.");
        }
        else
        {
            statusText.setString("Save failed!");
        }
        statusTimer = 2.0f;
    });



    btnBackMenu.setOnClick([this]()
    {
        if (navigate) navigate("Menu");
    });

    btnUndo.setOnClick([this]()
    {
        if (pendingAIMove) return;

        bool changed = false;

        if (vsAI)
        {
            if (game.canUndo()) { game.undo(); changed = true; }
            if (game.canUndo()) { game.undo(); changed = true; }
        }
        else
        {
            if (game.canUndo()) { game.undo(); changed = true; }
        }

        if (changed)
        {
            updateTurnText();
            updateTurnPanel();
            updateScoreTexts();
            statusText.setString("Undid move(s).");
            statusTimer = 1.5f;
        }
    });

    btnRedo.setOnClick([this]()
    {
        if (pendingAIMove) return;

        bool changed = false;

        if (vsAI)
        {
            if (game.canRedo()) { game.redo(); changed = true; }
            if (game.canRedo()) { game.redo(); changed = true; }
        }
        else
        {
            if (game.canRedo()) { game.redo(); changed = true; }
        }

        if (changed)
        {
            updateTurnText();
            updateTurnPanel();
            updateScoreTexts();
            statusText.setString("Redid move(s).");
            statusTimer = 1.5f;
        }
    });

    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();
}

void GameScreen::setBoardSize(int size)
{
    if (size == 9 || size == 13 || size == 19)
    {
        game.reset(size);
        layoutDone = false;

        statusText.setString("");
        statusTimer = 0.f;

        updateTurnText();
        updateTurnPanel();
        updateScoreTexts();
    }
}

void GameScreen::setCurrentPlayer(int player)
{
    if (player == 0 || player == 1)
    {
        game.setCurrentPlayer(player);
        updateTurnText();
        updateTurnPanel();
    }
}

void GameScreen::updateTurnText()
{
    std::string turnStr = "Turn: " + std::string(game.getCurrentPlayer() == 0 ? "Black" : "White");
    turnText.setString(turnStr);
}

void GameScreen::updateTurnPanel()
{
    turnPanelText.setString(game.getCurrentPlayer() == 0 ? "Black Go!" : "White Go!");
    centerOrigin(turnPanelText);
}

void GameScreen::updateScoreTexts()
{
    blackScoreText.setString("Black captured: " + std::to_string(game.getBlackCaptured()));
    whiteScoreText.setString("White captured: " + std::to_string(game.getWhiteCaptured()));
}

void GameScreen::loadBoardTheme()
{
    boardTheme = loadBoardThemeOrDefault();

    boardSkinLoaded = false;
    boardSkinTexture = sf::Texture();

    if (boardTheme == "Myth")
    {
        if (tryLoadTexture(boardSkinTexture, MYTH_JPG, MYTH_PNG))
        {
            boardSkinTexture.setSmooth(true);
            boardSkinLoaded = true;
        }
    }
    else if (boardTheme == "Pirate")
    {
        if (tryLoadTexture(boardSkinTexture, PIRATE_JPG, PIRATE_PNG))
        {
            boardSkinTexture.setSmooth(true);
            boardSkinLoaded = true;
        }
    }
}

void GameScreen::applyBoardThemeToRect()
{
    if ((boardTheme == "Myth" || boardTheme == "Pirate") && boardSkinLoaded)
    {
        boardRect.setTexture(&boardSkinTexture);
        boardRect.setFillColor(sf::Color::White);
    }
    else
    {
        boardRect.setTexture(nullptr);
        if (boardTheme == "DarkBlue") boardRect.setFillColor(darkBlueColor());
        else boardRect.setFillColor(defaultWoodColor());
    }
}

void GameScreen::loadStoneTheme()
{
    stoneTheme = loadStoneThemeOrDefault();
    applyStoneTheme();
}

void GameScreen::applyStoneTheme()
{
    stoneSkinLoaded = false;
    stoneBlackTexture = sf::Texture();
    stoneWhiteTexture = sf::Texture();

    if (stoneTheme == "Weapon")
    {
        if (stoneBlackTexture.loadFromFile(WEAPON_BLACK) &&
            stoneWhiteTexture.loadFromFile(WEAPON_WHITE))
        {
            stoneBlackTexture.setSmooth(true);
            stoneWhiteTexture.setSmooth(true);
            stoneSkinLoaded = true;
        }
    }
    else if (stoneTheme == "Pirate")
    {
        if (stoneBlackTexture.loadFromFile(STPIRATE_BLACK) &&
            stoneWhiteTexture.loadFromFile(STPIRATE_WHITE))
        {
            stoneBlackTexture.setSmooth(true);
            stoneWhiteTexture.setSmooth(true);
            stoneSkinLoaded = true;
        }
    }
}

void GameScreen::layout(const sf::Vector2u& winSize)
{
    const float winW = (float)winSize.x;
    const float winH = (float)winSize.y;

    if (bgSprite)
    {
        auto tex = bgTexture.getSize();
        float sx = winW / (float)tex.x;
        float sy = winH / (float)tex.y;
        bgSprite->setScale(sf::Vector2f{sx, sy});
        bgSprite->setPosition(sf::Vector2f{0.f, 0.f});
    }

    bool loadedFromPreGame = false;
    std::ifstream cfg(PREGAME_CONFIG_PATH);
    if (cfg)
    {
        int bSize = 0;
        int mode  = 0;
        cfg >> bSize >> mode;

        if (cfg && (bSize == 9 || bSize == 13 || bSize == 19))
        {
            game.reset(bSize);
            loadedFromPreGame = true;

            vsAI = (mode != 0);
            aiPlayerIndex = 1;

            if (vsAI)
            {
                if (mode == 1) ai.setDifficulty(AIDifficulty::Easy);
                else if (mode == 2) ai.setDifficulty(AIDifficulty::Medium);
                else ai.setDifficulty(AIDifficulty::Hard);
            }

            statusText.setString("");
            statusTimer = 0.f;
        }

        cfg.close();
        std::remove(PREGAME_CONFIG_PATH);
    }

    if (!loadedFromPreGame)
    {
        if (!game.loadFromFile(SAVE_PATH))
        {
            std::cout << "[GameScreen] No valid save, using default boardSize="
                    << game.getBoardSize() << "\n";
            
            vsAI = false;
        }
        else
        {
            std::cout << "[GameScreen] Loaded save from " << SAVE_PATH << "\n";

            
            vsAI = false;              
            aiPlayerIndex = 1;        
            ai.setDifficulty(AIDifficulty::Easy);

            std::ifstream meta("save_ai.txt");
            if (meta)
            {
                int vsFlag = 0;
                int diffInt = 1;
                int aiIdx = 1;
                if (meta >> vsFlag >> diffInt >> aiIdx)
                {
                    vsAI = (vsFlag != 0);
                    aiPlayerIndex = aiIdx;

                    if      (diffInt == 1) ai.setDifficulty(AIDifficulty::Easy);
                    else if (diffInt == 2) ai.setDifficulty(AIDifficulty::Medium);
                    else if (diffInt == 3) ai.setDifficulty(AIDifficulty::Hard);
                    
                }
            }
        }
    }


    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();

    std::string boardStr =
        "Board size: " + std::to_string(game.getBoardSize()) + " x " + std::to_string(game.getBoardSize());
    boardSizeText.setString(boardStr);
    boardSizeText.setPosition(sf::Vector2f{40.f, winH * 0.20f});

    turnText.setPosition(sf::Vector2f{40.f, winH * 0.26f});
    blackScoreText.setPosition(sf::Vector2f{40.f, winH * 0.32f});
    whiteScoreText.setPosition(sf::Vector2f{40.f, winH * 0.38f});
    statusText.setPosition(sf::Vector2f{40.f, winH * 0.44f});

    const float bottomMargin = 40.f;
    const float gapX         = 20.f;
    const float gapY         = 16.f;

    float currentXBottom = 40.f;
    float bottomRowY     = 0.f;

    {
        const float paddingX = 40.f;
        const float paddingY = 16.f;

        float textW = btnPass.textWidth();
        float textH = font.getLineSpacing(24U);

        float w = textW + paddingX;
        float h = textH + paddingY;

        bottomRowY = winH - h - bottomMargin;

        btnPass.setSize(sf::Vector2f{w, h});
        btnPass.setPosition(sf::Vector2f{currentXBottom, bottomRowY});
        currentXBottom += w + gapX;
    }

    {
        const float paddingX = 40.f;
        const float paddingY = 16.f;

        float textW = btnSave.textWidth();
        float textH = font.getLineSpacing(24U);

        float w = textW + paddingX;
        float h = textH + paddingY;

        btnSave.setSize(sf::Vector2f{w, h});
        btnSave.setPosition(sf::Vector2f{currentXBottom, bottomRowY});
        currentXBottom += w + gapX;
    }

    {
        const float paddingX = 48.f;
        const float paddingY = 18.f;

        float textW = btnBackMenu.textWidth();
        float textH = font.getLineSpacing(24U);

        float w = textW + paddingX;
        float h = textH + paddingY;

        btnBackMenu.setSize(sf::Vector2f{w, h});
        btnBackMenu.setPosition(sf::Vector2f{currentXBottom, bottomRowY});
    }

    {
        const float paddingX = 40.f;
        const float paddingY = 16.f;

        float textFinishW = btnFinishGame.textWidth();
        float textH       = font.getLineSpacing(24U);

        float w = textFinishW + paddingX;
        float h = textH       + paddingY;

        float midRowY = bottomRowY - h - gapY;

        btnFinishGame.setSize(sf::Vector2f{w, h});
        btnFinishGame.setPosition(sf::Vector2f{40.f, midRowY});

        float topRowY = midRowY - h - gapY;
        float x       = 40.f;

        btnUndo.setSize(sf::Vector2f{w, h});
        btnUndo.setPosition(sf::Vector2f{x, topRowY});

        x += w + gapX;

        btnRedo.setSize(sf::Vector2f{w, h});
        btnRedo.setPosition(sf::Vector2f{x, topRowY});
    }

    int nPoints = game.getBoardSize();
    if (nPoints != 9 && nPoints != 13 && nPoints != 19)
        nPoints = 9;

    int segments = nPoints - 1;
    float maxBoardSide = std::min(winH * 0.8f, winW * 0.6f);

    cellSize       = maxBoardSide / (float)segments;
    boardPixelSize = cellSize * (float)segments;

    float frameThickness = cellSize * 0.8f;

    boardRect.setSize(sf::Vector2f{
        boardPixelSize + 2.f * frameThickness,
        boardPixelSize + 2.f * frameThickness
    });

    
    loadBoardTheme();
    applyBoardThemeToRect();

    
    loadStoneTheme();

    boardRect.setOutlineColor(sf::Color(160, 110, 52));
    boardRect.setOutlineThickness(4.f);

    float boardX = winW - boardRect.getSize().x - 60.f;
    float boardY = (winH - boardRect.getSize().y) * 0.5f;
    boardRect.setPosition(sf::Vector2f{boardX, boardY});

    boardOrigin = sf::Vector2f{
        boardX + frameThickness,
        boardY + frameThickness
    };

    {
        updateTurnPanel();

        sf::FloatRect tb = turnPanelText.getLocalBounds();
        float textW = tb.size.x;
        float textH = tb.size.y;

        const float padX = 24.f;
        const float padY = 12.f;

        float leftMargin  = 40.f;
        float topMargin   = 40.f;
        float rightLimit  = boardRect.getPosition().x - 20.f;

        float panelW = rightLimit - leftMargin;
        if (panelW < textW + padX * 2.f)
            panelW = textW + padX * 2.f;

        float panelH = textH + padY * 2.f;

        turnPanelRect.setSize(sf::Vector2f{panelW, panelH});
        turnPanelRect.setPosition(sf::Vector2f{leftMargin, topMargin});

        float centerX = leftMargin + panelW * 0.5f;
        float centerY = topMargin  + panelH * 0.5f;
        turnPanelText.setPosition(sf::Vector2f{centerX, centerY});
    }

    layoutDone = true;
}

void GameScreen::handleBoardClick(int mouseX, int mouseY)
{
    if (game.isMarkingDead())
    {
        float mx = (float)mouseX;
        float my = (float)mouseY;

        float localX = mx - boardOrigin.x;
        float localY = my - boardOrigin.y;

        if (localX < 0.f || localY < 0.f) return;
        if (localX > boardPixelSize || localY > boardPixelSize) return;

        int n = game.getBoardSize();
        if (n != 9 && n != 13 && n != 19) n = 9;

        float fx = localX / cellSize;
        float fy = localY / cellSize;

        int j = (int)std::lround(fx);
        int i = (int)std::lround(fy);

        if (i < 0 || i >= n || j < 0 || j >= n) return;

        float gx = (float)j * cellSize;
        float gy = (float)i * cellSize;
        float dx = std::abs(localX - gx);
        float dy = std::abs(localY - gy);
        float maxDist = cellSize * 0.4f;
        if (dx > maxDist || dy > maxDist) return;

        GoGame::MarkDeadResult res = game.markDeadGroup(i, j);
        if (!res.ok)
        {
            statusText.setString(res.message);
            statusTimer = 2.0f;
        }
        else
        {
            updateScorePreview();
        }
        return;
    }

    if (game.isGameOver()) return;

    float mx = (float)mouseX;
    float my = (float)mouseY;

    float localX = mx - boardOrigin.x;
    float localY = my - boardOrigin.y;

    if (localX < 0.f || localY < 0.f) return;
    if (localX > boardPixelSize || localY > boardPixelSize) return;

    int n = game.getBoardSize();
    if (n != 9 && n != 13 && n != 19) n = 9;

    float fx = localX / cellSize;
    float fy = localY / cellSize;

    int j = (int)std::lround(fx);
    int i = (int)std::lround(fy);

    if (i < 0 || i >= n || j < 0 || j >= n) return;

    float gx = (float)j * cellSize;
    float gy = (float)i * cellSize;
    float dx = std::abs(localX - gx);
    float dy = std::abs(localY - gy);
    float maxDist = cellSize * 0.4f;
    if (dx > maxDist || dy > maxDist) return;

    GoGame::MoveResult res = game.playMove(i, j);
    if (!res.ok)
    {
        statusText.setString(res.message);
        statusTimer = 2.0f;
        return;
    }

    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();
    statusText.setString("");
    statusTimer = 0.f;

    if (vsAI && !game.isGameOver() && !game.isMarkingDead())
    {
        int curPlayer = game.getCurrentPlayer();
        if (curPlayer == aiPlayerIndex)
        {
            pendingAIMove = true;
            aiThinkTimer  = 1.0f;
            statusText.setString("AI is thinking...");
            statusTimer = 0.f;
        }
    }
}

void GameScreen::handleEvent(const sf::Event& e)
{
    if (!layoutDone) return;

    if (vsAI && pendingAIMove) return;

    btnUndo.handleEvent(e);
    btnRedo.handleEvent(e);
    btnPass.handleEvent(e);
    btnSave.handleEvent(e);
    btnBackMenu.handleEvent(e);
    btnFinishGame.handleEvent(e);

    if (auto mouse = e.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button == sf::Mouse::Button::Left)
        {
            if (!pendingAIMove)
                handleBoardClick(mouse->position.x, mouse->position.y);
        }
    }
}

void GameScreen::update(float dt)
{
    if (!game.isMarkingDead() && !game.isGameOver())
    {
        if (statusTimer > 0.f)
        {
            statusTimer -= dt;
            if (statusTimer <= 0.f)
            {
                statusTimer = 0.f;
                statusText.setString("");
            }
        }
    }

    if (pendingAIMove && !game.isGameOver() && !game.isMarkingDead())
    {
        aiThinkTimer -= dt;
        if (aiThinkTimer <= 0.f)
        {
            int curPlayer = game.getCurrentPlayer();
            if (vsAI && curPlayer == aiPlayerIndex)
            {
                int aiColor = (aiPlayerIndex == 0 ? GoGame::Black : GoGame::White);

                auto move = ai.chooseMove(game, aiColor);
                if (move.first >= 0 && move.second >= 0)
                {
                    GoGame::MoveResult aiRes = game.playMove(move.first, move.second);
                    if (!aiRes.ok)
                        std::cout << "[AI] Illegal move: " << aiRes.message << "\n";
                }
                else
                {
                    game.pass();
                }

                updateTurnText();
                updateTurnPanel();
                updateScoreTexts();
            }

            pendingAIMove = false;
            statusText.setString("");
            statusTimer = 0.f;
        }
    }
}

void GameScreen::updateScorePreview()
{
    GoGame::JapaneseScore s = game.computeJapaneseScoreWithDead();

    auto toStr1 = [](double x) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f", x);
        return std::string(buf);
    };

    std::string text;
    text  = "Mark-dead mode (Japanese scoring)\n";
    text += "Black: " + toStr1(s.blackTotal)
         + "  [Territory " + std::to_string(s.blackTerritory)
         + ", Captures " + std::to_string(s.blackCaptures) + "]\n";
    text += "White: " + toStr1(s.whiteTotal)
         + "  [Territory " + std::to_string(s.whiteTerritory)
         + ", Captures " + std::to_string(s.whiteCaptures)
         + ", Komi " + toStr1(s.komi) + "]\n";
    text += "Neutral (dame): " + std::to_string(s.neutral);

    statusText.setString(text);
    statusTimer = 0.f;
}

void GameScreen::draw(sf::RenderWindow& window)
{
    if (!layoutDone)
        layout(window.getSize());

    if (bgSprite)
        window.draw(*bgSprite);

    window.draw(boardSizeText);
    window.draw(turnText);
    window.draw(blackScoreText);
    window.draw(whiteScoreText);

    if (!statusText.getString().isEmpty())
        window.draw(statusText);

    window.draw(turnPanelRect);
    window.draw(turnPanelText);

    window.draw(boardRect);

    int nPoints = game.getBoardSize();
    if (nPoints != 9 && nPoints != 13 && nPoints != 19)
        nPoints = 9;

    sf::Color gridColor = sf::Color::Black;

    for (int i = 0; i < nPoints; ++i)
    {
        float offset = (float)i * cellSize;

        float x = boardOrigin.x + offset;
        sf::RectangleShape vLine;
        vLine.setSize(sf::Vector2f{1.f, boardPixelSize});
        vLine.setFillColor(gridColor);
        vLine.setPosition(sf::Vector2f{x, boardOrigin.y});
        window.draw(vLine);

        float y = boardOrigin.y + offset;
        sf::RectangleShape hLine;
        hLine.setSize(sf::Vector2f{boardPixelSize, 1.f});
        hLine.setFillColor(gridColor);
        hLine.setPosition(sf::Vector2f{boardOrigin.x, y});
        window.draw(hLine);
    }

    int n = game.getBoardSize();
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    float stoneRadius = cellSize * 0.35f;
    float stoneDiameter = stoneRadius * 2.f;

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            int v = game.getCell(i, j);
            if (v == GoGame::Empty)
                continue;

            float cx = boardOrigin.x + (float)j * cellSize;
            float cy = boardOrigin.y + (float)i * cellSize;

            if (stoneSkinLoaded)
            {
                const sf::Texture& tex = (v == GoGame::Black) ? stoneBlackTexture : stoneWhiteTexture;

                sf::Sprite sp(tex); 
                sf::Vector2u tsz = tex.getSize();
                if (tsz.x > 0 && tsz.y > 0)
                {
                    sp.setOrigin(sf::Vector2f{ (float)tsz.x * 0.5f, (float)tsz.y * 0.5f });

                    float sx = stoneDiameter / (float)tsz.x;
                    float sy = stoneDiameter / (float)tsz.y;
                    float s  = std::min(sx, sy);
                    sp.setScale(sf::Vector2f{s, s});
                }

                sp.setPosition(sf::Vector2f{cx, cy});
                window.draw(sp);
            }
            else
            {
                sf::CircleShape stone(stoneRadius);
                stone.setOrigin(sf::Vector2f{stoneRadius, stoneRadius});
                stone.setPosition(sf::Vector2f{cx, cy});

                if (v == GoGame::Black)
                {
                    stone.setFillColor(sf::Color::Black);
                    stone.setOutlineColor(sf::Color(80, 80, 80));
                }
                else
                {
                    stone.setFillColor(sf::Color::White);
                    stone.setOutlineColor(sf::Color(160, 160, 160));
                }
                stone.setOutlineThickness(2.f);

                window.draw(stone);
            }
        }
    }

    btnPass.draw(window);
    btnSave.draw(window);
    btnBackMenu.draw(window);
    btnFinishGame.draw(window);
    btnRedo.draw(window);
    btnUndo.draw(window);
}
