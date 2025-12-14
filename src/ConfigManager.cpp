#include "ConfigManager.h"

ConfigManager& ConfigManager::instance()
{
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager()
{
    
    config.boardColor = sf::Color(214, 177, 122);

    
    config.stoneTheme = "Default";
}

void ConfigManager::setBoardColor(const sf::Color& c)
{
    config.boardColor = c;
}

sf::Color ConfigManager::getBoardColor() const
{
    return config.boardColor;
}

void ConfigManager::setStoneTheme(const std::string& theme)
{
   
    if (theme == "Default" || theme == "Weapon" || theme == "Pirate")
        config.stoneTheme = theme;
    else
        config.stoneTheme = "Default";
}

const std::string& ConfigManager::getStoneTheme() const
{
    return config.stoneTheme;
}
