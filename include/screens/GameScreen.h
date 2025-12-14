#pragma once
#include "Screen.h"

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <utility>

#include "widgets/Button.h"
#include "GameLogic.h"
#include "AI.h"

class GameScreen : public Screen
{
public:
    using NavigateFn = std::function<void(const std::string&)>;

    explicit GameScreen(NavigateFn onNavigate);

    void setBoardSize(int size);
    void setCurrentPlayer(int player);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void layout(const sf::Vector2u& winSize);

    void updateTurnText();
    void updateTurnPanel();
    void updateScoreTexts();

    void handleBoardClick(int mouseX, int mouseY);

    
    void loadBoardTheme();
    void applyBoardThemeToRect();

    
    void loadStoneTheme();
    void applyStoneTheme(); 

    
    void updateScorePreview();

private:
    NavigateFn navigate;

    GoGame game;

    GoAI ai;
    bool vsAI;
    int  aiPlayerIndex;
    bool pendingAIMove;
    float aiThinkTimer;

    sf::Font font;

    sf::Texture bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    sf::Text titleText;
    sf::Text boardSizeText;
    sf::Text turnText;
    sf::Text blackScoreText;
    sf::Text whiteScoreText;
    sf::Text statusText;

    sf::RectangleShape turnPanelRect;
    sf::Text           turnPanelText;

    Button btnPass;
    Button btnSave;
    Button btnBackMenu;
    Button btnFinishGame;
    Button btnUndo;
    Button btnRedo;

    sf::RectangleShape boardRect;

    
    std::string boardTheme;
    sf::Texture boardSkinTexture;
    bool boardSkinLoaded;

    
    std::string stoneTheme;                 
    sf::Texture stoneBlackTexture;
    sf::Texture stoneWhiteTexture;
    bool stoneSkinLoaded;

    
    sf::Vector2f boardOrigin;
    float boardPixelSize;
    float cellSize;
    bool layoutDone;

    
    float statusTimer;
};
