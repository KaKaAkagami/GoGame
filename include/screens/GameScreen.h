#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Screen.h"
#include "widgets/Button.h"

class GameScreen : public Screen
{
public:
    using NavigateFn = std::function<void(const std::string&)>;

    explicit GameScreen(NavigateFn onNavigate);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void layout(const sf::Vector2u& winSize);

    // Được App gọi trước khi switch sang "Game"
    void setBoardSize(int size);       // 9, 13, 19
    void setCurrentPlayer(int player); // 0 = Black, 1 = White

private:
    struct GameState
    {
        int boardSize;       // 9, 13, 19 (default = 9)
        int currentPlayer;   // 0 = Black, 1 = White
        int blackCaptured;
        int whiteCaptured;
    };

    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path);

    // ====== Helpers cho board & logic ======
    void ensureBoardArray();                    // đảm bảo boardCells có kích thước boardSize*boardSize
    void handleBoardClick(int mouseX, int mouseY);
    void updateTurnText();
    void updateTurnPanel();
    void updateScoreTexts();

    // BƯỚC 1: tìm group & đếm liberties
    // row, col: toạ độ quân (0..n-1), color: 1 = đen, 2 = trắng
    // outGroup: danh sách index (i*n + j) của group
    // outLiberties: số liberties của cả group (ô trống kề 4 hướng, không trùng)
    void getGroupAndLiberties(
        int row, int col, int color,
        std::vector<int>& outGroup,
        int& outLiberties
    ) const;

private:
    NavigateFn navigate;

    sf::Font font;

    sf::Texture                 bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    // HUD texts
    sf::Text titleText;       // đang để trống, không vẽ
    sf::Text boardSizeText;
    sf::Text turnText;
    sf::Text blackScoreText;
    sf::Text whiteScoreText;
    sf::Text statusText;   // "Saved!" / "Illegal move" / "Ko rule" / end game

    // Bảng "Black Go! / White Go!"
    sf::RectangleShape turnPanelRect;
    sf::Text           turnPanelText;

    // Buttons
    Button btnPass;       // Pass
    Button btnSave;       // Save game
    Button btnBackMenu;   // quay về Menu

    // --- Board rendering ---
    sf::RectangleShape boardRect;     // nền + khung bàn cờ
    sf::Vector2f       boardOrigin;   // góc trên trái vùng lưới (tọa độ line đầu tiên)
    float              boardPixelSize; // kích thước vùng lưới (vuông) = (N-1)*cellSize
    float              cellSize;       // khoảng cách giữa 2 đường lưới

    GameState          state;

    // 0 = empty, 1 = black, 2 = white
    std::vector<int>   boardCells;

    // ===== Hỗ trợ luật Ko (simple ko) =====
    std::vector<int>   prevBoardCells;
    bool               hasPrevBoard;

    bool  layoutDone;
    float statusTimer;     // để hiển thị status tạm thời (Saved!, Illegal move,...)

    // ===== Pass & end game =====
    int  consecutivePasses;  // số lượt pass liên tiếp
    bool gameOver;           // true khi 2 bên đã pass liên tiếp
};
