
#pragma once
#include <memory>
#include <SFML/Graphics.hpp>

class IconButton
{
public:
    IconButton() = default;

    void setTexture(const sf::Texture& texture);
    void setPosition(sf::Vector2f pos);
    void setSize(sf::Vector2f size);
    bool contains(sf::Vector2f point) const;
    void draw(sf::RenderWindow& window) const;

private:
    std::unique_ptr<sf::Sprite> sprite; 
};
