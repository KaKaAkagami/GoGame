#include "screens/SettingsScreen.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cctype>

#include "ConfigManager.h"

namespace
{
    constexpr const char* FONT_UI_PATH   = "assets/fonts/Inter_28pt-SemiBold.ttf";
    constexpr const char* BG_IMAGE_PATH  = "assets/img/menu_bg.jpg";
    constexpr const char* SETTINGS_PATH  = "settings.cfg";

    // board textures
    constexpr const char* MYTH_JPG   = "assets/img/Myth.jpg";
    constexpr const char* MYTH_PNG   = "assets/img/Myth.png";
    constexpr const char* PIRATE_JPG = "assets/img/Pirate.jpg";
    constexpr const char* PIRATE_PNG = "assets/img/Pirate.png";

    // stone textures
    constexpr const char* STONE_WEAPON_BLACK = "assets/img/stone/Weapon/Weapon_black.png";
    constexpr const char* STONE_WEAPON_WHITE = "assets/img/stone/Weapon/Weapon_white.png";
    constexpr const char* STONE_PIRATE_BLACK = "assets/img/stone/Pirate/Pirate_black.png";
    constexpr const char* STONE_PIRATE_WHITE = "assets/img/stone/Pirate/Pirate_white.png";

    static void centerOrigin(sf::Text& t)
    {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(sf::Vector2f{
            b.position.x + b.size.x * 0.5f,
            b.position.y + b.size.y * 0.5f
        });
    }

    static bool tryLoadTexture(sf::Texture& tex, const char* jpg, const char* png)
    {
        if (tex.loadFromFile(jpg)) return true;
        if (tex.loadFromFile(png)) return true;
        return false;
    }

    static sf::Color defaultWoodColor()
    {
        return sf::Color(214, 177, 122);
    }

    static sf::Color darkBlueColor()
    {
        // #283878
        return sf::Color(
            static_cast<std::uint8_t>(0x28),
            static_cast<std::uint8_t>(0x38),
            static_cast<std::uint8_t>(0x78)
        );
    }

    static std::string trim(const std::string& s)
    {
        auto isSpace = [](unsigned char ch){ return std::isspace(ch) != 0; };
        std::size_t a = 0;
        while (a < s.size() && isSpace((unsigned char)s[a])) ++a;
        std::size_t b = s.size();
        while (b > a && isSpace((unsigned char)s[b - 1])) --b;
        return s.substr(a, b - a);
    }
}

