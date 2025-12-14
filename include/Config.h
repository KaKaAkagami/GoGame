#pragma once
#include <SFML/Graphics.hpp>
#include <string>

struct BoardConfig
{
    sf::Color boardColor;

    // "Default" | "Pirate" | "Weapon"
    std::string stoneSkin = "Default";
};
