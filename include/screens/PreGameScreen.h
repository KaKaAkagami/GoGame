
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

   
    sf::Text labelBoard;
    sf::Text labelMode;

    
    std::vector<Button> boardButtons;

   
    Button btn2P;
    Button btnEasy;
    Button btnMedium;
    Button btnHard;

    Button btnStart;

  
    Button btnReturn;

    bool layoutDone;

    int selectedBoard;  
    int selectedMode;   

    std::vector<sf::Vector2f> boardBtnPositions;
    std::array<sf::Vector2f, 4> modeBtnPositions;
};
