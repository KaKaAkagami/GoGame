// src/screens/GameScreen.cpp
#include "screens/GameScreen.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>   // std::remove
#include <cmath>
#include <queue>    // cho BFS group

namespace
{
    constexpr const char* FONT_UI_PATH = "assets/fonts/Inter_28pt-SemiBold.ttf";
    constexpr const char* SAVE_PATH = "savegame.txt";
    constexpr const char* PREGAME_CONFIG_PATH = "pregame_tmp.txt";
    constexpr const char* BG_IMAGE_PATH = "assets/img/menu_bg.jpg";

    void centerOrigin(sf::Text& t)
    {
        sf::FloatRect b = t.getLocalBounds();
        float cx = b.position.x + b.size.x * 0.5f;
        float cy = b.position.y + b.size.y * 0.5f;
        t.setOrigin(sf::Vector2f{cx, cy});
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
    , btnFinishGame(font, "Finish and Score", 24U) // Nút Finish Game (kết thúc ván & chốt điểm)
    , btnUndo(font, "Undo", 24U)
    , btnRedo(font, "Redo", 24U)

    , boardRect()
    , boardOrigin(0.f, 0.f)
    , boardPixelSize(0.f)
    , cellSize(0.f)
      
    
   
    , layoutDone(false)
    , statusTimer(0.f)
    
    
{
    if (!font.openFromFile(FONT_UI_PATH))
    {
        std::cerr << "[GameScreen] Failed to load font: "
                  << FONT_UI_PATH << "\n";
    }

    // Background
    if (!bgTexture.loadFromFile(BG_IMAGE_PATH))
    {
        std::cerr << "[GameScreen] Failed to load background: "
                  << BG_IMAGE_PATH << "\n";
    }
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

    // Bảng thông báo lượt đi
    turnPanelRect.setFillColor(sf::Color(30, 30, 30, 220));
    turnPanelRect.setOutlineColor(sf::Color::White);
    turnPanelRect.setOutlineThickness(2.f);

    turnPanelText.setFillColor(sf::Color::White);
    // Nút Mark Dead
    btnFinishGame.setOnClick([this]()
    {
        // Chỉ cho mark khi game đã kết thúc
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

    // Tính ai thắng, thắng bao nhiêu điểm
    
    double diff = s.blackTotal - s.whiteTotal;
    if (diff > 0.0)
        text += "\nBlack wins by " + toStr1(diff) + " points.";
    else if (diff < 0.0)
        text += "\nWhite wins by " + toStr1(-diff) + " points.";
    else
        text += "\nResult: Draw.";

    statusText.setString(text);
    statusTimer = 0.f;  // không auto xoá

    turnPanelText.setString("Game over");
    centerOrigin(turnPanelText);

    });
    // Nút Pass
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
        // vừa chuyển sang MarkDead mode
        updateScorePreview();           // hiển thị điểm ngay
        // panel text cho đẹp
        turnPanelText.setString("Mark dead stones");
        centerOrigin(turnPanelText);
        return; // không countdown timer
    }

        statusText.setString(res.message);

        if (game.isGameOver())
        {
            // ván đã kết thúc → giữ message, không tự xoá
            statusTimer = 0.f;
        }
        else
        {
            statusTimer = 2.0f;
        }

        updateTurnText();
        updateTurnPanel();
        updateScoreTexts();
    });


    // Nút Save
    btnSave.setOnClick([this]()
    {
        if (game.saveToFile(SAVE_PATH))
        {
            statusText.setString("Game saved!");
            statusTimer = 2.0f; // hiện 2 giây
        }
        else
        {
            statusText.setString("Save failed!");
            statusTimer = 2.0f;
        }
    });

    // Nút Back về Menu
    btnBackMenu.setOnClick([this]()
    {
        if (navigate)
            navigate("Menu");
    }); 


    btnUndo.setOnClick([this]()
    {
        // Nếu AI đang nghĩ thì đừng cho undo
        if (pendingAIMove) return;

        bool changed = false;

        if (vsAI)
        {
            // Undo 2 nước: AI + người
            if (game.canUndo()) { game.undo(); changed = true; }
            if (game.canUndo()) { game.undo(); changed = true; }
        }
        else
        {
            if (game.canUndo())
            {
                game.undo();
                changed = true;
            }
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

    // REDO 
    btnRedo.setOnClick([this]()
    {
        if (pendingAIMove) return;

        bool changed = false;

        if (vsAI)
        {
            // Redo 2 nước: người + AI
            if (game.canRedo()) { game.redo(); changed = true; }
            if (game.canRedo()) { game.redo(); changed = true; }
        }
        else
        {
            if (game.canRedo())
            {
                game.redo();
                changed = true;
            }
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

    
    

    // turn text & panel ban đầu
    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();

    
}

// đảm bảo boardCells đúng kích thước boardSize x boardSize


void GameScreen::setBoardSize(int size)
{
    if (size == 9 || size == 13 || size == 19)
    {
        game.reset(size); 
        
        layoutDone      = false; // buộc layout lại để vẽ board mới

       
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
    // cập nhật nội dung
    turnPanelText.setString(game.getCurrentPlayer() == 0 ? "Black Go!" : "White Go!");

    centerOrigin(turnPanelText);
}

void GameScreen::updateScoreTexts()
{
    blackScoreText.setString("Black captured: " + std::to_string(game.getBlackCaptured()));
    whiteScoreText.setString("White captured: " + std::to_string(game.getWhiteCaptured()));
}



void GameScreen::layout(const sf::Vector2u& winSize)
{
    const float winW = static_cast<float>(winSize.x);
    const float winH = static_cast<float>(winSize.y);

    if (bgSprite)
    {
        auto tex = bgTexture.getSize();
        float sx = winW / static_cast<float>(tex.x);
        float sy = winH / static_cast<float>(tex.y);
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

                // 0 = 2 Players, 1..3 = AI
                vsAI         = (mode != 0);
                aiPlayerIndex = 1; // AI chơi trắng

                if (vsAI)
                {
                    if (mode == 1)      ai.setDifficulty(AIDifficulty::Easy);
                    else if (mode == 2) ai.setDifficulty(AIDifficulty::Medium);
                    else                ai.setDifficulty(AIDifficulty::Hard);
                }

                statusText.setString("");
                statusTimer = 0.f;
            }

            cfg.close();
            std::remove(PREGAME_CONFIG_PATH); // xoá file tạm sau khi dùng
        }

        if (!loadedFromPreGame)
    {
        if (!game.loadFromFile(SAVE_PATH))
        {
            std::cout << "[GameScreen] No valid save, using default boardSize="
                      << game.getBoardSize() << "\n";
        }
        else
        {
            std::cout << "[GameScreen] Loaded save from " << SAVE_PATH << "\n";
        }

        // Continue game: treat as 2 players (hoặc sau này lưu thêm info vsAI riêng)
        vsAI = false;
    }



    

    
    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();

    

    std::string boardStr =
        "Board size: " + std::to_string(game.getBoardSize()) + " x " + std::to_string(game.getBoardSize());
    boardSizeText.setString(boardStr);
    boardSizeText.setPosition(sf::Vector2f{40.f, winH * 0.20f});

    boardSizeText.setFillColor(sf::Color::White);

    turnText.setPosition(sf::Vector2f{40.f, winH * 0.26f});

    // Score texts: chỉ set vị trí, nội dung đã trong updateScoreTexts()
    blackScoreText.setPosition(sf::Vector2f{40.f, winH * 0.32f});
    whiteScoreText.setPosition(sf::Vector2f{40.f, winH * 0.38f});

    // Status text (Saved! / Illegal move / Ko rule / end game)
    statusText.setPosition(sf::Vector2f{40.f, winH * 0.44f});

        // --- Buttons ---
    const float bottomMargin = 40.f;
    const float gapX         = 20.f;
    const float gapY         = 16.f;   // khoảng cách giữa 2 hàng nút

    // HÀNG DƯỚI: Pass, Save, Back to menu
    float currentXBottom = 40.f;
    float bottomRowY     = 0.f;   // sẽ gán sau khi biết chiều cao nút

    // Pass
    {
        const float paddingX = 40.f;
        const float paddingY = 16.f;

        float textW = btnPass.textWidth();
        float textH = font.getLineSpacing(24U);

        float w = textW + paddingX;
        float h = textH + paddingY;

        bottomRowY = winH - h - bottomMargin;

        btnPass.setSize(sf::Vector2f{w, h});
        btnPass.setPosition(sf::Vector2f{
            currentXBottom,
            bottomRowY
        });

        currentXBottom += w + gapX;
    }

    // Save
    {
        const float paddingX = 40.f;
        const float paddingY = 16.f;

        float textW = btnSave.textWidth();
        float textH = font.getLineSpacing(24U);

        float w = textW + paddingX;
        float h = textH + paddingY;

        btnSave.setSize(sf::Vector2f{w, h});
        btnSave.setPosition(sf::Vector2f{
            currentXBottom,
            bottomRowY
        });

        currentXBottom += w + gapX;
    }

    // Back to menu
    {
        const float paddingX = 48.f;
        const float paddingY = 18.f;

        float textW = btnBackMenu.textWidth();
        float textH = font.getLineSpacing(24U);

        float w = textW + paddingX;
        float h = textH + paddingY;

        btnBackMenu.setSize(sf::Vector2f{w, h});
        btnBackMenu.setPosition(sf::Vector2f{
            currentXBottom,
            bottomRowY
        });
    }

    // HÀNG GIỮA: nút Finish game 
    {
        const float paddingX = 40.f;
        const float paddingY = 16.f;

        float textFinishW = btnFinishGame.textWidth();
        float textH       = font.getLineSpacing(24U);

        float w = textFinishW + paddingX;
        float h = textH       + paddingY;

        // Hàng giữa: ngay trên hàng nút Pass/Save/Back
        float midRowY = bottomRowY - h - gapY;

        btnFinishGame.setSize(sf::Vector2f{w, h});
        btnFinishGame.setPosition(sf::Vector2f{40.f, midRowY});

        // HÀNG TRÊN CÙNG: Undo + Redo 
        // Dùng cùng kích thước w,h 
        float topRowY = midRowY - h - gapY;
        float x       = 40.f;

        btnUndo.setSize(sf::Vector2f{w, h});
        btnUndo.setPosition(sf::Vector2f{x, topRowY});

        x += w + gapX;

        btnRedo.setSize(sf::Vector2f{w, h});
        btnRedo.setPosition(sf::Vector2f{x, topRowY});
    }



    int nPoints = game.getBoardSize();// số giao điểm trên 1 cạnh (9, 13, 19)
    if (nPoints != 9 && nPoints != 13 && nPoints != 19)
        nPoints = 9;

    int segments = nPoints - 1;// số ô = (nPoints - 1)

    // chiếm tối đa ~80% chiều cao, ~60% chiều rộng bên phải
    float maxBoardSide = std::min(winH * 0.8f, winW * 0.6f);

    cellSize       = maxBoardSide / static_cast<float>(segments);
    boardPixelSize = cellSize * static_cast<float>(segments);

    float frameThickness = cellSize * 0.8f;

    boardRect.setSize(sf::Vector2f{
        boardPixelSize + 2.f * frameThickness,
        boardPixelSize + 2.f * frameThickness
    });
    boardRect.setFillColor(sf::Color(214, 177, 122)); // gỗ nhạt
    boardRect.setOutlineColor(sf::Color(160, 110, 52)); // viền gỗ đậm
    boardRect.setOutlineThickness(4.f);

    float boardX = winW - boardRect.getSize().x - 60.f;
    float boardY = (winH - boardRect.getSize().y) * 0.5f;
    boardRect.setPosition(sf::Vector2f{boardX, boardY});

    // origin của lưới ở trong khung
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
        float rightLimit  = boardRect.getPosition().x - 20.f; // không chạm bàn cờ

        float panelW = rightLimit - leftMargin;
        if (panelW < textW + padX * 2.f)
        {
            panelW = textW + padX * 2.f;
        }
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
    // Nếu đang ở MarkDead mode -> click để gỡ group
    if (game.isMarkingDead())
    {
        // chuyển screen coords 
        float mx = static_cast<float>(mouseX);
        float my = static_cast<float>(mouseY);

        float localX = mx - boardOrigin.x;
        float localY = my - boardOrigin.y;

        if (localX < 0.f || localY < 0.f)
            return;
        if (localX > boardPixelSize || localY > boardPixelSize)
            return;

        int n = game.getBoardSize();
        if (n != 9 && n != 13 && n != 19)
            n = 9;

        float fx = localX / cellSize;
        float fy = localY / cellSize;

        int j = static_cast<int>(std::round(fx));
        int i = static_cast<int>(std::round(fy));

        if (i < 0 || i >= n || j < 0 || j >= n)
            return;

        float gx = static_cast<float>(j) * cellSize;
        float gy = static_cast<float>(i) * cellSize;
        float dx = std::abs(localX - gx);
        float dy = std::abs(localY - gy);
        float maxDist = cellSize * 0.4f;
        if (dx > maxDist || dy > maxDist)
            return;
        
        // Gỡ cả group chết
        GoGame::MarkDeadResult res = game.markDeadGroup(i, j);
        if (!res.ok)
        {
            statusText.setString(res.message);
            statusTimer = 2.0f;
        }
        else
        {
            updateScorePreview(); // tính lại điểm theo Nhật luật với board mới
        }
        return; // không xử lý như nước đi bình thường nữa
           
        
    }

    // Ngược lại: chế độ chơi bình thường dùng code playMove
    
    if (game.isGameOver())
        return;

    // chuyển screen coords -> local trên bàn
    float mx = static_cast<float>(mouseX);
    float my = static_cast<float>(mouseY);

    float localX = mx - boardOrigin.x;
    float localY = my - boardOrigin.y;

    if (localX < 0.f || localY < 0.f)
        return;
    if (localX > boardPixelSize || localY > boardPixelSize)
        return;

    int n = game.getBoardSize();
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    // vị trí tương đối theo cellSize
    float fx = localX / cellSize;
    float fy = localY / cellSize;

    int j = static_cast<int>(std::round(fx));
    int i = static_cast<int>(std::round(fy));

    if (i < 0 || i >= n || j < 0 || j >= n) 
        return;

    // chỉ cho click gần giao điểm (không ở quá giữa ô)
    float gx = static_cast<float>(j) * cellSize;
    float gy = static_cast<float>(i) * cellSize;
    float dx = std::abs(localX - gx);
    float dy = std::abs(localY - gy);
    float maxDist = cellSize * 0.4f;
    if (dx > maxDist || dy > maxDist)
        return;

    GoGame::MoveResult res = game.playMove(i, j);
    if (!res.ok)
    {
        statusText.setString(res.message);
        statusTimer = 2.0f;
        return;
    }

    // 3. Nước đi hợp lệ → cập nhật HUD
    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();
    statusText.setString(""); // hoặc hiện "OK" nếu bạn thích
    statusTimer = 0.f;

        // Sau khi human đi xong:
    // nếu đang chơi với AI và chưa vào mark-dead / game over
    


    // Trang thai AI dang suy nghi
    if (vsAI && !game.isGameOver() && !game.isMarkingDead())
    {
        int curPlayer = game.getCurrentPlayer(); // 0 = Black, 1 = White
        if (curPlayer == aiPlayerIndex)          // đến lượt AI
        {
            pendingAIMove = true;
            aiThinkTimer  = 1.0f;               // delay cho “suy nghĩ”

            // Thông báo cho người chơi biết bot đang nghĩ
            statusText.setString("AI is thinking...");
            statusTimer = 0.f; // không tự xoá trong lúc đang nghĩ
        }
    }


}

void GameScreen::handleEvent(const sf::Event& e)
{
    if (!layoutDone)
        return;

    // Nếu đang chơi với AI và AI đang trong trạng thái "đang suy nghĩ"
    // thì bỏ qua toàn bộ input (không xử lý nút, không click bàn)
    if (vsAI && pendingAIMove)
        return;
    
    btnUndo.handleEvent(e);
    btnRedo.handleEvent(e);
    btnPass.handleEvent(e);
    btnSave.handleEvent(e);
    btnBackMenu.handleEvent(e);
    btnFinishGame.handleEvent(e);
    // xử lý click đặt quân
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
    // 1) Cập nhật statusText (trừ khi game over / mark-dead)
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

    // 2) Xử lý AI move nếu đang chờ
if (pendingAIMove && !game.isGameOver() && !game.isMarkingDead())
{
    aiThinkTimer -= dt;
    if (aiThinkTimer <= 0.f)
    {
        int curPlayer = game.getCurrentPlayer();
        if (vsAI && curPlayer == aiPlayerIndex)
        {
            int aiColor = (aiPlayerIndex == 0 ? GoGame::Black
                                              : GoGame::White);

            auto move = ai.chooseMove(game, aiColor);
            if (move.first >= 0 && move.second >= 0)
            {
                GoGame::MoveResult aiRes = game.playMove(move.first, move.second);
                if (!aiRes.ok)
                {
                    std::cout << "[AI] Illegal move: " << aiRes.message << "\n";
                }
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

        // Xoá “AI is thinking...” sau khi đã đánh xong
        statusText.setString("");
        statusTimer = 0.f;
    }
}
}

void GameScreen::updateScorePreview()
{
    GoGame::JapaneseScore s = game.computeJapaneseScoreWithDead();

    auto toStr1 = [](double x) {
        // in 1 số thập phân cho gọn
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
    statusTimer = 0.f;   // không auto xoá trong mode này
}

void GameScreen::draw(sf::RenderWindow& window)
{
    if (!layoutDone)
        layout(window.getSize());

    // Background
    if (bgSprite)
        window.draw(*bgSprite);

    // HUD (bên trái) — KHÔNG vẽ titleText nữa
    window.draw(boardSizeText);
    window.draw(turnText);
    window.draw(blackScoreText);
    window.draw(whiteScoreText);

    if (!statusText.getString().isEmpty())
        window.draw(statusText);

    // Bảng "Black Go! / White Go!"
    window.draw(turnPanelRect);
    window.draw(turnPanelText);

    // --- Vẽ bàn cờ ---
    window.draw(boardRect);

    int nPoints = game.getBoardSize();
    if (nPoints != 9 && nPoints != 13 && nPoints != 19)
        nPoints = 9;

    sf::Color gridColor = sf::Color::Black;

    // Số đường lưới = nPoints (VD: 9 điểm -> 8 ô)
    for (int i = 0; i < nPoints; ++i)
    {
        float offset = static_cast<float>(i) * cellSize;

        // Vertical line
        float x = boardOrigin.x + offset;
        sf::RectangleShape vLine;
        vLine.setSize(sf::Vector2f{1.f, boardPixelSize});
        vLine.setFillColor(gridColor);
        vLine.setPosition(sf::Vector2f{x, boardOrigin.y});
        window.draw(vLine);

        // Horizontal line
        float y = boardOrigin.y + offset;
        sf::RectangleShape hLine;
        hLine.setSize(sf::Vector2f{boardPixelSize, 1.f});
        hLine.setFillColor(gridColor);
        hLine.setPosition(sf::Vector2f{boardOrigin.x, y});
        window.draw(hLine);
    }

    //Vẽ quân cờ
    
    int n = game.getBoardSize();
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    float stoneRadius = cellSize * 0.35f;

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            int v = game.getCell(i, j);
            if (v == GoGame::Empty)
                continue;

            float cx = boardOrigin.x + static_cast<float>(j) * cellSize;
            float cy = boardOrigin.y + static_cast<float>(i) * cellSize;

            sf::CircleShape stone(stoneRadius);
            stone.setOrigin(sf::Vector2f{stoneRadius, stoneRadius});
            stone.setPosition(sf::Vector2f{cx, cy});

            if (v == GoGame::Black)
            {
                stone.setFillColor(sf::Color::Black);
                stone.setOutlineColor(sf::Color(80, 80, 80));
            }
            else // 2 = white
            {
                stone.setFillColor(sf::Color::White);
                stone.setOutlineColor(sf::Color(160, 160, 160));
            }
            stone.setOutlineThickness(2.f);

            window.draw(stone);
        }
    }

    // Buttons
    btnPass.draw(window);
    btnSave.draw(window);
    btnBackMenu.draw(window);
    btnFinishGame.draw(window);
    btnRedo.draw(window);
    btnUndo.draw(window);
}
