#include <SFML/Graphics.hpp>
#include <algorithm>
#include <optional>
#include <string>

void initMine(const sf::Font& font);
void updateMine(const sf::Vector2f& mousePos, int money, float deltaTime, bool keyA, bool keyD, long long& collectedIron, int& xp);
bool handleMineClick(const sf::Vector2f& mousePos, long long& collectedIron, int& money, int& xp);
void drawMine(sf::RenderWindow& window, int money);
void playMineMusic();
void stopMineMusic();
void playIronworksMusic();
void stopIronworksMusic();
void playStocksMusic();
void stopStocksMusic();

void initIronworks(const sf::Font& font);
void updateIronworks(const sf::Vector2f& mousePos, long long& iron, long long& steel, int& money);
bool handleIronworksClick(const sf::Vector2f& mousePos, long long& iron, long long& steel, int& money);
void drawIronworks(sf::RenderWindow& window, int money);

void initStocks(const sf::Font& font);
void updateStocks(const sf::Vector2f& mousePos, long long& steel, int& money);
bool handleStocksClick(const sf::Vector2f& mousePos, long long& steel, int& money);
void drawStocks(sf::RenderWindow& window);

enum class Location {
    StartMenu,
    Mine,
    Furnace,
    Market,
    Map
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Kliker - Huta");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("fonts/arial.ttf")) //FONT REQUIRED (.ttf)!
    {
        // Font loading failed, but continue anyway
    }

