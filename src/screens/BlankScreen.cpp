#include "screens/BlankScreen.h"

BlankScreen::BlankScreen(const std::string& message)
    : font()
    , text(font, message, 32U)
{
    [[maybe_unused]] bool okFont =
        font.openFromFile("assets/fonts/Inter_28pt-SemiBold.ttf");

    text.setFillColor(sf::Color::White);
    text.setLetterSpacing(1.2f);
    text.setPosition(sf::Vector2f{80.f, 320.f});
}

void BlankScreen::handleEvent(const sf::Event& /*e*/)
{
    // màn hình trống, chưa xử lý gì
}

void BlankScreen::update(float /*dt*/)
{
    // không có animation
}

void BlankScreen::draw(sf::RenderWindow& window)
{
    window.draw(text);
}
