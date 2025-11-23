#pragma once
#include <SFML/Graphics.hpp>

class Screen
{
public:
    virtual ~Screen() = default;

    virtual void handleEvent(const sf::Event& e) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};