SettingsScreen::SettingsScreen(NavigateFn onNavigate,
                               SetVolumeFn onSetVolume,
                               GetVolumeFn onGetVolume)
    : navigate(std::move(onNavigate))
    , setVolume(std::move(onSetVolume))
    , getVolume(std::move(onGetVolume))
    , bgTexture()
    , bgSprite(nullptr)
    , font()

    , bgmTagRect()
    , bgmTagText(font, "BGM", 28U)
    , sliderTrack()
    , sliderKnob()
    , bgmValueText(font, "0", 28U)
    , draggingSlider(false)

    , editTabRect()
    , editTabText(font, "Edit Board", 28U)

    , btnDarkBlue(font, "Dark Blue", 28U)
    , btnDefault(font, "Default", 28U)
    , btnMyth(font, "Myth", 28U)
    , btnPirate(font, "Pirate", 28U)

    , previewBoardRect()
    , previewTex()
    , previewHasTexture(false)

    // Edit Stone
    , editStoneTabRect()
    , editStoneTabText(font, "Edit Stone", 28U)

    , btnStoneWeapon(font, "Weapon", 28U)
    , btnStonePirate(font, "Pirate", 28U)
    , btnStoneDefault(font, "Default", 28U)

    , stonePreviewRect()
    , stoneBlackTex()
    , stoneWhiteTex()
    , stonePreviewLoaded(false)

    // current settings
    , currentVolume(40.f)
    , boardTheme("Default")
    , stoneTheme("Default")

    , btnReturn(font, "Return", 28U)
    , layoutDone(false)
{
    if (!font.openFromFile(FONT_UI_PATH))
        std::cerr << "[SettingsScreen] Failed to load font: " << FONT_UI_PATH << "\n";

    if (!bgTexture.loadFromFile(BG_IMAGE_PATH))
        std::cerr << "[SettingsScreen] Failed to load background: " << BG_IMAGE_PATH << "\n";
    else
    {
        bgTexture.setSmooth(true);
        bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    }

    // base styles
    bgmTagText.setFillColor(sf::Color::White);
    bgmValueText.setFillColor(sf::Color::White);
    editTabText.setFillColor(sf::Color::White);
    editStoneTabText.setFillColor(sf::Color::White);

    bgmTagRect.setFillColor(sf::Color(30, 30, 30));
    bgmTagRect.setOutlineColor(sf::Color::White);
    bgmTagRect.setOutlineThickness(2.f);

    editTabRect.setFillColor(sf::Color(30, 30, 30));
    editTabRect.setOutlineColor(sf::Color::White);
    editTabRect.setOutlineThickness(2.f);

    editStoneTabRect.setFillColor(sf::Color(30, 30, 30));
    editStoneTabRect.setOutlineColor(sf::Color::White);
    editStoneTabRect.setOutlineThickness(2.f);

    sliderTrack.setFillColor(sf::Color(120, 120, 120, 200));
    sliderTrack.setOutlineColor(sf::Color::White);
    sliderTrack.setOutlineThickness(2.f);

    sliderKnob.setFillColor(sf::Color::White);
    sliderKnob.setOutlineColor(sf::Color::Black);
    sliderKnob.setOutlineThickness(2.f);

    stonePreviewRect.setFillColor(sf::Color(0, 0, 0, 30));
    stonePreviewRect.setOutlineColor(sf::Color::White);
    stonePreviewRect.setOutlineThickness(2.f);

    // Load persisted settings first (volume + board + stone)
    loadSettings();

    // callbacks
    btnReturn.setOnClick([this]()
    {
        if (navigate) navigate("Menu");
    });

    btnDarkBlue.setOnClick([this]() { setBoardTheme("DarkBlue"); });
    btnDefault .setOnClick([this]() { setBoardTheme("Default");  });
    btnMyth    .setOnClick([this]() { setBoardTheme("Myth");     });
    btnPirate  .setOnClick([this]() { setBoardTheme("Pirate");   });

    btnStoneWeapon .setOnClick([this]() { setStoneTheme("Weapon"); });
    btnStonePirate .setOnClick([this]() { setStoneTheme("Pirate"); });
    btnStoneDefault.setOnClick([this]() { setStoneTheme("Default"); });

    // apply volume
    if (setVolume) setVolume(currentVolume);
    updateVolumeText(currentVolume);

    // apply previews + global config
    setBoardTheme(boardTheme);
    setStoneTheme(stoneTheme);
}

void SettingsScreen::loadSettings()
{
    currentVolume = 40.f;
    boardTheme    = "Default";
    stoneTheme    = "Default";

    std::ifstream in(SETTINGS_PATH);
    if (!in) return;

    std::string line;
    while (std::getline(in, line))
    {
        line = trim(line);
        if (line.empty()) continue;

        if (line.rfind("volume=", 0) == 0)
        {
            try
            {
                float v = std::stof(line.substr(7));
                currentVolume = std::clamp(v, 0.f, 100.f);
            }
            catch (...) {}
        }
        else if (line.rfind("boardTheme=", 0) == 0)
        {
            std::string t = trim(line.substr(11));
            if (t == "Default" || t == "DarkBlue" || t == "Myth" || t == "Pirate")
                boardTheme = t;
        }
        else if (line.rfind("stoneTheme=", 0) == 0)
        {
            std::string s = trim(line.substr(11));
            if (s == "Default" || s == "Weapon" || s == "Pirate")
                stoneTheme = s;
        }
    }
}

