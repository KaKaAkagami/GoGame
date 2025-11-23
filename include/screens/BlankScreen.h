#pragma once
#include "Screen.h"
#include <string>

class BlankScreen : public Screen
{
public:
    explicit BlankScreen(const std::string& message);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::Font font;
    sf::Text text;
};
