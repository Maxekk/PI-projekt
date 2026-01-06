#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <string>
#include <vector>

// Mine location UI elements
static sf::RectangleShape mineButton({200.f, 80.f});
static std::optional<sf::Text> mineButtonText;

// Upgrades button and store
static sf::RectangleShape upgradesButton({100.f, 40.f});
static std::optional<sf::Text> upgradesButtonText;
static bool storeOpen = false;

// Store UI
static sf::RectangleShape storeBackground({600.f, 400.f});
static sf::RectangleShape storeCloseButton({80.f, 40.f});
static std::optional<sf::Text> storeCloseButtonText;
static std::optional<sf::Text> storeTitleText;

// Double Click upgrade
static bool doubleClickPurchased = false;
static sf::RectangleShape doubleClickButton({250.f, 60.f});
static std::optional<sf::Text> doubleClickButtonText;
static const int doubleClickCost = 100;

// Sound for mine click
static sf::SoundBuffer pickaxeSoundBuffer;
static bool soundLoaded = false;
static std::vector<sf::Sound> activeSounds; // Keep sounds alive while playing

void initMine(const sf::Font& font)
{
    if (mineButtonText.has_value()) return;
    
    // TODO: BUG AFTER 5 CLICKS SOUNDS GLITCHES
    // Load pickaxe sound
    if (!soundLoaded)
    {
        // Try loading from current directory first
        if (pickaxeSoundBuffer.loadFromFile("pickaxe-mine.wav"))
        {
            // Verify the buffer actually loaded data
            if (pickaxeSoundBuffer.getSampleCount() > 0)
            {
                soundLoaded = true;
            }
        }
    }
    
    mineButton.setFillColor(sf::Color(100, 100, 150));
    mineButton.setPosition({300.f, 350.f});
    mineButton.setOutlineColor(sf::Color::White);
    mineButton.setOutlineThickness(2.f);

    mineButtonText = sf::Text(font, "Kop", 32);
    mineButtonText->setFillColor(sf::Color::White);
    mineButtonText->setOutlineColor(sf::Color::Black);
    mineButtonText->setOutlineThickness(1.f);
    mineButtonText->setPosition({350.f, 365.f});

    // Upgrades button (below MAPA button at 690, 10, so below it at 690, 60)
    upgradesButton.setFillColor(sf::Color(80, 80, 80));
    upgradesButton.setPosition({690.f, 60.f});

    upgradesButtonText = sf::Text(font, "UPGRADES", 16);
    upgradesButtonText->setFillColor(sf::Color::White);
    upgradesButtonText->setOutlineColor(sf::Color::Black);
    upgradesButtonText->setOutlineThickness(1.f);
    upgradesButtonText->setPosition({695.f, 68.f});

    // Store UI
    storeBackground.setFillColor(sf::Color(40, 40, 50));
    storeBackground.setOutlineColor(sf::Color::White);
    storeBackground.setOutlineThickness(3.f);
    storeBackground.setPosition({100.f, 100.f}); // Centered-ish

    storeCloseButton.setFillColor(sf::Color(150, 50, 50));
    storeCloseButton.setPosition({610.f, 110.f}); // Top right of store

    storeCloseButtonText = sf::Text(font, "X", 24);
    storeCloseButtonText->setFillColor(sf::Color::White);
    storeCloseButtonText->setOutlineColor(sf::Color::Black);
    storeCloseButtonText->setOutlineThickness(1.f);
    storeCloseButtonText->setPosition({635.f, 115.f});

    storeTitleText = sf::Text(font, "STORE", 32);
    storeTitleText->setFillColor(sf::Color::White);
    storeTitleText->setOutlineColor(sf::Color::Black);
    storeTitleText->setOutlineThickness(2.f);
    storeTitleText->setPosition({350.f, 120.f});

    // Double Click upgrade button
    doubleClickButton.setFillColor(sf::Color(60, 100, 60));
    doubleClickButton.setOutlineColor(sf::Color::White);
    doubleClickButton.setOutlineThickness(2.f);
    doubleClickButton.setPosition({175.f, 180.f});

    doubleClickButtonText = sf::Text(font, "Double Click\nCost: 100$", 20);
    doubleClickButtonText->setFillColor(sf::Color::White);
    doubleClickButtonText->setOutlineColor(sf::Color::Black);
    doubleClickButtonText->setOutlineThickness(1.f);
    doubleClickButtonText->setPosition({200.f, 190.f});
}