void SettingsScreen::saveSettings() const
{
    std::ofstream out(SETTINGS_PATH);
    if (!out)
    {
        std::cerr << "[SettingsScreen] Cannot write " << SETTINGS_PATH << "\n";
        return;
    }

    out << "volume=" << currentVolume << "\n";
    out << "boardTheme=" << boardTheme << "\n";
    out << "stoneTheme=" << stoneTheme << "\n";
}

void SettingsScreen::setBoardTheme(const std::string& theme)
{
    boardTheme = theme;

    previewHasTexture = false;
    previewBoardRect.setTexture(nullptr);

    if (boardTheme == "Default")
    {
        previewBoardRect.setFillColor(defaultWoodColor());
        ConfigManager::instance().setBoardColor(defaultWoodColor());
    }
    else if (boardTheme == "DarkBlue")
    {
        previewBoardRect.setFillColor(darkBlueColor());
        ConfigManager::instance().setBoardColor(darkBlueColor());
    }
    else if (boardTheme == "Myth")
    {
        if (tryLoadTexture(previewTex, MYTH_JPG, MYTH_PNG))
        {
            previewTex.setSmooth(true);
            previewBoardRect.setTexture(&previewTex);
            previewBoardRect.setFillColor(sf::Color::White);
            previewHasTexture = true;
        }
        else
        {
            previewBoardRect.setFillColor(defaultWoodColor());
            ConfigManager::instance().setBoardColor(defaultWoodColor());
        }
    }
    else if (boardTheme == "Pirate")
    {
        if (tryLoadTexture(previewTex, PIRATE_JPG, PIRATE_PNG))
        {
            previewTex.setSmooth(true);
            previewBoardRect.setTexture(&previewTex);
            previewBoardRect.setFillColor(sf::Color::White);
            previewHasTexture = true;
        }
        else
        {
            previewBoardRect.setFillColor(defaultWoodColor());
            ConfigManager::instance().setBoardColor(defaultWoodColor());
        }
    }

    saveSettings();
}

void SettingsScreen::setStoneTheme(const std::string& theme)
{
    stoneTheme = theme;
    stonePreviewLoaded = false;

    // Global apply for GameScreen (ConfigManager will load textures)
    ConfigManager::instance().setStoneTheme(stoneTheme);

    if (stoneTheme == "Weapon")
    {
        bool okB = stoneBlackTex.loadFromFile(STONE_WEAPON_BLACK);
        bool okW = stoneWhiteTex.loadFromFile(STONE_WEAPON_WHITE);
        if (okB && okW)
        {
            stoneBlackTex.setSmooth(true);
            stoneWhiteTex.setSmooth(true);
            stonePreviewLoaded = true;
        }
    }
    else if (stoneTheme == "Pirate")
    {
        bool okB = stoneBlackTex.loadFromFile(STONE_PIRATE_BLACK);
        bool okW = stoneWhiteTex.loadFromFile(STONE_PIRATE_WHITE);
        if (okB && okW)
        {
            stoneBlackTex.setSmooth(true);
            stoneWhiteTex.setSmooth(true);
            stonePreviewLoaded = true;
        }
    }

    saveSettings();
}

float SettingsScreen::volumeFromMouseX(float mx) const
{
    const float x0 = sliderTrack.getPosition().x;
    const float w  = sliderTrack.getSize().x;

    float t = 0.f;
    if (w > 0.f) t = (mx - x0) / w;
    t = std::clamp(t, 0.f, 1.f);

    return t * 100.f;
}

void SettingsScreen::updateKnobFromVolume(float v)
{
    v = std::clamp(v, 0.f, 100.f);

    const float x0 = sliderTrack.getPosition().x;
    const float y0 = sliderTrack.getPosition().y;
    const float w  = sliderTrack.getSize().x;
    const float h  = sliderTrack.getSize().y;

    float t = v / 100.f;

    float knobX = x0 + t * w;
    float knobY = y0 + h * 0.5f;

    sliderKnob.setPosition(sf::Vector2f{knobX, knobY});
}

void SettingsScreen::updateVolumeText(float v)
{
    int iv = (int)std::lround(std::clamp(v, 0.f, 100.f));
    bgmValueText.setString(std::to_string(iv));
}

