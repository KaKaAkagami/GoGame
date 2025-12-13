#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Screen.h"
#include "widgets/Button.h"
#include "GameLogic.h"
#include "AI.h"

class GameScreen : public Screen
{
public:
    using NavigateFn = std::function<void(const std::string&)>;

    explicit GameScreen(NavigateFn onNavigate);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void layout(const sf::Vector2u& winSize);

    
    void setBoardSize(int size);// 9, 13, 19
    void setCurrentPlayer(int player); // 0 = Black, 1 = White

private:
    

    
    void handleBoardClick(int mouseX, int mouseY);
    void updateTurnText();
    void updateTurnPanel();
    void updateScoreTexts();


   

private:
    NavigateFn navigate;

    sf::Font font;

    sf::Texture                 bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    sf::Text titleText;      
    sf::Text boardSizeText;
    sf::Text turnText;
    sf::Text blackScoreText;
    sf::Text whiteScoreText;
    sf::Text statusText;// "Saved!" / "Illegal move" / "Ko rule" / end game

    // Bảng "Black Go! / White Go!"
    sf::RectangleShape turnPanelRect;
    sf::Text           turnPanelText;

    // Buttons
    Button btnPass;// Pass
    Button btnSave;// Save game
    Button btnBackMenu;// quay về Menu

    Button btnFinishGame; //  nút Mark Dead
    Button btnUndo;
    Button btnRedo;
 
    sf::RectangleShape boardRect;// nền + khung bàn cờ
    sf::Vector2f boardOrigin;// góc trên trái vùng lưới (tọa độ line đầu tiên)
    float boardPixelSize;// kích thước vùng lưới (vuông) = (N-1)*cellSize
    float cellSize;// khoảng cách giữa 2 đường lưới

    

    bool  layoutDone;
    float statusTimer;// để hiển thị status tạm thời (Saved!, Illegal move,...)

    // Logic thuần cờ vây
    GoGame game;
    bool vsAI = false;
    int aiColor = 1; // giả sử 1 = White
    int       aiPlayerIndex;
    GoAI ai;
    bool pendingAIMove;      // true = vừa tới lượt AI, chờ AI đánh
    float aiThinkTimer;      // thời gian chờ trước khi AI đánh

    int selectedMode = 0; // 0 = 2P, 1..3 = AI
    void updateScorePreview();
};
