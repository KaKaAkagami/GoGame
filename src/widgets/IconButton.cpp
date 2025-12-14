
#include "widgets/IconButton.h"

void IconButton::setTexture(const sf::Texture& texture)
{
    sprite = std::make_unique<sf::Sprite>(texture);
}

void IconButton::setPosition(sf::Vector2f pos)
{
    if (!sprite)
        return;
    sprite->setPosition(pos);
}

void IconButton::setSize(sf::Vector2f size)
{
    if (!sprite)
        return;

    const auto bounds = sprite->getGlobalBounds();
    if (bounds.size.x == 0.f || bounds.size.y == 0.f)
        return;

    const float sx = size.x / bounds.size.x;
    const float sy = size.y / bounds.size.y;

    sprite->setScale(sf::Vector2f{sx, sy});
}

bool IconButton::contains(sf::Vector2f point) const
{
    if (!sprite)
        return false;
    return sprite->getGlobalBounds().contains(point);
}

void IconButton::draw(sf::RenderWindow& window) const
{
    if (sprite)
        window.draw(*sprite);
}