void SettingsScreen::layout(const sf::Vector2u& winSize)
{
    const float winW = (float)winSize.x;
    const float winH = (float)winSize.y;

    if (bgSprite)
    {
        const sf::Vector2u texSize = bgTexture.getSize();
        if (texSize.x && texSize.y)
        {
            const float sx = winW / (float)texSize.x;
            const float sy = winH / (float)texSize.y;
            bgSprite->setScale(sf::Vector2f{sx, sy});
        }
        bgSprite->setPosition(sf::Vector2f{0.f, 0.f});
    }

    const float margin = 18.f;
    const float tagW   = 90.f;
    const float tagH   = 54.f;
    const float gap    = 16.f;

    bgmTagRect.setSize(sf::Vector2f{tagW, tagH});
    bgmTagRect.setPosition(sf::Vector2f{margin, margin});

    centerOrigin(bgmTagText);
    bgmTagText.setPosition(sf::Vector2f{
        margin + tagW * 0.5f,
        margin + tagH * 0.5f
    });

    const float trackX = margin + tagW + gap;
    const float trackY = margin;
    const float trackW = winW - trackX - margin - 70.f;
    const float trackH = tagH;

    sliderTrack.setSize(sf::Vector2f{std::max(200.f, trackW), trackH});
    sliderTrack.setPosition(sf::Vector2f{trackX, trackY});

    const float knobR = trackH * 0.32f;
    sliderKnob.setRadius(knobR);
    sliderKnob.setOrigin(sf::Vector2f{knobR, knobR});
    updateKnobFromVolume(currentVolume);

    centerOrigin(bgmValueText);
    bgmValueText.setPosition(sf::Vector2f{
        sliderTrack.getPosition().x + sliderTrack.getSize().x + 38.f,
        sliderTrack.getPosition().y + sliderTrack.getSize().y * 0.5f
    });

    const float leftHalfW   = winW * 0.5f;
    const float leftCenterX = leftHalfW * 0.5f;

    const float tabW = 220.f;
    const float tabH = 56.f;

    editTabRect.setSize(sf::Vector2f{tabW, tabH});
    editTabRect.setPosition(sf::Vector2f{
        leftCenterX - tabW * 0.5f,
        margin + tagH + 22.f
    });

    centerOrigin(editTabText);
    editTabText.setPosition(sf::Vector2f{
        editTabRect.getPosition().x + tabW * 0.5f,
        editTabRect.getPosition().y + tabH * 0.5f
    });

    const float btnW    = 260.f;
    const float btnH    = 58.f;
    const float btnGapX = 18.f;
    const float btnGapY = 18.f;

    const float row1Y = editTabRect.getPosition().y + tabH + 18.f;
    const float twoW  = btnW * 2.f + btnGapX;
    const float rowX  = leftCenterX - twoW * 0.5f;

    btnDarkBlue.setSize(sf::Vector2f{btnW, btnH});
    btnDefault .setSize(sf::Vector2f{btnW, btnH});
    btnMyth    .setSize(sf::Vector2f{btnW, btnH});
    btnPirate  .setSize(sf::Vector2f{btnW, btnH});

    btnDarkBlue.setPosition(sf::Vector2f{rowX, row1Y});
    btnDefault .setPosition(sf::Vector2f{rowX + btnW + btnGapX, row1Y});

    const float row2Y = row1Y + btnH + btnGapY;
    btnMyth  .setPosition(sf::Vector2f{rowX, row2Y});
    btnPirate.setPosition(sf::Vector2f{rowX + btnW + btnGapX, row2Y});

    const float previewTop  = row2Y + btnH + 22.f;
    const float previewSize = std::min(winH - previewTop - 110.f, leftHalfW * 0.78f);

    previewBoardRect.setSize(sf::Vector2f{previewSize, previewSize});
    previewBoardRect.setOutlineColor(sf::Color::Black);
    previewBoardRect.setOutlineThickness(3.f);
    previewBoardRect.setPosition(sf::Vector2f{
        leftCenterX - previewSize * 0.5f,
        previewTop
    });

    // Right half (Edit Stone)
    const float rightHalfX   = leftHalfW;
    const float rightHalfW   = winW - rightHalfX;
    const float rightCenterX = rightHalfX + rightHalfW * 0.5f;

    editStoneTabRect.setSize(sf::Vector2f{tabW, tabH});
    editStoneTabRect.setPosition(sf::Vector2f{
        rightCenterX - tabW * 0.5f,
        editTabRect.getPosition().y
    });

    centerOrigin(editStoneTabText);
    editStoneTabText.setPosition(sf::Vector2f{
        editStoneTabRect.getPosition().x + tabW * 0.5f,
        editStoneTabRect.getPosition().y + tabH * 0.5f
    });

    btnStoneWeapon.setSize(sf::Vector2f{btnW, btnH});
    btnStonePirate.setSize(sf::Vector2f{btnW, btnH});
    btnStoneDefault.setSize(sf::Vector2f{btnW, btnH});

    const float stoneRowX = rightCenterX - twoW * 0.5f;
    btnStoneWeapon.setPosition(sf::Vector2f{stoneRowX, row1Y});
    btnStonePirate.setPosition(sf::Vector2f{stoneRowX + btnW + btnGapX, row1Y});

    btnStoneDefault.setPosition(sf::Vector2f{
        rightCenterX - btnW * 0.5f,
        row2Y
    });

    stonePreviewRect.setSize(sf::Vector2f{previewSize, previewSize});
    stonePreviewRect.setPosition(sf::Vector2f{
        rightCenterX - previewSize * 0.5f,
        previewTop
    });

    // Return
    const float paddingX = 48.f;
    const float paddingY = 18.f;

    const float textW = btnReturn.textWidth();
    const float retW  = textW + paddingX;
    const float retH  = btnReturn.height() + paddingY;

    btnReturn.setSize(sf::Vector2f{retW, retH});
    btnReturn.setPosition(sf::Vector2f{
        winW - retW - margin,
        winH - retH - margin
    });

    updateVolumeText(currentVolume);
    centerOrigin(bgmValueText);
    bgmValueText.setPosition(sf::Vector2f{
        sliderTrack.getPosition().x + sliderTrack.getSize().x + 38.f,
        sliderTrack.getPosition().y + sliderTrack.getSize().y * 0.5f
    });

    updateKnobFromVolume(currentVolume);

    layoutDone = true;
}

