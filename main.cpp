#include <SFML/Graphics.hpp>
#include <algorithm>
#include <optional>
#include <string>

void initMine(const sf::Font& font);
void updateMine(int mineClicks, const sf::Vector2f& mousePos, int money);
bool handleMineClick(const sf::Vector2f& mousePos, int& mineClicks, long long& collectedIron, int& money, int& xp);
void drawMine(sf::RenderWindow& window, int money);

void initIronworks(const sf::Font& font);
void updateIronworks(const sf::Vector2f& mousePos, long long& iron, long long& steel);
bool handleIronworksClick(const sf::Vector2f& mousePos, long long& iron, long long& steel);
void drawIronworks(sf::RenderWindow& window);

void initStocks(const sf::Font& font);
void updateStocks(const sf::Vector2f& mousePos);
bool handleStocksClick(const sf::Vector2f& mousePos);
void drawStocks(sf::RenderWindow& window);

enum class Location {
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
    // Icon size (larger for better visibility)
    const float iconSize = 120.f;
    const float iconSpacing = 30.f; // Space between icon and text
    
    // Mine icon
    sf::Texture mineIconTexture;
    std::optional<sf::Sprite> mineIconSprite;
    if (mineIconTexture.loadFromFile("images/mine-icon.png"))
    {
        mineIconSprite = sf::Sprite(mineIconTexture);
        // Scale icon to desired size (maintain aspect ratio)
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
    Location currentLocation = Location::Mine;
    int level = 1;
    int xp = 0; // Experience points
    int money = 200; // Will be used later
    int mineClicks = 0;
    long long collectedIron = 0; // Iron collected from mine
    long long steel = 0; // Steel produced from iron

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
        mainbarSprite->setPosition({0.f, 0.f}); // Top left corner
    }

    // --- UI: TOP LEFT (LEVEL + MONEY) ---
    sf::Text statsText(font);
    statsText.setCharacterSize(18);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition({10.f, 10.f});

    // --- UI: MAP BUTTON (TOP RIGHT)
    sf::Texture mapButtonTexture;
    std::optional<sf::Sprite> mapButtonSprite;
    if (mapButtonTexture.loadFromFile("images/ui-mapa.png"))
    {
        mapButtonSprite = sf::Sprite(mapButtonTexture);
        mapButtonSprite->setPosition({690.f, 10.f});
        // Scale to appropriate button size (100x40)
        sf::Vector2u textureSize = mapButtonTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 100.f / static_cast<float>(textureSize.x);
            float scaleY = 40.f / static_cast<float>(textureSize.y);
            mapButtonSprite->setScale({scaleX, scaleY});
        }
    }
    
    // Keep old button for bounds checking (invisible)
    sf::RectangleShape mapButton({100.f, 40.f});
    mapButton.setPosition({690.f, 10.f});

    // --- CENTER TEXT (WHERE YOU ARE) ---
    sf::Text locationText(font);
    locationText.setCharacterSize(32);
    locationText.setFillColor(sf::Color::White);
    locationText.setPosition({250.f, 250.f});

    // --- MAP BUTTONS ---
    // Larger text for map screen
    const unsigned int mapTextSize = 52;
    sf::Text mineText(font, "KOPALNIA", mapTextSize);
    sf::Text furnaceText(font, "HUTA", mapTextSize);
    sf::Text marketText(font, "GIELDA", mapTextSize);

    // Text positions (centered and spaced out to use more screen)
    float textStartX = 200.f + iconSize + iconSpacing;
    float verticalSpacing = 140.f; // More space between items
    float startY = 150.f; // Start higher up
    mineText.setPosition({textStartX, startY});
    furnaceText.setPosition({textStartX, startY + verticalSpacing});
    marketText.setPosition({textStartX, startY + verticalSpacing * 2.f});

    // Default colors for hover effects
    sf::Color mineTextDefaultColor = sf::Color::White;
    sf::Color furnaceTextDefaultColor = sf::Color::White;
    sf::Color marketTextDefaultColor = sf::Color::White;
    sf::Color hoverColor = sf::Color(255, 200, 100); // Light orange/yellow for hover

    // Initialize location modules
    initMine(font);
    initIronworks(font);
    initStocks(font);

    while (window.isOpen())
    {
        // Get mouse position for hover detection
        auto mouse = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos(mouse.x, mouse.y);

        // --- HOVER DETECTION ---
        // Reset text colors to default
        mineText.setFillColor(mineTextDefaultColor);
        furnaceText.setFillColor(furnaceTextDefaultColor);
        marketText.setFillColor(marketTextDefaultColor);

        // Apply hover effect to map button (scale up slightly, only when not on map)
        if (currentLocation != Location::Map && mapButtonSprite.has_value() && mapButton.getGlobalBounds().contains(mousePos))
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

        // Update location-specific UI
        if (currentLocation == Location::Mine)
        {
            updateMine(mineClicks, mousePos, money);
        }
        else if (currentLocation == Location::Furnace)
        {
            updateIronworks(mousePos, collectedIron, steel);
        }
        else if (currentLocation == Location::Market)
        {
            updateStocks(mousePos);
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

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                // Click MAPA button (only when not on map)
                if (currentLocation != Location::Map && mapButton.getGlobalBounds().contains(mousePos))
                {
                    currentLocation = Location::Map;
                }

                // Map click - lokzalization change
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
                    handleMineClick(mousePos, mineClicks, collectedIron, money, xp);
                }
                else if (currentLocation == Location::Furnace)
                {
                    handleIronworksClick(mousePos, collectedIron, steel);
                }
                else if (currentLocation == Location::Market)
                {
                    handleStocksClick(mousePos);
                }
            }
        }

        // Check for level up
        int xpNeeded = getXPForNextLevel(level);
        while (xp >= xpNeeded && level < 100) // Cap at level 100
        {
            xp -= xpNeeded;
            level++;
            xpNeeded = getXPForNextLevel(level);
        }

        // --- UPDATE TEXT ---
        int nextLevelXP = getXPForNextLevel(level);
        statsText.setString(
            "Level: " + std::to_string(level) + " (" + std::to_string(xp) + "/" + std::to_string(nextLevelXP) + ")" +
            "\nIron: " + std::to_string(collectedIron) + " kg" +
            "\nSteel: " + std::to_string(steel) + " kg" +
            "\n$ " + std::to_string(money)
        );

        switch (currentLocation)
        {
            case Location::Mine:
                locationText.setString("KOPALNIA");
                break;
            case Location::Furnace:
                locationText.setString("HUTA");
                break;
            case Location::Market:
                locationText.setString("GIELDA");
                break;
            case Location::Map:
                locationText.setString("MAPA");
                break;
        }

        window.clear(sf::Color(30, 30, 30));

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

        // UI zawsze
        // Draw mainbar at top left (on all locations)
        if (mainbarSprite.has_value())
        {
            window.draw(*mainbarSprite);
        }
        
        window.draw(statsText);
        
        // Draw MAP button (using ui-huta.png image) only when NOT on map
        if (currentLocation != Location::Map && mapButtonSprite.has_value())
        {
            window.draw(*mapButtonSprite);
        }

        if (currentLocation == Location::Map)
        {
            // Draw location icons (to the left of text)
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
                drawIronworks(window);
            }
            else if (currentLocation == Location::Market)
            {
                drawStocks(window);
            }
        }

        window.display();
    }
}