    // --- MAP BACKGROUND ---
    sf::Texture mapBgTexture;
    std::optional<sf::Sprite> mapBgSprite;
    if (mapBgTexture.loadFromFile("images/map-bg.jpg"))
    {
        mapBgSprite = sf::Sprite(mapBgTexture);
        // Scale to fit window
        sf::Vector2u textureSize = mapBgTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 800.f / static_cast<float>(textureSize.x);
            float scaleY = 600.f / static_cast<float>(textureSize.y);
            mapBgSprite->setScale({scaleX, scaleY});
            mapBgSprite->setPosition({0.f, 0.f});
        }
    }

    // --- LOCATION BACKGROUNDS ---
    // Mine (Kopalnia) background
    sf::Texture mineBgTexture;
    std::optional<sf::Sprite> mineBgSprite;
    if (mineBgTexture.loadFromFile("images/mine-bg.jpg"))
    {
        mineBgSprite = sf::Sprite(mineBgTexture);
        sf::Vector2u textureSize = mineBgTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 800.f / static_cast<float>(textureSize.x);
            float scaleY = 600.f / static_cast<float>(textureSize.y);
            mineBgSprite->setScale({scaleX, scaleY});
            mineBgSprite->setPosition({0.f, 0.f});
        }
    }

    // Furnace (Huta) background
    sf::Texture furnaceBgTexture;
    std::optional<sf::Sprite> furnaceBgSprite;
    if (furnaceBgTexture.loadFromFile("images/ironworks-bg.jpg"))
    {
        furnaceBgSprite = sf::Sprite(furnaceBgTexture);
        sf::Vector2u textureSize = furnaceBgTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 800.f / static_cast<float>(textureSize.x);
            float scaleY = 600.f / static_cast<float>(textureSize.y);
            furnaceBgSprite->setScale({scaleX, scaleY});
            furnaceBgSprite->setPosition({0.f, 0.f});
        }
    }

    // Menu background
    sf::Texture menuBgTexture;
    std::optional<sf::Sprite> menuBgSprite;
    float menuPanPosX = 0.f;
    float menuPanMinX = 0.f;
    float menuPanMaxX = 0.f;
    float menuPanSpeed = 20.f;
    int menuPanDir = 1;
    if (menuBgTexture.loadFromFile("images/menu-bg.jpg"))
    {
        menuBgSprite = sf::Sprite(menuBgTexture);
        sf::Vector2u ts = menuBgTexture.getSize();
        if (ts.x > 0 && ts.y > 0)
        {
            float scale = 600.f / static_cast<float>(ts.y);
            menuBgSprite->setScale({scale, scale});
            float scaledWidth = ts.x * scale;
            if (scaledWidth > 800.f)
            {
                menuPanMinX = 800.f - scaledWidth;
                menuPanMaxX = 0.f;
            }
            else
            {
                menuPanMinX = menuPanMaxX = (800.f - scaledWidth) / 2.f;
            }
            menuPanPosX = menuPanMinX;
            menuBgSprite->setPosition({menuPanPosX, 0.f});
        }
    }

    // Market (Gielda) background
    sf::Texture marketBgTexture;
    std::optional<sf::Sprite> marketBgSprite;
    if (marketBgTexture.loadFromFile("images/stock-bg.jpg"))
    {
        marketBgSprite = sf::Sprite(marketBgTexture);
        sf::Vector2u textureSize = marketBgTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 800.f / static_cast<float>(textureSize.x);
            float scaleY = 600.f / static_cast<float>(textureSize.y);
            marketBgSprite->setScale({scaleX, scaleY});
            marketBgSprite->setPosition({0.f, 0.f});
        }
    }

    // --- LOCATION ICONS (for map screen) ---
    const float iconSize = 120.f;
    const float iconSpacing = 30.f;
    
    // Mine icon
    sf::Texture mineIconTexture;
    std::optional<sf::Sprite> mineIconSprite;
    if (mineIconTexture.loadFromFile("images/mine-icon.png"))
    {
        mineIconSprite = sf::Sprite(mineIconTexture);
        sf::Vector2u textureSize = mineIconTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scale = iconSize / static_cast<float>(std::max(textureSize.x, textureSize.y));
            mineIconSprite->setScale({scale, scale});
        }
    }
    
    // Furnace icon
    sf::Texture furnaceIconTexture;
    std::optional<sf::Sprite> furnaceIconSprite;
    if (furnaceIconTexture.loadFromFile("images/furnace-icon.png"))
    {
        furnaceIconSprite = sf::Sprite(furnaceIconTexture);
        sf::Vector2u textureSize = furnaceIconTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scale = iconSize / static_cast<float>(std::max(textureSize.x, textureSize.y));
            furnaceIconSprite->setScale({scale, scale});
        }
    }
    
    // Market icon
    sf::Texture marketIconTexture;
    std::optional<sf::Sprite> marketIconSprite;
    if (marketIconTexture.loadFromFile("images/market-icon.png"))
    {
        marketIconSprite = sf::Sprite(marketIconTexture);
        sf::Vector2u textureSize = marketIconTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scale = iconSize / static_cast<float>(std::max(textureSize.x, textureSize.y));
            marketIconSprite->setScale({scale, scale});
        }
    }

    // --- GAME STATE ---
    Location currentLocation = Location::StartMenu;
    int level = 1;
    int xp = 0;
    int money = 200;
    long long collectedIron = 0;
    long long steel = 0;

    // Function to calculate XP needed for next level
    auto getXPForNextLevel = [](int currentLevel) -> int {
        switch(currentLevel) {
            case 1: return 50;
            case 2: return 100;
            case 3: return 150;
            case 4: return 250;
            case 5: return 400;
            case 6: return 600;
            case 7: return 850;
            case 8: return 1150;
            case 9: return 1500;
            default: return 2000 + (currentLevel - 10) * 500; // 2000, 2500, 3000...
        }
    };

    // --- UI: MAINBAR (TOP LEFT) ---
    sf::Texture mainbarTexture;
    std::optional<sf::Sprite> mainbarSprite;
    if (mainbarTexture.loadFromFile("images/UI-mainbar.png"))
    {
        mainbarSprite = sf::Sprite(mainbarTexture);
        sf::Vector2u textureSize = mainbarTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 250.f / static_cast<float>(textureSize.x);
            float scaleY = 120.f / static_cast<float>(textureSize.y);
            mainbarSprite->setScale({scaleX, scaleY});
        }
        mainbarSprite->setPosition({0.f, 0.f}); // Top left corner
    }

    // --- UI: STATS ICONS ---
    const float statsIconSize = 20.f;
    const float statsIconTextSpacing = 5.f;
    
    // Iron icon
    sf::Texture ironIconTexture;
    std::optional<sf::Sprite> ironIconSprite;
    if (ironIconTexture.loadFromFile("images/iron-ingot.png"))
    {
        ironIconSprite = sf::Sprite(ironIconTexture);
        sf::Vector2u textureSize = ironIconTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scale = statsIconSize / static_cast<float>(std::max(textureSize.x, textureSize.y));
            ironIconSprite->setScale({scale, scale});
        }
    }
    
    // Steel icon
    sf::Texture steelIconTexture;
    std::optional<sf::Sprite> steelIconSprite;
    if (steelIconTexture.loadFromFile("images/steel-icon.png"))
    {
        steelIconSprite = sf::Sprite(steelIconTexture);
        sf::Vector2u textureSize = steelIconTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scale = statsIconSize / static_cast<float>(std::max(textureSize.x, textureSize.y));
            steelIconSprite->setScale({scale, scale});
        }
    }
    
    // Cash icon
    sf::Texture cashIconTexture;
    std::optional<sf::Sprite> cashIconSprite;
    if (cashIconTexture.loadFromFile("images/cash-icon.png"))
    {
        cashIconSprite = sf::Sprite(cashIconTexture);
        sf::Vector2u textureSize = cashIconTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scale = statsIconSize / static_cast<float>(std::max(textureSize.x, textureSize.y));
            cashIconSprite->setScale({scale, scale});
        }
    }

    // --- UI: TOP LEFT (LEVEL + MONEY) ---
    sf::Text statsLevelText(font);
    sf::Text statsIronText(font);
    sf::Text statsSteelText(font);
    sf::Text statsMoneyText(font);
    const float statsCharSize = 18.f;
    statsLevelText.setCharacterSize(static_cast<unsigned int>(statsCharSize));
    statsIronText.setCharacterSize(static_cast<unsigned int>(statsCharSize));
    statsSteelText.setCharacterSize(static_cast<unsigned int>(statsCharSize));
    statsMoneyText.setCharacterSize(static_cast<unsigned int>(statsCharSize));
    statsLevelText.setFillColor(sf::Color::White);
    statsIronText.setFillColor(sf::Color::White);
    statsSteelText.setFillColor(sf::Color::White);
    statsMoneyText.setFillColor(sf::Color::White);
    // Base position and vertical spacing
    const sf::Vector2f statsBasePos = {10.f, 6.f};
    const float statsVSpacing = 25.5f;
    const float statsIconX = statsBasePos.x;
    statsLevelText.setPosition(statsBasePos);
    // Text positions adjusted to be after icons
    statsIronText.setPosition({statsBasePos.x + statsIconSize + statsIconTextSpacing, statsBasePos.y + statsVSpacing});
    statsSteelText.setPosition({statsBasePos.x + statsIconSize + statsIconTextSpacing, statsBasePos.y + statsVSpacing * 2.f});
    statsMoneyText.setPosition({statsBasePos.x + statsIconSize + statsIconTextSpacing, statsBasePos.y + statsVSpacing * 3.f + 4.f});

    // --- UI: MAP BUTTON (TOP RIGHT)
    sf::Texture mapButtonTexture;
    std::optional<sf::Sprite> mapButtonSprite;
    if (mapButtonTexture.loadFromFile("images/ui-mapa.png"))
    {
        mapButtonSprite = sf::Sprite(mapButtonTexture);
        mapButtonSprite->setPosition({690.f, 10.f});
        sf::Vector2u textureSize = mapButtonTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 100.f / static_cast<float>(textureSize.x);
            float scaleY = 40.f / static_cast<float>(textureSize.y);
            mapButtonSprite->setScale({scaleX, scaleY});
        }
    }
    
    sf::RectangleShape mapButton({100.f, 40.f});
    mapButton.setPosition({690.f, 10.f});

    // --- CENTER TEXT (WHERE YOU ARE) ---
    sf::Text locationText(font);
    locationText.setCharacterSize(32);
    locationText.setFillColor(sf::Color::White);
    locationText.setPosition({250.f, 250.f});

    // --- MAP BUTTONS ---
    const unsigned int mapTextSize = 52;
    sf::Text mineText(font, "KOPALNIA", mapTextSize);
    sf::Text furnaceText(font, "HUTA", mapTextSize);
    sf::Text marketText(font, "GIELDA", mapTextSize);

    // Text positions
    float textStartX = 200.f + iconSize + iconSpacing;
    float verticalSpacing = 140.f;
    float startY = 150.f;
    mineText.setPosition({textStartX, startY});
    furnaceText.setPosition({textStartX, startY + verticalSpacing});
    marketText.setPosition({textStartX, startY + verticalSpacing * 2.f});

    // Default colors for hover effects
    sf::Color mineTextDefaultColor = sf::Color::White;
    sf::Color furnaceTextDefaultColor = sf::Color::White;
    sf::Color marketTextDefaultColor = sf::Color::White;
    sf::Color hoverColor = sf::Color(255, 200, 100);

    // --- START MENU UI ---
    sf::RectangleShape playButton({200.f, 60.f});
    playButton.setPosition({300.f, 200.f});
    playButton.setFillColor(sf::Color(100, 100, 100));

    sf::RectangleShape quitButton({200.f, 60.f});
    quitButton.setPosition({300.f, 300.f});
    quitButton.setFillColor(sf::Color(100, 100, 100));

    // Button images (optional replacements)
    sf::Texture playBtnTexture;
    std::optional<sf::Sprite> playBtnSprite;
    sf::Vector2f playBtnBaseScale = {1.f, 1.f};
    sf::Texture quitBtnTexture;
    std::optional<sf::Sprite> quitBtnSprite;
    sf::Vector2f quitBtnBaseScale = {1.f, 1.f};
    sf::Texture statsBtnTexture;
    std::optional<sf::Sprite> statsBtnSprite;
    sf::Vector2f statsBtnBaseScale = {1.f, 1.f};
    // Try to load image buttons and position/scale them to the rectangle sizes
    if (playBtnTexture.loadFromFile("images/play-btn.png"))
    {
        playBtnSprite = sf::Sprite(playBtnTexture);
        sf::Vector2u ts = playBtnTexture.getSize();
        if (ts.x > 0 && ts.y > 0)
        {
            float scaleX = 200.f / static_cast<float>(ts.x);
            float scaleY = 60.f / static_cast<float>(ts.y);
            playBtnSprite->setScale({scaleX, scaleY});
            playBtnBaseScale = playBtnSprite->getScale();
        }
        playBtnSprite->setPosition(playButton.getPosition());
    }
    if (quitBtnTexture.loadFromFile("images/quit-btn.png"))
    {
        quitBtnSprite = sf::Sprite(quitBtnTexture);
        sf::Vector2u ts = quitBtnTexture.getSize();
        if (ts.x > 0 && ts.y > 0)
        {
            float scaleX = 200.f / static_cast<float>(ts.x);
            float scaleY = 60.f / static_cast<float>(ts.y);
            quitBtnSprite->setScale({scaleX, scaleY});
            quitBtnBaseScale = quitBtnSprite->getScale();
        }
        quitBtnSprite->setPosition(quitButton.getPosition());
    }
    // Optional stats button (placed below quit)
    if (statsBtnTexture.loadFromFile("images/stats-btn.png"))
    {
        statsBtnSprite = sf::Sprite(statsBtnTexture);
        sf::Vector2u ts = statsBtnTexture.getSize();
        if (ts.x > 0 && ts.y > 0)
        {
            float scaleX = 200.f / static_cast<float>(ts.x);
            float scaleY = 60.f / static_cast<float>(ts.y);
            statsBtnSprite->setScale({scaleX, scaleY});
            statsBtnBaseScale = statsBtnSprite->getScale();
        }
        statsBtnSprite->setPosition({300.f, 380.f});
    }

    // Start menu title image
    sf::Texture startTitleTexture;
    std::optional<sf::Sprite> startTitleSprite;
    bool startTitleLoaded = false;
    if (startTitleTexture.loadFromFile("images/start-menu-title.png"))
    {
        startTitleSprite = sf::Sprite(startTitleTexture);
        sf::Vector2u textureSize = startTitleTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 400.f / static_cast<float>(textureSize.x);
            float scaleY = scaleX; // keep aspect ratio
            startTitleSprite->setScale({scaleX, scaleY});
            float scaledWidth = textureSize.x * scaleX;
            float xPos = (800.f - scaledWidth) / 2.f;
            startTitleSprite->setPosition({xPos, 20.f});
        }
        startTitleLoaded = true;
    }

    sf::Text titleText(font);
    titleText.setString("KLIKER - HUTA");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    {
        sf::FloatRect b = titleText.getLocalBounds();
        titleText.setPosition(sf::Vector2f{400.f - b.size.x / 2.f, 100.f - b.size.y / 2.f});
    }

    sf::Text playText(font);
    playText.setString("Play");
    playText.setCharacterSize(28);
    playText.setFillColor(sf::Color::White);
    {
        sf::FloatRect b = playText.getLocalBounds();
        playText.setPosition(sf::Vector2f{playButton.getPosition().x + playButton.getSize().x / 2.f - b.size.x / 2.f,
                     playButton.getPosition().y + playButton.getSize().y / 2.f - b.size.y / 2.f});
    }

    sf::Text quitText(font);
    quitText.setString("Quit");
    quitText.setCharacterSize(28);
    quitText.setFillColor(sf::Color::White);
    {
        sf::FloatRect b = quitText.getLocalBounds();
        quitText.setPosition(sf::Vector2f{quitButton.getPosition().x + quitButton.getSize().x / 2.f - b.size.x / 2.f,
                     quitButton.getPosition().y + quitButton.getSize().y / 2.f - b.size.y / 2.f});
    }

    // --- STATS / SESSION TRACKING ---
    sf::Clock sessionClock;
    long long totalCollectedIron = 0;
    long long totalMeltedIron = 0;
    long long totalEarnedMoney = 0;
    long long totalSpentMoney = 0;
    int maxLevel = level;
    long long prevCollectedIron = collectedIron;
    long long prevSteel = steel;
    int prevMoney = money;

    bool statsOpen = false;
    bool pausedMenu = false;
    Location menuReturnLocation = Location::Mine;
    sf::RectangleShape statsPanel({560.f, 360.f});
    statsPanel.setPosition({120.f, 100.f});
    statsPanel.setFillColor(sf::Color(20, 20, 30));
    statsPanel.setOutlineColor(sf::Color::White);
    statsPanel.setOutlineThickness(2.f);

    sf::Text statsTitle(font, "Session Stats", 32);
    statsTitle.setFillColor(sf::Color::White);
    statsTitle.setPosition({140.f, 110.f});

    sf::Text statsLines[6] = { sf::Text(font), sf::Text(font), sf::Text(font), sf::Text(font), sf::Text(font), sf::Text(font) };
    for (int i=0;i<6;i++) { statsLines[i].setFillColor(sf::Color::White); statsLines[i].setCharacterSize(20); }

    sf::RectangleShape backButton({200.f, 50.f});
    backButton.setPosition({300.f, 420.f});
    backButton.setFillColor(sf::Color(100,100,100));
    sf::Text backText(font, "Back", 24);
    backText.setFillColor(sf::Color::White);
    sf::FloatRect bt = backText.getLocalBounds();
    backText.setPosition(sf::Vector2f{backButton.getPosition().x + backButton.getSize().x/2.f - bt.size.x/2.f,
                         backButton.getPosition().y + backButton.getSize().y/2.f - bt.size.y/2.f});

    // Initialize location modules
    initMine(font);
    initIronworks(font);
    initStocks(font);

    // Start mine music if starting location is Mine
    if (currentLocation == Location::Mine)
        playMineMusic();

    sf::Clock frameClock;

    while (window.isOpen())
    {
        float deltaTime = frameClock.restart().asSeconds();
        auto mouse = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos(mouse.x, mouse.y);
        
        // Get keyboard input (A and D keys)
        bool keyA = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
        bool keyD = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);

        // --- HOVER DETECTION ---
        mineText.setFillColor(mineTextDefaultColor);
        furnaceText.setFillColor(furnaceTextDefaultColor);
        marketText.setFillColor(marketTextDefaultColor);

        // Apply hover effect to map button (scale up slightly, only when not on map or menu)
        if (currentLocation != Location::Map && currentLocation != Location::StartMenu && mapButtonSprite.has_value() && mapButton.getGlobalBounds().contains(mousePos))
        {
            sf::Vector2u textureSize = mapButtonTexture.getSize();
            if (textureSize.x > 0 && textureSize.y > 0)
            {
                float scaleX = 100.f / static_cast<float>(textureSize.x);
                float scaleY = 40.f / static_cast<float>(textureSize.y);
                mapButtonSprite->setScale({scaleX * 1.05f, scaleY * 1.05f});
            }
        }
        else if (mapButtonSprite.has_value())
        {
            // Reset scale
            sf::Vector2u textureSize = mapButtonTexture.getSize();
            if (textureSize.x > 0 && textureSize.y > 0)
            {
                float scaleX = 100.f / static_cast<float>(textureSize.x);
                float scaleY = 40.f / static_cast<float>(textureSize.y);
                mapButtonSprite->setScale({scaleX, scaleY});
            }
        }

        // Start menu hover: prefer image buttons, fallback to rectangle coloring
        if (currentLocation == Location::StartMenu)
        {
            // Play button hover
            if (playBtnSprite.has_value())
            {
                if (playBtnSprite->getGlobalBounds().contains(mousePos))
                    playBtnSprite->setScale({playBtnBaseScale.x * 1.05f, playBtnBaseScale.y * 1.05f});
                else
                    playBtnSprite->setScale(playBtnBaseScale);
            }
            else
            {
                if (playButton.getGlobalBounds().contains(mousePos))
                    playButton.setFillColor(sf::Color(150,150,150));
                else
                    playButton.setFillColor(sf::Color(100,100,100));
            }

            // Quit button hover
            if (quitBtnSprite.has_value())
            {
                if (quitBtnSprite->getGlobalBounds().contains(mousePos))
                    quitBtnSprite->setScale({quitBtnBaseScale.x * 1.05f, quitBtnBaseScale.y * 1.05f});
                else
                    quitBtnSprite->setScale(quitBtnBaseScale);
            }
            else
            {
                if (quitButton.getGlobalBounds().contains(mousePos))
                    quitButton.setFillColor(sf::Color(150,150,150));
                else
                    quitButton.setFillColor(sf::Color(100,100,100));
            }

            // Stats button hover
            if (statsBtnSprite.has_value())
            {
                if (statsBtnSprite->getGlobalBounds().contains(mousePos))
                    statsBtnSprite->setScale({statsBtnBaseScale.x * 1.05f, statsBtnBaseScale.y * 1.05f});
                else
                    statsBtnSprite->setScale(statsBtnBaseScale);
            }
        }

        // Update menu background panning when on Start Menu
        if (currentLocation == Location::StartMenu && menuBgSprite.has_value())
        {
            if (menuPanMinX < menuPanMaxX)
            {
                menuPanPosX += menuPanDir * menuPanSpeed * deltaTime;
                if (menuPanPosX <= menuPanMinX)
                {
                    menuPanPosX = menuPanMinX;
                    menuPanDir = 1;
                }
                else if (menuPanPosX >= menuPanMaxX)
                {
                    menuPanPosX = menuPanMaxX;
                    menuPanDir = -1;
                }
                menuBgSprite->setPosition({menuPanPosX, 0.f});
            }
            else
            {
                menuBgSprite->setPosition({menuPanMinX, 0.f});
            }
        }

        // Update location-specific UI
        if (currentLocation == Location::Mine)
        {
            updateMine(mousePos, money, deltaTime, keyA, keyD, collectedIron, xp);
        }
        else if (currentLocation == Location::Furnace)
        {
            updateIronworks(mousePos, collectedIron, steel, money);
        }
        else if (currentLocation == Location::Market)
        {
            updateStocks(mousePos, steel, money);
        }

        // Apply hover effects to map location texts when on map
        if (currentLocation == Location::Map)
        {
            if (mineText.getGlobalBounds().contains(mousePos))
                mineText.setFillColor(hoverColor);

            if (furnaceText.getGlobalBounds().contains(mousePos))
                furnaceText.setFillColor(hoverColor);

            if (marketText.getGlobalBounds().contains(mousePos))
                marketText.setFillColor(hoverColor);
        }

        Location prevLocation = currentLocation;
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::KeyPressed>())
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                {
                    // Toggle pause menu: open as StartMenu with resume; store return location
                    if (currentLocation != Location::StartMenu)
                    {
                        menuReturnLocation = currentLocation;
                        currentLocation = Location::StartMenu;
                        pausedMenu = true;
                        statsOpen = false;
                    }
                    else
                    {
                        // If already on StartMenu and it was the paused menu, resume
                        if (pausedMenu)
                        {
                            currentLocation = menuReturnLocation;
                            pausedMenu = false;
                            statsOpen = false;
                        }
                    }
                }
            }

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                // If on Start Menu, handle Play/Quit and skip other clicks
                if (currentLocation == Location::StartMenu)
                {
                    // If stats overlay is open, check back button
                    if (statsOpen)
                    {
                        if (backButton.getGlobalBounds().contains(mousePos))
                        {
                            statsOpen = false;
                            continue;
                        }
                        // Ignore other clicks while stats open
                        continue;
                    }

                    bool handled = false;
                    if (playBtnSprite.has_value())
                    {
                        if (playBtnSprite->getGlobalBounds().contains(mousePos))
                        {
                            if (pausedMenu)
                            {
                                currentLocation = menuReturnLocation;
                                pausedMenu = false;
                            }
                            else
                            {
                                currentLocation = Location::Mine;
                            }
                            handled = true;
                        }
                    }
                    else if (playButton.getGlobalBounds().contains(mousePos))
                    {
                        if (pausedMenu)
                        {
                            currentLocation = menuReturnLocation;
                            pausedMenu = false;
                        }
                        else
                        {
                            currentLocation = Location::Mine;
                        }
                        handled = true;
                    }

                    if (!handled)
                    {
                        if (quitBtnSprite.has_value())
                        {
                            if (quitBtnSprite->getGlobalBounds().contains(mousePos))
                            {
                                window.close();
                                handled = true;
                            }
                        }
                        else if (quitButton.getGlobalBounds().contains(mousePos))
                        {
                            window.close();
                            handled = true;
                        }
                    }

                    // stats button opens stats overlay
                    if (!handled && statsBtnSprite.has_value() && statsBtnSprite->getGlobalBounds().contains(mousePos))
                    {
                        statsOpen = true;
                        handled = true;
                    }

                    if (handled) continue;
                }

                // Click MAPA button
                if (currentLocation != Location::Map && currentLocation != Location::StartMenu && mapButton.getGlobalBounds().contains(mousePos))
                {
                    currentLocation = Location::Map;
                }

                // Map click
                if (currentLocation == Location::Map)
                {
                    if (mineText.getGlobalBounds().contains(mousePos))
                    {
                        currentLocation = Location::Mine;
                    }
                    else if (furnaceText.getGlobalBounds().contains(mousePos))
                    {
                        currentLocation = Location::Furnace;
                    }
                    else if (marketText.getGlobalBounds().contains(mousePos))
                    {
                        currentLocation = Location::Market;
                }
            }

                // Handle location-specific clicks
                if (currentLocation == Location::Mine)
                {
                    handleMineClick(mousePos, collectedIron, money, xp);
                }
                else if (currentLocation == Location::Furnace)
                {
                    handleIronworksClick(mousePos, collectedIron, steel, money);
                }
                else if (currentLocation == Location::Market)
                {
                    handleStocksClick(mousePos, steel, money);
                }
            }
        }

        // If location changed, start/stop location music accordingly
        if (prevLocation != currentLocation)
        {
            if (prevLocation == Location::Mine && currentLocation != Location::Mine)
                stopMineMusic();
            if (prevLocation == Location::Furnace && currentLocation != Location::Furnace)
                stopIronworksMusic();
            if (prevLocation == Location::Market && currentLocation != Location::Market)
                stopStocksMusic();

            if (currentLocation == Location::Mine && prevLocation != Location::Mine)
                playMineMusic();
            if (currentLocation == Location::Furnace && prevLocation != Location::Furnace)
                playIronworksMusic();
            if (currentLocation == Location::Market && prevLocation != Location::Market)
                playStocksMusic();
        }

        // Check for level up
        int xpNeeded = getXPForNextLevel(level);
        while (xp >= xpNeeded && level < 100)
        {
            xp -= xpNeeded;
            level++;
            xpNeeded = getXPForNextLevel(level);
        }

        // --- UPDATE TEXT ---
        int nextLevelXP = getXPForNextLevel(level);
        statsLevelText.setString("Level: " + std::to_string(level) + " (" + std::to_string(xp) + "/" + std::to_string(nextLevelXP) + ")");
        statsIronText.setString(": " + std::to_string(collectedIron) + " kg");
        statsSteelText.setString(": " + std::to_string(steel) + " kg");
        statsMoneyText.setString(": " + std::to_string(money));

        // --- UPDATE SESSION STATS
        if (collectedIron > prevCollectedIron)
        {
            totalCollectedIron += (collectedIron - prevCollectedIron);
        }
        if (steel > prevSteel)
        {
            long long steelGained = steel - prevSteel;
            totalMeltedIron += steelGained * 10;
        }
        int deltaMoney = money - prevMoney;
        if (deltaMoney > 0) totalEarnedMoney += deltaMoney;
        else if (deltaMoney < 0) totalSpentMoney += -deltaMoney;
        if (level > maxLevel) maxLevel = level;
        // update prev snapshots
        prevCollectedIron = collectedIron;
        prevSteel = steel;
        prevMoney = money;


        window.clear(sf::Color(30, 30, 30));

        if (currentLocation == Location::StartMenu)
        {
            if (menuBgSprite.has_value())
            {
                window.draw(*menuBgSprite);
            }
            else if (furnaceBgSprite.has_value())
            {
                window.draw(*furnaceBgSprite);
            }
            if (startTitleSprite.has_value())
                window.draw(*startTitleSprite);
            else
                window.draw(titleText);
            // If stats overlay is open render it instead of the menu buttons
            if (statsOpen)
            {
                window.draw(statsPanel);
                window.draw(statsTitle);

                float sx = statsPanel.getPosition().x + 20.f;
                float sy = statsPanel.getPosition().y + 60.f;
                float lineH = 36.f;
                float minutes = sessionClock.getElapsedTime().asSeconds() / 60.f;
                statsLines[0].setString("Time played: " + std::to_string((int)minutes) + " min");
                statsLines[1].setString("Achieved level: " + std::to_string(maxLevel));
                statsLines[2].setString("Total collected iron: " + std::to_string(totalCollectedIron) + " kg");
                statsLines[3].setString("Total melted iron: " + std::to_string(totalMeltedIron) + " kg");
                statsLines[4].setString("Total earned money: $" + std::to_string(totalEarnedMoney));
                statsLines[5].setString("Total spent money: $" + std::to_string(totalSpentMoney));

                for (int i=0;i<6;i++) { statsLines[i].setPosition({sx, sy + i * lineH}); window.draw(statsLines[i]); }
                window.draw(backButton);
                window.draw(backText);
            }
            else
            {
                // Draw image buttons if available, otherwise draw rectangles+text
                if (pausedMenu)
                {
                    if (playBtnSprite.has_value()) window.draw(*playBtnSprite);
                    else { window.draw(playButton); }
                }
                else
                {
                    if (playBtnSprite.has_value()) window.draw(*playBtnSprite);
                    else { window.draw(playButton); window.draw(playText); }
                }

                if (quitBtnSprite.has_value()) window.draw(*quitBtnSprite);
                else { window.draw(quitButton); window.draw(quitText); }

                if (statsBtnSprite.has_value()) window.draw(*statsBtnSprite);
            }
            window.display();
            continue;
        }

        // Draw background based on current location
        if (currentLocation == Location::Map && mapBgSprite.has_value())
        {
            window.draw(*mapBgSprite);
        }
        else if (currentLocation == Location::Mine && mineBgSprite.has_value())
        {
            window.draw(*mineBgSprite);
        }
        else if (currentLocation == Location::Furnace && furnaceBgSprite.has_value())
        {
            window.draw(*furnaceBgSprite);
        }
        else if (currentLocation == Location::Market && marketBgSprite.has_value())
        {
            window.draw(*marketBgSprite);
        }

        // Draw mainbar at top left (on all locations)
        if (mainbarSprite.has_value())
        {
            window.draw(*mainbarSprite);
        }
        
        window.draw(statsLevelText);
        
        // Draw iron icon and text
        if (ironIconSprite.has_value())
        {
            ironIconSprite->setPosition({statsIconX, statsBasePos.y + statsVSpacing});
            window.draw(*ironIconSprite);
        }
        window.draw(statsIronText);
        
        // Draw steel icon and text
        if (steelIconSprite.has_value())
        {
            steelIconSprite->setPosition({statsIconX, statsBasePos.y + statsVSpacing * 2.f});
            window.draw(*steelIconSprite);
        }
        window.draw(statsSteelText);
        
        // Draw cash icon and text
            if (cashIconSprite.has_value())
            {
                cashIconSprite->setPosition({statsIconX, statsBasePos.y + statsVSpacing * 3.f + 4.f});
                window.draw(*cashIconSprite);
            }
        window.draw(statsMoneyText);
        
        // Draw MAP button
        if (currentLocation != Location::Map && currentLocation != Location::StartMenu && mapButtonSprite.has_value())
        {
            window.draw(*mapButtonSprite);
        }

        if (currentLocation == Location::Map)
        {
            // Draw location icons
            float iconX = 200.f;
            float startY = 150.f;
            float verticalSpacing = 140.f;
            float iconYOffset = (static_cast<float>(mapTextSize) - iconSize) / 2.f; // Center icon vertically with text
            
            if (mineIconSprite.has_value())
            {
                mineIconSprite->setPosition({iconX, startY + iconYOffset});
                window.draw(*mineIconSprite);
            }
            if (furnaceIconSprite.has_value())
            {
                furnaceIconSprite->setPosition({iconX, startY + verticalSpacing + iconYOffset});
                window.draw(*furnaceIconSprite);
            }
            if (marketIconSprite.has_value())
            {
                marketIconSprite->setPosition({iconX, startY + verticalSpacing * 2.f + iconYOffset});
                window.draw(*marketIconSprite);
            }
            
            // Draw location texts
            window.draw(mineText);
            window.draw(furnaceText);
            window.draw(marketText);
        }
        else
        {
            // Draw location text only for non-mine locations
            if (currentLocation != Location::Mine)
        {
            window.draw(locationText);
            }
            
            // Draw location-specific UI
            if (currentLocation == Location::Mine)
            {
                drawMine(window, money);
            }
            else if (currentLocation == Location::Furnace)
            {
                drawIronworks(window, money);
            }
            else if (currentLocation == Location::Market)
            {
                drawStocks(window);
            }
        }

        window.display();
    }
}
