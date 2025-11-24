#pragma once 

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class Button
{
public:
    using ClickCallback = std::function<void()>;

    Button(const sf::Font& font, const std::string& text, unsigned int size);

    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);

    sf::Vector2f getPosition() const { return background.getPosition(); }
    sf::Vector2f getSize() const { return background.getSize(); }

    float width() const;
    float height() const;
    float textWidth() const;

    void setOnClick(ClickCallback cb);

    bool contains(const sf::Vector2f& p) const;
    void handleEvent(const sf::Event& e);
    void draw(sf::RenderTarget& target) const;

private:
    void updateLabelTransform(); // căn giữa label trong ô

    sf::RectangleShape background;
    sf::Text label;
    ClickCallback onClick;
};
