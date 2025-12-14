#include "ConfigManager.h"

ConfigManager& ConfigManager::instance()
{
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager()
{
    // Default board color giống màu gỗ hiện tại của bạn
    config.boardColor = sf::Color(214, 177, 122);

    // Default stone theme
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
    // Chỉ nhận 3 giá trị hợp lệ để tránh lỗi config bậy
    if (theme == "Default" || theme == "Weapon" || theme == "Pirate")
        config.stoneTheme = theme;
    else
        config.stoneTheme = "Default";
}

const std::string& ConfigManager::getStoneTheme() const
{
    return config.stoneTheme;
}
