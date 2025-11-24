#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <array>

#include "Screen.h"
#include "widgets/Button.h"

class PreGameScreen : public Screen
{
public:
    using NavigateFn = std::function<void(const std::string&)>;

    explicit PreGameScreen(NavigateFn onNavigate);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void layout(const sf::Vector2u& winSize);

private:
    NavigateFn navigate;

    sf::Texture                 bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    sf::Font font;

    // Labels
    sf::Text labelBoard;
    sf::Text labelMode;

    // Board size options: 9x9, 13x13, 19x19
    std::vector<Button> boardButtons;

    // Game mode buttons
    Button btn2P;// 2 Players Mode
    Button btnEasy;// Easy AI
    Button btnMedium;// Medium AI
    Button btnHard;// Hard AI

    Button btnStart;// Start the game

    // Return
    Button btnReturn;

    bool layoutDone;

    int selectedBoard;  
    int selectedMode;   

    std::vector<sf::Vector2f> boardBtnPositions;// size = 3
    std::array<sf::Vector2f, 4> modeBtnPositions;// 0:2P,1:Easy,2:Medium,3:Hard
};
