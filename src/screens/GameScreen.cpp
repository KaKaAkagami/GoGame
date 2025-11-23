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
    constexpr const char* FONT_UI_PATH        = "assets/fonts/Inter_28pt-SemiBold.ttf";
    constexpr const char* SAVE_PATH           = "savegame.txt";
    constexpr const char* PREGAME_CONFIG_PATH = "pregame_tmp.txt";
    constexpr const char* BG_IMAGE_PATH       = "assets/img/menu_bg.jpg";

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
    , turnPanelText(font, "", 28U)   // CHỮ PANEL TO HƠN
    , btnPass(font, "Pass", 24U)
    , btnSave(font, "Save game", 24U)
    , btnBackMenu(font, "Back to menu", 24U)
    , boardRect()
    , boardOrigin(0.f, 0.f)
    , boardPixelSize(0.f)
    , cellSize(0.f)
    , state{9, 0, 0, 0}   // default: 9x9, Black to move, scores 0
    , boardCells()
    , prevBoardCells()
    , hasPrevBoard(false)
    , layoutDone(false)
    , statusTimer(0.f)
    , consecutivePasses(0)
    , gameOver(false)
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

    // HUD text style
    titleText.setString("");
    titleText.setFillColor(sf::Color::White);  // không quan trọng vì không vẽ

    boardSizeText.setFillColor(sf::Color::White);
    turnText.setFillColor(sf::Color::White);
    blackScoreText.setFillColor(sf::Color::White);
    whiteScoreText.setFillColor(sf::Color::White);

    statusText.setFillColor(sf::Color(0, 255, 0)); // xanh lá sáng

    // Bảng thông báo lượt đi
    turnPanelRect.setFillColor(sf::Color(30, 30, 30, 220));
    turnPanelRect.setOutlineColor(sf::Color::White);
    turnPanelRect.setOutlineThickness(2.f);

    turnPanelText.setFillColor(sf::Color::White);

    // Nút Pass
    btnPass.setOnClick([this]()
    {
        if (gameOver)
            return;

        std::string who = (state.currentPlayer == 0 ? "Black" : "White");
        ++consecutivePasses;

        if (consecutivePasses >= 2)
        {
            // Kết thúc ván: tính winner theo số quân bắt được
            gameOver   = true;
            statusTimer = 0.f; // giữ thông báo, không auto xoá

            int diff = state.blackCaptured - state.whiteCaptured;
            if (diff > 0)
            {
                statusText.setString(
                    "Both players passed\nBlack wins by "
                    + std::to_string(diff)
                    + " capture" + (diff > 1 ? "s" : "")
                );
            }
            else if (diff < 0)
            {
                diff = -diff;
                statusText.setString(
                    "Both players passed\nWhite wins by "
                    + std::to_string(diff)
                    + " capture" + (diff > 1 ? "s" : "")
                );
            }
            else
            {
                statusText.setString("Both players passed\nNo winner (tie)");
            }
        }
        else
        {
            // Pass 1 lần: đổi lượt, hiển thị thông báo tạm
            statusText.setString(who + " passed");
            statusTimer = 2.0f;

            state.currentPlayer = 1 - state.currentPlayer;
            updateTurnText();
            updateTurnPanel();
        }
    });

    // Nút Save
    btnSave.setOnClick([this]()
    {
        if (saveToFile(SAVE_PATH))
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

    // mảng quân ban đầu rỗng 9x9
    ensureBoardArray();

    // turn text & panel ban đầu
    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();

    // Ko: khởi tạo prev = bàn hiện tại (toàn 0)
    prevBoardCells = boardCells;
    hasPrevBoard   = true;
}

// đảm bảo boardCells đúng kích thước boardSize x boardSize
void GameScreen::ensureBoardArray()
{
    int n = state.boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    std::size_t expected = static_cast<std::size_t>(n * n);
    if (boardCells.size() != expected)
        boardCells.assign(expected, 0);
}

void GameScreen::setBoardSize(int size)
{
    if (size == 9 || size == 13 || size == 19)
    {
        state.boardSize = size;
        ensureBoardArray();
        layoutDone      = false; // buộc layout lại để vẽ board mới

        // reset Ko history & pass cho board mới
        prevBoardCells     = boardCells;
        hasPrevBoard       = true;
        consecutivePasses  = 0;
        gameOver           = false;
        statusText.setString("");
        statusTimer        = 0.f;
    }
}

void GameScreen::setCurrentPlayer(int player)
{
    if (player == 0 || player == 1)
    {
        state.currentPlayer = player;
        updateTurnText();
        updateTurnPanel();
    }
}

bool GameScreen::loadFromFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
        return false;

    int bSize = 0;
    int cur   = 0;
    int bCap  = 0;
    int wCap  = 0;

    in >> bSize >> cur >> bCap >> wCap;
    if (!in)
        return false;

    // validate đơn giản
    if (bSize != 9 && bSize != 13 && bSize != 19)
        return false;
    if (cur < 0 || cur > 1)
        return false;

    state.boardSize     = bSize;
    state.currentPlayer = cur;
    state.blackCaptured = bCap;
    state.whiteCaptured = wCap;

    ensureBoardArray();

    // đọc mảng quân (n*n số)
    int n = state.boardSize;
    for (int i = 0; i < n * n; ++i)
    {
        if (!(in >> boardCells[static_cast<std::size_t>(i)]))
            return false;
        int v = boardCells[static_cast<std::size_t>(i)];
        if (v < 0 || v > 2) // 0,1,2
            boardCells[static_cast<std::size_t>(i)] = 0;
    }

    layoutDone = false;
    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();

    // Ko: sau khi load, dùng bàn hiện tại làm trạng thái trước
    prevBoardCells    = boardCells;
    hasPrevBoard      = true;
    consecutivePasses = 0;
    gameOver          = false;
    statusText.setString("");
    statusTimer       = 0.f;

    return true;
}

