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

    // settings persistence
    void loadSettings();
    void saveSettings() const;

    // volume helpers
    float volumeFromMouseX(float mx) const;
    void  updateKnobFromVolume(float v);
    void  updateVolumeText(float v);

    // board theme
    void setBoardTheme(const std::string& theme);

    // stone theme
    void setStoneTheme(const std::string& theme);

private:
    // callbacks
    NavigateFn  navigate;
    SetVolumeFn setVolume;
    GetVolumeFn getVolume;

    // background
    sf::Texture bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    sf::Font font;

    // ===== BGM =====
    sf::RectangleShape bgmTagRect;
    sf::Text           bgmTagText;

    sf::RectangleShape sliderTrack;
    sf::CircleShape    sliderKnob;
    sf::Text           bgmValueText;

    bool  draggingSlider;
    float currentVolume;

    // ===== Edit Board =====
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

    // ===== Edit Stone =====
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

    // return
    Button btnReturn;
    bool   layoutDone;
};
