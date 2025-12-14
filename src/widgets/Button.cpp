#include "widgets/Button.h"
#include <iostream>


Button::Button(const sf::Font& font, const std::string& text, unsigned int size)
    : background{}
    , label(font, text, size)   
    , onClick(nullptr)
{
    // màu chữ trắng
    label.setFillColor(sf::Color::White);

    // style ô
    background.setFillColor(sf::Color(30, 30, 30));
    background.setOutlineColor(sf::Color::White);
    background.setOutlineThickness(2.f);

    // padding quanh chữ
    const float paddingX = 32.f;
    const float paddingY = 14.f;

    const sf::FloatRect bounds = label.getLocalBounds();
    sf::Vector2f bgSize{
        bounds.size.x + paddingX * 2.f,
        bounds.size.y + paddingY * 2.f
    };

    background.setSize(bgSize);

    
    background.setPosition(sf::Vector2f{0.f, 0.f});
    updateLabelTransform();
}

// geometry
void Button::setSize(const sf::Vector2f& size)
{
    background.setSize(size);
    updateLabelTransform();
}

void Button::setPosition(const sf::Vector2f& pos)
{
    background.setPosition(pos);
    updateLabelTransform();
}

float Button::width() const
{
    return background.getSize().x;
}

float Button::height() const
{
    return background.getSize().y;
}

float Button::textWidth() const
{
    const sf::FloatRect b = label.getLocalBounds();
    return b.size.x;
}

// callback
void Button::setOnClick(ClickCallback cb)
{
    onClick = std::move(cb);
}

// hit test
bool Button::contains(const sf::Vector2f& p) const
{
    return background.getGlobalBounds().contains(p);
}

// event
void Button::handleEvent(const sf::Event& e)
{
    if (!onClick)
        return;

    if (auto* m = e.getIf<sf::Event::MouseButtonPressed>())
    {
        if (m->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mp{
                static_cast<float>(m->position.x),
                static_cast<float>(m->position.y)
            };

            if (contains(mp))
                onClick();
        }
    }
}

// draw
void Button::draw(sf::RenderTarget& target) const
{
    target.draw(background);
    target.draw(label);
}

// private: căn giữa label trong ô
void Button::updateLabelTransform()
{
    const sf::Vector2f size = background.getSize();
    const sf::Vector2f pos  = background.getPosition();

    sf::Vector2f center{
        pos.x + size.x * 0.5f,
        pos.y + size.y * 0.5f
    };

    const sf::FloatRect tb = label.getLocalBounds();
    sf::Vector2f textCenter{
        tb.position.x + tb.size.x * 0.5f,
        tb.position.y + tb.size.y * 0.5f
    };

    label.setOrigin(textCenter);
    label.setPosition(center);
}