bool GameScreen::saveToFile(const std::string& path)
{
    std::ofstream out(path);
    if (!out)
        return false;

    // dòng 1: boardSize currentPlayer blackCaptured whiteCaptured
    out << state.boardSize << " "
        << state.currentPlayer << " "
        << state.blackCaptured << " "
        << state.whiteCaptured << "\n";

    // dòng 2: toàn bộ mảng quân
    int n = state.boardSize;
    ensureBoardArray();

    for (int i = 0; i < n * n; ++i)
    {
        out << boardCells[static_cast<std::size_t>(i)];
        if (i + 1 < n * n)
            out << ' ';
    }
    out << "\n";

    return static_cast<bool>(out);
}

void GameScreen::updateTurnText()
{
    std::string turnStr = "Turn: " + std::string(state.currentPlayer == 0 ? "Black" : "White");
    turnText.setString(turnStr);
}

void GameScreen::updateTurnPanel()
{
    // cập nhật nội dung
    turnPanelText.setString(state.currentPlayer == 0 ? "Black Go!" : "White Go!");

    // sẽ được đặt position trong layout(), ở đây chỉ reset origin cho đẹp
    centerOrigin(turnPanelText);
}

void GameScreen::updateScoreTexts()
{
    blackScoreText.setString("Black captured: " + std::to_string(state.blackCaptured));
    whiteScoreText.setString("White captured: " + std::to_string(state.whiteCaptured));
}

