
#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Screen.h"
#include "widgets/Button.h"

class MenuScreen : public Screen
{
public:
    using NavigateFn = std::function<void(const std::string&)>;

    explicit MenuScreen(NavigateFn onNavigate);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void layout(const sf::Vector2u& winSize);

private:
    NavigateFn navigate;

    
    sf::Texture bgTexture;
    std::unique_ptr<sf::Sprite>bgSprite;   

    
    sf::Font fontUI;
    sf::Font fontTitle;
    sf::Text title;
    sf::Text madeBy;
    std::vector<Button> buttons;

    bool layoutDone;
};