void SettingsScreen::handleEvent(const sf::Event& e)
{
    if (!layoutDone) return;

    btnDarkBlue.handleEvent(e);
    btnDefault .handleEvent(e);
    btnMyth    .handleEvent(e);
    btnPirate  .handleEvent(e);

    btnStoneWeapon.handleEvent(e);
    btnStonePirate.handleEvent(e);
    btnStoneDefault.handleEvent(e);

    btnReturn.handleEvent(e);

    if (auto* mp = e.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mp->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f m((float)mp->position.x, (float)mp->position.y);

            if (sliderKnob.getGlobalBounds().contains(m) ||
                sliderTrack.getGlobalBounds().contains(m))
            {
                draggingSlider = true;

                float v = volumeFromMouseX(m.x);
                currentVolume = v;

                if (setVolume) setVolume(v);
                updateKnobFromVolume(v);
                updateVolumeText(v);
                saveSettings();

                return;
            }
        }
    }

    if (auto* mr = e.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mr->button == sf::Mouse::Button::Left)
            draggingSlider = false;
    }

    if (auto* mm = e.getIf<sf::Event::MouseMoved>())
    {
        if (draggingSlider)
        {
            float mx = (float)mm->position.x;

            float v = volumeFromMouseX(mx);
            currentVolume = v;

            if (setVolume) setVolume(v);
            updateKnobFromVolume(v);
            updateVolumeText(v);
            saveSettings();
        }
    }
}

void SettingsScreen::update(float) {}

