
#include "ScreenManager.h"
#include <iostream>

void ScreenManager::addScreen(const std::string& name, std::unique_ptr<Screen> screen)
{
    screens[name] = std::move(screen);
}

void ScreenManager::switchTo(const std::string& name)
{
    auto it = screens.find(name);
    if (it == screens.end())
    {
        std::cerr << "[ScreenManager] Tried to switch to unknown screen: "
                  << name << '\n';
        return;
    }

    current = it->second.get();
}

void ScreenManager::handleEvent(const sf::Event& e)
{
    if (current)
        current->handleEvent(e);
}

void ScreenManager::update(float dt)
{
    if (current)
        current->update(dt);
}

void ScreenManager::draw(sf::RenderWindow& window)
{
    if (current)
        current->draw(window);
}