// ===== BƯỚC 1: TÌM GROUP & LIBERTIES BẰNG BFS =====
void GameScreen::getGroupAndLiberties(
    int row, int col, int color,
    std::vector<int>& outGroup,
    int& outLiberties
) const
{
    outGroup.clear();
    outLiberties = 0;

    int n = state.boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    if (row < 0 || row >= n || col < 0 || col >= n)
        return;

    int startIdx = row * n + col;
    if (startIdx < 0 || startIdx >= n * n)
        return;

    if (color != 1 && color != 2)
        return;

    if (boardCells.empty() ||
        boardCells[static_cast<std::size_t>(startIdx)] != color)
    {
        return; // ô này không phải quân cần kiểm tra
    }

    std::vector<bool> visited(static_cast<std::size_t>(n * n), false);
    std::vector<bool> libertyMarked(static_cast<std::size_t>(n * n), false);

    std::queue<int> q;
    visited[static_cast<std::size_t>(startIdx)] = true;
    q.push(startIdx);

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    while (!q.empty())
    {
        int idx = q.front();
        q.pop();

        outGroup.push_back(idx);

        int r = idx / n;
        int c = idx % n;

        for (int k = 0; k < 4; ++k)
        {
            int nr = r + dr[k];
            int nc = c + dc[k];
            if (nr < 0 || nr >= n || nc < 0 || nc >= n)
                continue;

            int nIdx = nr * n + nc;
            int v    = boardCells[static_cast<std::size_t>(nIdx)];

            if (v == 0)
            {
                // đây là 1 liberty, nhưng tránh đếm trùng
                if (!libertyMarked[static_cast<std::size_t>(nIdx)])
                {
                    libertyMarked[static_cast<std::size_t>(nIdx)] = true;
                    ++outLiberties;
                }
            }
            else if (v == color && !visited[static_cast<std::size_t>(nIdx)])
            {
                visited[static_cast<std::size_t>(nIdx)] = true;
                q.push(nIdx);
            }
        }
    }
}

