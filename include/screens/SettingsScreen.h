
#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>
#include <string>
#include <memory>

#include "Screen.h"
#include "widgets/Button.h"

class SettingsScreen : public Screen
{
public:
    using NavigateFn  = std::function<void(const std::string&)>;
    using SetVolumeFn = std::function<void(float)>;
    using GetVolumeFn = std::function<float(void)>;

    explicit SettingsScreen(NavigateFn onNavigate,
                            SetVolumeFn onSetVolume,
                            GetVolumeFn onGetVolume);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void layout(const sf::Vector2u& winSize);

    
    void loadSettings();
    void saveSettings() const;

    
    float volumeFromMouseX(float mx) const;
    void  updateKnobFromVolume(float v);
    void  updateVolumeText(float v);

   
    void setBoardTheme(const std::string& theme);

   
    void setStoneTheme(const std::string& theme);

private:
    
    NavigateFn  navigate;
    SetVolumeFn setVolume;
    GetVolumeFn getVolume;

    sf::Texture bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    sf::Font font;

    sf::RectangleShape bgmTagRect;
    sf::Text           bgmTagText;

    sf::RectangleShape sliderTrack;
    sf::CircleShape    sliderKnob;
    sf::Text           bgmValueText;

    bool  draggingSlider;
    float currentVolume;

    sf::RectangleShape editTabRect;
    sf::Text           editTabText;

    Button btnDarkBlue;
    Button btnDefault;
    Button btnMyth;
    Button btnPirate;

    sf::RectangleShape previewBoardRect;
    sf::Texture        previewTex;
    bool               previewHasTexture;
    std::string        boardTheme;

    sf::RectangleShape editStoneTabRect;
    sf::Text           editStoneTabText;

    Button btnStoneWeapon;
    Button btnStonePirate;
    Button btnStoneDefault;

    sf::RectangleShape stonePreviewRect;
    sf::Texture        stoneBlackTex;
    sf::Texture        stoneWhiteTex;
    bool               stonePreviewLoaded;
    std::string        stoneTheme;

    
    Button btnReturn;
    bool   layoutDone;
};
