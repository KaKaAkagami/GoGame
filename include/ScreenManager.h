#pragma once
#include "Screen.h"

#include <map>
#include <memory>
#include <string>

class ScreenManager
{
public:
    void addScreen(const std::string& name, std::unique_ptr<Screen> screen);
    void switchTo(const std::string& name);

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    std::map<std::string, std::unique_ptr<Screen>> screens;
    Screen* current = nullptr;
};