void GameScreen::layout(const sf::Vector2u& winSize)
{
    const float winW = static_cast<float>(winSize.x);
    const float winH = static_cast<float>(winSize.y);

    // ===== Background full màn =====
    if (bgSprite)
    {
        auto tex = bgTexture.getSize();
        float sx = winW / static_cast<float>(tex.x);
        float sy = winH / static_cast<float>(tex.y);
        bgSprite->setScale(sf::Vector2f{sx, sy});
        bgSprite->setPosition(sf::Vector2f{0.f, 0.f});
    }

    // ====== 1. Ưu tiên cấu hình từ PreGame (Play -> chọn board -> Start) ======
    bool loadedFromPreGame = false;
    {
        std::ifstream cfg(PREGAME_CONFIG_PATH);
        if (cfg)
        {
            int bSize = 0;
            int mode  = 0;
            cfg >> bSize >> mode;

            if (cfg && (bSize == 9 || bSize == 13 || bSize == 19))
            {
                state.boardSize     = bSize;
                state.currentPlayer = 0;
                state.blackCaptured = 0;
                state.whiteCaptured = 0;

                // 🔥 RESET HOÀN TOÀN BÀN CỜ KHI START TỪ PREGAME
                boardCells.assign(
                    static_cast<std::size_t>(state.boardSize * state.boardSize),
                    0
                );

                loadedFromPreGame = true;

                std::cout << "[GameScreen] Loaded boardSize from PreGame: "
                          << bSize << "x" << bSize << " (mode=" << mode << ")\n";

                // reset Ko + pass cho ván mới
                prevBoardCells     = boardCells;
                hasPrevBoard       = true;
                consecutivePasses  = 0;
                gameOver           = false;
                statusText.setString("");
                statusTimer        = 0.f;
            }

            cfg.close();
            std::remove(PREGAME_CONFIG_PATH); // xoá file tạm sau khi dùng
        }
    }

    // ====== 2. Nếu KHÔNG vào từ PreGame, thử load savegame (Continue game) ======
    if (!loadedFromPreGame)
    {
        if (loadFromFile(SAVE_PATH))
        {
            std::cout << "[GameScreen] Loaded save from " << SAVE_PATH << "\n";
        }
        else
        {
            std::cout << "[GameScreen] No valid save, keep boardSize="
                      << state.boardSize << "\n";
        }
    }

    ensureBoardArray();
    updateTurnText();
    updateTurnPanel();
    updateScoreTexts();

    // đảm bảo prevBoardCells đồng bộ với board sau layout
    prevBoardCells = boardCells;
    hasPrevBoard   = true;

    // --- Board size & scores ---
    std::string boardStr =
        "Board size: " + std::to_string(state.boardSize) + " x " + std::to_string(state.boardSize);
    boardSizeText.setString(boardStr);
    boardSizeText.setPosition(sf::Vector2f{40.f, winH * 0.20f});

    boardSizeText.setFillColor(sf::Color::White);

    turnText.setPosition(sf::Vector2f{40.f, winH * 0.26f});

    // Score texts: chỉ set vị trí, nội dung đã trong updateScoreTexts()
    blackScoreText.setPosition(sf::Vector2f{40.f, winH * 0.32f});
    whiteScoreText.setPosition(sf::Vector2f{40.f, winH * 0.38f});

    // Status text (Saved! / Illegal move / Ko rule / end game)
    statusText.setPosition(sf::Vector2f{40.f, winH * 0.44f});

    // --- Buttons (Pass, Save, Back) ở dưới bên trái ---
    const float bottomMargin = 40.f;
    const float gapX         = 20.f;

    float currentX = 40.f;

    // Pass
    {
        const float paddingX = 40.f;
        const float paddingY = 16.f;

        float textW = btnPass.textWidth();
        float textH = font.getLineSpacing(24U);

        float w = textW + paddingX;
        float h = textH + paddingY;

        btnPass.setSize(sf::Vector2f{w, h});
        btnPass.setPosition(sf::Vector2f{
            currentX,
            winH - h - bottomMargin
        });

        currentX += w + gapX;
    }

    // Save
    float saveBtnH = 0.f;
    {
        const float paddingX = 40.f;
        const float paddingY = 16.f;

        float textW = btnSave.textWidth();
        float textH = font.getLineSpacing(24U);

        float w = textW + paddingX;
        float h = textH + paddingY;
        saveBtnH = h;

        btnSave.setSize(sf::Vector2f{w, h});
        btnSave.setPosition(sf::Vector2f{
            currentX,
            winH - h - bottomMargin
        });

        currentX += w + gapX;
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
            currentX,
            winH - h - bottomMargin
        });
    }

    // --- Board geometry (phụ thuộc boardSize) ---
    int nPoints = state.boardSize;             // số giao điểm trên 1 cạnh (9, 13, 19)
    if (nPoints != 9 && nPoints != 13 && nPoints != 19)
        nPoints = 9;

    int segments = nPoints - 1;                // số ô = (nPoints - 1)

    // chiếm tối đa ~80% chiều cao, ~60% chiều rộng bên phải
    float maxBoardSide = std::min(winH * 0.8f, winW * 0.6f);

    cellSize       = maxBoardSide / static_cast<float>(segments);
    boardPixelSize = cellSize * static_cast<float>(segments);

    float frameThickness = cellSize * 0.8f;

    boardRect.setSize(sf::Vector2f{
        boardPixelSize + 2.f * frameThickness,
        boardPixelSize + 2.f * frameThickness
    });
    boardRect.setFillColor(sf::Color(214, 177, 122));       // gỗ nhạt
    boardRect.setOutlineColor(sf::Color(160, 110, 52));     // viền gỗ đậm
    boardRect.setOutlineThickness(4.f);

    float boardX = winW - boardRect.getSize().x - 60.f;
    float boardY = (winH - boardRect.getSize().y) * 0.5f;
    boardRect.setPosition(sf::Vector2f{boardX, boardY});

    // origin của lưới ở trong khung
    boardOrigin = sf::Vector2f{
        boardX + frameThickness,
        boardY + frameThickness
    };

    // --- Turn panel: nằm bên trái, phía trên, trong vùng không có bàn cờ ---
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
    if (gameOver)
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

    int n = state.boardSize;
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

    ensureBoardArray();
    std::size_t idx = static_cast<std::size_t>(i * n + j);

    if (boardCells[idx] != 0)
        return; // ô đã có quân

    // LƯU BÀN CŨ (để dùng cho Ko & revert nếu cần)
    std::vector<int> oldBoard = boardCells;

    // đặt quân tạm thời
    int color    = (state.currentPlayer == 0 ? 1 : 2); // 1 = đen, 2 = trắng
    int opponent = (color == 1 ? 2 : 1);
    boardCells[idx] = color;

    // ===== BƯỚC 2: BẮT QUÂN LÂN CẬN (NẾU CÓ) =====
    int capturedThisMove = 0;
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    for (int k = 0; k < 4; ++k)
    {
        int nr = i + dr[k];
        int nc = j + dc[k];
        if (nr < 0 || nr >= n || nc < 0 || nc >= n)
            continue;

        std::size_t nIdx = static_cast<std::size_t>(nr * n + nc);
        if (boardCells[nIdx] != opponent)
            continue;

        std::vector<int> oppGroup;
        int oppLibs = 0;
        getGroupAndLiberties(nr, nc, opponent, oppGroup, oppLibs);

        if (oppLibs == 0)
        {
            // bắt toàn bộ group đối thủ
            for (int gIdx : oppGroup)
            {
                if (gIdx >= 0 && gIdx < n * n)
                    boardCells[static_cast<std::size_t>(gIdx)] = 0;
            }
            capturedThisMove += static_cast<int>(oppGroup.size());
        }
    }

    // ===== CẤM TỰ SÁT: nếu không bắt quân nào và group của mình liberties = 0 => move illegal =====
    {
        std::vector<int> myGroup;
        int myLibs = 0;
        getGroupAndLiberties(i, j, color, myGroup, myLibs);

        if (myLibs == 0 && capturedThisMove == 0)
        {
            // tự sát -> revert quân vừa đặt
            boardCells = oldBoard;
            statusText.setString("Illegal move");
            statusTimer = 2.0f;
            return;
        }

        // debug log
        std::cout << "[GameScreen] Placed "
                  << (color == 1 ? "Black" : "White")
                  << " at (" << i << "," << j << "), group size="
                  << myGroup.size() << ", liberties=" << myLibs
                  << ", captured=" << capturedThisMove << "\n";
    }

    // ===== LUẬT KO: cấm tái tạo lại bàn trước lượt đi trước đó =====
    if (hasPrevBoard && prevBoardCells.size() == boardCells.size())
    {
        bool same = std::equal(boardCells.begin(), boardCells.end(),
                               prevBoardCells.begin());
        if (same)
        {
            // move vi phạm Ko → revert toàn bộ bàn và không đổi lượt
            boardCells = oldBoard;
            statusText.setString("Ko rule");
            statusTimer = 2.0f;
            return;
        }
    }

    // ===== CẬP NHẬT ĐIỂM BẮT QUÂN =====
    if (capturedThisMove > 0)
    {
        if (color == 1)
            state.blackCaptured += capturedThisMove;
        else
            state.whiteCaptured += capturedThisMove;

        updateScoreTexts();
    }

    // ===== LƯU BÀN TRƯỚC NƯỚC ĐI (cho lần check Ko tiếp theo) =====
    prevBoardCells   = oldBoard;
    hasPrevBoard     = true;
    consecutivePasses = 0; // có nước đi mới => reset chuỗi pass

    // đổi lượt
    state.currentPlayer = 1 - state.currentPlayer;
    updateTurnText();
    updateTurnPanel();
}