void updateMine(int mineClicks, const sf::Vector2f& mousePos, int money)
{
    // Hover effect for mine button
    if (!storeOpen)
    {
        if (mineButton.getGlobalBounds().contains(mousePos))
        {
            mineButton.setFillColor(sf::Color(120, 120, 180));
        }
        else
        {
            mineButton.setFillColor(sf::Color(100, 100, 150));
        }
    }

    // Hover effect for upgrades button
    if (upgradesButton.getGlobalBounds().contains(mousePos))
    {
        upgradesButton.setFillColor(sf::Color(100, 100, 100));
    }
    else
    {
        upgradesButton.setFillColor(sf::Color(80, 80, 80));
    }

    // Hover effect for store close button
    if (storeOpen && storeCloseButton.getGlobalBounds().contains(mousePos))
    {
        storeCloseButton.setFillColor(sf::Color(180, 70, 70));
    }
    else if (storeOpen)
    {
        storeCloseButton.setFillColor(sf::Color(150, 50, 50));
    }

    // Hover effect for double click upgrade button
    if (storeOpen && !doubleClickPurchased)
    {
        if (doubleClickButton.getGlobalBounds().contains(mousePos))
        {
            if (money >= doubleClickCost)
            {
                doubleClickButton.setFillColor(sf::Color(80, 130, 80));
            }
            else
            {
                doubleClickButton.setFillColor(sf::Color(100, 60, 60));
            }
        }
        else
        {
            if (money >= doubleClickCost)
            {
                doubleClickButton.setFillColor(sf::Color(60, 100, 60));
            }
            else
            {
                doubleClickButton.setFillColor(sf::Color(80, 50, 50));
            }
        }
    }
}

<<<<<<< Updated upstream
bool handleMineClick(const sf::Vector2f& mousePos, int& mineClicks, long long& iron)
=======
bool handleMineClick(const sf::Vector2f& mousePos, int& mineClicks, int& collectedIron, int& money, int& xp)
>>>>>>> Stashed changes
{
    // Handle store close button
    if (storeOpen && storeCloseButton.getGlobalBounds().contains(mousePos))
    {
        storeOpen = false;
        return true;
    }

    // Handle double click upgrade purchase
    if (storeOpen && !doubleClickPurchased && doubleClickButton.getGlobalBounds().contains(mousePos))
    {
        if (money >= doubleClickCost)
        {
            money -= doubleClickCost;
            doubleClickPurchased = true;
            doubleClickButton.setFillColor(sf::Color(40, 40, 40));
            if (doubleClickButtonText.has_value())
            {
                doubleClickButtonText->setString("Double Click\nPURCHASED");
            }
            return true;
        }
    }

    // Handle upgrades button
    if (!storeOpen && upgradesButton.getGlobalBounds().contains(mousePos))
    {
        storeOpen = true;
        return true;
    }

    // Handle mine button (only when store is closed)
    if (!storeOpen && mineButton.getGlobalBounds().contains(mousePos))
    {
        mineClicks++;
<<<<<<< Updated upstream
        iron++;
=======
        xp++; // Each click gives 1 XP
        
        // Play pickaxe sound
        if (soundLoaded && pickaxeSoundBuffer.getSampleCount() > 0)
        {
            // Create new sound instance and play it
            activeSounds.emplace_back(pickaxeSoundBuffer);
            sf::Sound& newSound = activeSounds.back();
            newSound.setVolume(100.f); // Ensure volume is set
            newSound.play();
            
            // Keep only last 5 sounds to avoid memory issues
            if (activeSounds.size() > 5)
            {
                activeSounds.erase(activeSounds.begin());
            }
        }
        
        if (doubleClickPurchased)
        {
            collectedIron += 2; // Double click upgrade active
        }
        else
        {
            collectedIron++; // Normal click
        }
>>>>>>> Stashed changes
        return true;
    }
    return false;
}

void drawMine(sf::RenderWindow& window, int money)
{
    // Always draw upgrades button
    window.draw(upgradesButton);
    if (upgradesButtonText.has_value())
        window.draw(*upgradesButtonText);

    // Draw store if open
    if (storeOpen)
    {
        window.draw(storeBackground);
        if (storeTitleText.has_value())
            window.draw(*storeTitleText);
        window.draw(storeCloseButton);
        if (storeCloseButtonText.has_value())
            window.draw(*storeCloseButtonText);
        
        // Draw double click upgrade button
        if (!doubleClickPurchased)
        {
            window.draw(doubleClickButton);
            if (doubleClickButtonText.has_value())
            {
                // Update text color based on affordability
                if (money >= doubleClickCost)
                {
                    doubleClickButtonText->setFillColor(sf::Color::White);
                }
                else
                {
                    doubleClickButtonText->setFillColor(sf::Color(200, 150, 150));
                }
                window.draw(*doubleClickButtonText);
            }
        }
        else
        {
            // Draw purchased upgrade (grayed out)
            window.draw(doubleClickButton);
            if (doubleClickButtonText.has_value())
            {
                doubleClickButtonText->setFillColor(sf::Color(150, 150, 150));
                window.draw(*doubleClickButtonText);
            }
        }
    }
    else
    {
        // Draw mine button only when store is closed
        window.draw(mineButton);
        if (mineButtonText.has_value())
            window.draw(*mineButtonText);
    }
}

