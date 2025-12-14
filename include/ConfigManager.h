#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class ConfigManager
{
public:
    struct Config
    {
        sf::Color    boardColor;
        std::string  stoneTheme; 
    };

    static ConfigManager& instance();

    
    void setBoardColor(const sf::Color& c);
    sf::Color getBoardColor() const;

    
    void setStoneTheme(const std::string& theme);
    const std::string& getStoneTheme() const;

private:
    ConfigManager();

private:
    Config config;
};