void GameScreen::handleEvent(const sf::Event& e)
{
    if (!layoutDone)
        return;

    btnPass.handleEvent(e);
    btnSave.handleEvent(e);
    btnBackMenu.handleEvent(e);

    // xử lý click đặt quân
    if (auto mouse = e.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button == sf::Mouse::Button::Left)
        {
            handleBoardClick(mouse->position.x, mouse->position.y);
        }
    }
}

void GameScreen::update(float dt)
{
    // Khi gameOver: giữ nguyên statusText, không tự xoá
    if (gameOver)
        return;

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

    int nPoints = state.boardSize;
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

    // --- Vẽ quân cờ ---
    ensureBoardArray();
    int n = state.boardSize;
    if (n != 9 && n != 13 && n != 19)
        n = 9;

    float stoneRadius = cellSize * 0.35f;

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            int v = boardCells[static_cast<std::size_t>(i * n + j)];
            if (v == 0)
                continue;

            float cx = boardOrigin.x + static_cast<float>(j) * cellSize;
            float cy = boardOrigin.y + static_cast<float>(i) * cellSize;

            sf::CircleShape stone(stoneRadius);
            stone.setOrigin(sf::Vector2f{stoneRadius, stoneRadius});
            stone.setPosition(sf::Vector2f{cx, cy});

            if (v == 1)
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
}