void SettingsScreen::draw(sf::RenderWindow& window)
{
    if (!layoutDone)
        layout(window.getSize());

    if (bgSprite)
        window.draw(*bgSprite);

    window.draw(bgmTagRect);
    window.draw(bgmTagText);
    window.draw(sliderTrack);
    window.draw(sliderKnob);
    window.draw(bgmValueText);

    window.draw(editTabRect);
    window.draw(editTabText);

    btnDarkBlue.draw(window);
    btnDefault.draw(window);
    btnMyth.draw(window);
    btnPirate.draw(window);

    window.draw(previewBoardRect);

    // 9x9 grid preview
    {
        const int nPoints = 9;
        const int segments = nPoints - 1;

        const sf::Vector2f pos = previewBoardRect.getPosition();
        const sf::Vector2f sz  = previewBoardRect.getSize();

        const float frame = 26.f;
        const float inner = std::max(10.f, sz.x - frame * 2.f);
        const float cell  = inner / (float)segments;

        const float ox = pos.x + frame;
        const float oy = pos.y + frame;

        sf::Color gridColor = sf::Color::Black;

        for (int i = 0; i < nPoints; ++i)
        {
            float offset = (float)i * cell;

            sf::RectangleShape vLine;
            vLine.setSize(sf::Vector2f{1.f, inner});
            vLine.setFillColor(gridColor);
            vLine.setPosition(sf::Vector2f{ox + offset, oy});
            window.draw(vLine);

            sf::RectangleShape hLine;
            hLine.setSize(sf::Vector2f{inner, 1.f});
            hLine.setFillColor(gridColor);
            hLine.setPosition(sf::Vector2f{ox, oy + offset});
            window.draw(hLine);
        }
    }

    // Edit Stone (right)
    window.draw(editStoneTabRect);
    window.draw(editStoneTabText);

    btnStoneWeapon.draw(window);
    btnStonePirate.draw(window);
    btnStoneDefault.draw(window);

    window.draw(stonePreviewRect);

    // demo 2 stones
    {
        sf::Vector2f pos = stonePreviewRect.getPosition();
        sf::Vector2f sz  = stonePreviewRect.getSize();

        float cx1 = pos.x + sz.x * 0.35f;
        float cx2 = pos.x + sz.x * 0.65f;
        float cy  = pos.y + sz.y * 0.50f;

        float target = sz.x * 0.22f;

        if (stoneTheme == "Default" || !stonePreviewLoaded)
        {
            float r = target * 0.5f;

            sf::CircleShape b(r);
            b.setOrigin(sf::Vector2f{r, r});
            b.setPosition(sf::Vector2f{cx1, cy});
            b.setFillColor(sf::Color::Black);
            b.setOutlineColor(sf::Color(80, 80, 80));
            b.setOutlineThickness(2.f);
            window.draw(b);

            sf::CircleShape w(r);
            w.setOrigin(sf::Vector2f{r, r});
            w.setPosition(sf::Vector2f{cx2, cy});
            w.setFillColor(sf::Color::White);
            w.setOutlineColor(sf::Color(160, 160, 160));
            w.setOutlineThickness(2.f);
            window.draw(w);
        }
        else
        {
            sf::Sprite sB(stoneBlackTex);
            sf::Sprite sW(stoneWhiteTex);

            auto bb = sB.getLocalBounds();
            auto bw = sW.getLocalBounds();

            if (bb.size.x > 0 && bb.size.y > 0)
            {
                sB.setOrigin(sf::Vector2f{bb.size.x * 0.5f, bb.size.y * 0.5f});
                float scaleB = target / std::max(bb.size.x, bb.size.y);
                sB.setScale(sf::Vector2f{scaleB, scaleB});
                sB.setPosition(sf::Vector2f{cx1, cy});
                window.draw(sB);
            }

            if (bw.size.x > 0 && bw.size.y > 0)
            {
                sW.setOrigin(sf::Vector2f{bw.size.x * 0.5f, bw.size.y * 0.5f});
                float scaleW = target / std::max(bw.size.x, bw.size.y);
                sW.setScale(sf::Vector2f{scaleW, scaleW});
                sW.setPosition(sf::Vector2f{cx2, cy});
                window.draw(sW);
            }
        }
    }

    btnReturn.draw(window);
}
