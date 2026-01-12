#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <algorithm>

// 40 Fe -> 4 Steel (30s)
static const int BATCH_1_IRON = 40;
static const float BATCH_1_TIME = 30.0f;

// 80 Fe -> 8 Steel (25s)
static const int BATCH_2_IRON = 80;
static const float BATCH_2_TIME = 25.0f;

// 120 Fe -> 12 Steel (15s)
static const int BATCH_3_IRON = 120;
static const float BATCH_3_TIME = 15.0f;

// work cost removed (no longer charged)
static const int OP_COST = 0;

// UI Elements
static sf::RectangleShape btnSmall({260.f, 60.f});
static sf::RectangleShape btnMedium({260.f, 60.f});
static sf::RectangleShape btnLarge({260.f, 60.f});

static std::optional<sf::Text> txtSmall;
static std::optional<sf::Text> txtMedium;
static std::optional<sf::Text> txtLarge;
static std::optional<sf::Text> infoText;

// Upgrades button and store (copied from mine)
static sf::RectangleShape upgradesButton({100.f, 40.f});
static std::optional<sf::Text> upgradesButtonText;
static sf::Texture upgradeBtnTexture;
static bool upgradeBtnTextureLoaded = false;
static std::optional<sf::Sprite> upgradeBtnSprite;
static sf::Vector2f upgradeBtnOriginalScale = {1.f, 1.f};
static bool storeOpen = false;

// Store UI
static sf::RectangleShape storeBackground({480.f, 440.f});
static sf::RectangleShape storeCloseButton({80.f, 40.f});
static std::optional<sf::Text> storeCloseButtonText;
static std::optional<sf::Text> storeTitleText;

// Efficient Melting upgrade (reduces melting time by 50%)
static bool efficientMeltingPurchased = false;
static sf::RectangleShape efficientMeltingButton({300.f, 60.f});
static std::optional<sf::Text> efficientMeltingButtonText;
static const int efficientMeltingCost = 100;

// HUTA title image at top center
static sf::Texture hutaTitleTexture;
static std::optional<sf::Sprite> hutaTitleSprite;
static bool hutaTitleLoaded = false;

// --- UK FLAG EASTER EGG VARIABLES ---
static sf::Texture ukFlagTexture;
static std::optional<sf::Sprite> ukFlagSprite;
static bool ukFlagLoaded = false;
static bool britishBonusCollected = false;
static std::optional<sf::Text> bonusMessageText;
static sf::Clock bonusMessageTimer;

// Music for ironworks theme
static sf::Music ironworksMusic;
static bool ironworksMusicLoaded = false;
static bool ironworksMusicPlaying = false;

// Logic Variables
static bool isWorking = false;
static sf::Clock workClock;
static float currentTotalTime = 0.0f;
static int currentBatchIron = 0;
static bool ironworksInitDone = false;

void initIronworks(const sf::Font& font)
{
    if (ironworksInitDone) return;

    // Load HUTA title image
    if (!hutaTitleLoaded && hutaTitleTexture.loadFromFile("images/ui-huta.png"))
    {
        hutaTitleSprite.emplace(hutaTitleTexture);
        sf::Vector2u ts = hutaTitleTexture.getSize();
        if (ts.x > 0) {
            float scale = 400.f / (float)ts.x;
            hutaTitleSprite->setScale({scale, scale});
            hutaTitleSprite->setPosition({(800.f - ts.x * scale) / 2.f, 20.f});
        }
        hutaTitleLoaded = true;
    }

    // uk flag init
    if (!ukFlagLoaded) {
        if (ukFlagTexture.loadFromFile("images/uk-flag.png")) {
            ukFlagSprite.emplace(ukFlagTexture);
            // Scale to approx 60x30 pixels
            sf::Vector2u ts = ukFlagTexture.getSize();
            if (ts.x > 0 && ts.y > 0) {
                ukFlagSprite->setScale({60.f / ts.x, 30.f / ts.y});
            }
            // Position under the Map button (Map button is approx at 690, 10, height 40)
            if (ukFlagSprite) ukFlagSprite->setPosition({710.f, 140.f});
        }
        ukFlagLoaded = true;
    }

    if (!bonusMessageText.has_value()) {
        bonusMessageText.emplace(font, "Zachowales sie jak Anglicy, otrzymujesz 50$", 20);
        bonusMessageText->setFillColor(sf::Color::Green);
        bonusMessageText->setOutlineColor(sf::Color::Black);
        bonusMessageText->setOutlineThickness(1.f);
        // Center the message roughly
        bonusMessageText->setPosition({180.f, 530.f});
    }

    // Load ironworks theme music (try a few common locations)
    if (!ironworksMusicLoaded)
    {
        try
        {
            bool opened = false;
            // Try local file first (most common)
            opened = ironworksMusic.openFromFile("ironworks-theme.wav");
            if (!opened) opened = ironworksMusic.openFromFile("sounds/ironworks-theme.wav");
            if (!opened) opened = ironworksMusic.openFromFile("audio/ironworks-theme.wav");

            if (opened)
            {
                ironworksMusicLoaded = true;
                ironworksMusic.setLooping(true);
                // Do not auto-play here; play when entering ironworks location
                ironworksMusicPlaying = false;
            }
            else
            {
                std::cerr << "ironworks: failed to open music file from tried paths\n";
            }
        }
        catch (const std::exception&)
        {
            ironworksMusicLoaded = false;
        }
    }

    float startX = 270.f;
    float startY = 180.f; // Przesunięte nieco wyżej
    float gap = 85.f;     // Odstęp między przyciskami

    // Button 1
    btnSmall.setPosition({startX, startY});
    btnSmall.setOutlineThickness(2.f);
    txtSmall.emplace(font, "Maly (40 Fe -> 30s)", 20);
    txtSmall->setPosition({startX + 20.f, startY + 18.f});

    // Button 2
    btnMedium.setPosition({startX, startY + gap});
    btnMedium.setOutlineThickness(2.f);
    txtMedium.emplace(font, "Sredni (80 Fe -> 25s)", 20);
    txtMedium->setPosition({startX + 20.f, startY + gap + 18.f});

    // Button 3
    btnLarge.setPosition({startX, startY + gap * 2});
    btnLarge.setOutlineThickness(2.f);
    txtLarge.emplace(font, "Duzy (120 Fe -> 15s)", 20);
    txtLarge->setPosition({startX + 20.f, startY + gap * 2 + 18.f});

    // Info text at bottom
    infoText.emplace(font, "Wybierz opcje.", 24);
    infoText->setFillColor(sf::Color::Yellow);
    infoText->setPosition({240.f, 460.f});

    // Upgrades button (below MAPA button)
    upgradesButton.setFillColor(sf::Color(80, 80, 80));
    upgradesButton.setPosition({690.f, 70.f});

    // Try to load image for upgrades button
    if (!upgradeBtnTextureLoaded && upgradeBtnTexture.loadFromFile("images/upgrade-btn-icon.png"))
    {
        upgradeBtnTextureLoaded = true;
        upgradeBtnSprite = sf::Sprite(upgradeBtnTexture);
        sf::Vector2u ts = upgradeBtnTexture.getSize();
        if (ts.x > 0 && ts.y > 0)
        {
            float desiredSize = 72.f;
            float scale = desiredSize / static_cast<float>(std::max(ts.x, ts.y));
            upgradeBtnOriginalScale = {scale, scale};
            upgradeBtnSprite->setScale(upgradeBtnOriginalScale);
            upgradeBtnSprite->setOrigin({ts.x / 2.f, ts.y / 2.f});
        }
        sf::Vector2f rectPos = upgradesButton.getPosition();
        sf::Vector2f rectSize = upgradesButton.getSize();
        upgradeBtnSprite->setPosition({rectPos.x + rectSize.x / 2.f, rectPos.y + rectSize.y / 2.f});
    }

    // Store UI
    storeBackground.setFillColor(sf::Color(40, 40, 50));
    storeBackground.setOutlineColor(sf::Color::White);
    storeBackground.setOutlineThickness(3.f);
    storeBackground.setPosition({160.f, 70.f});

    storeCloseButton.setFillColor(sf::Color(150, 50, 50));
    storeCloseButton.setPosition({540.f, 80.f});

    storeCloseButtonText = sf::Text(font, "X", 24);
    storeCloseButtonText->setFillColor(sf::Color::White);
    storeCloseButtonText->setOutlineColor(sf::Color::Black);
    storeCloseButtonText->setOutlineThickness(1.f);
    storeCloseButtonText->setPosition({565.f, 85.f});

    storeTitleText = sf::Text(font, "STORE", 32);
    storeTitleText->setFillColor(sf::Color::White);
    storeTitleText->setOutlineColor(sf::Color::Black);
    storeTitleText->setOutlineThickness(2.f);
    storeTitleText->setPosition({280.f, 90.f});

    // Efficient Melting upgrade button (reduces melting time by 50%)
    efficientMeltingButton.setFillColor(sf::Color(60, 100, 60));
    efficientMeltingButton.setOutlineColor(sf::Color::White);
    efficientMeltingButton.setOutlineThickness(2.f);
    efficientMeltingButton.setPosition({180.f, 235.f});

    efficientMeltingButtonText = sf::Text(font, "Efficient Melting\nCost: 100$", 18);
    efficientMeltingButtonText->setFillColor(sf::Color::White);
    efficientMeltingButtonText->setOutlineColor(sf::Color::Black);
    efficientMeltingButtonText->setOutlineThickness(1.f);
    efficientMeltingButtonText->setPosition({200.f, 245.f});

    ironworksInitDone = true;
}

// Requires: money for cost check
void updateIronworks(const sf::Vector2f& mousePos, long long& iron, long long& steel, int& money) 
{
    // uk flag effect
    if (ukFlagSprite && !britishBonusCollected) {
        if (ukFlagSprite->getGlobalBounds().contains(mousePos)) {
             // Semi-transparent when hovered
             ukFlagSprite->setColor(sf::Color(255, 255, 255, 200)); 
        } else {
             ukFlagSprite->setColor(sf::Color::White);
        }
    } else if (ukFlagSprite && britishBonusCollected) {
         // Dimmed if already collected
         ukFlagSprite->setColor(sf::Color(100, 100, 100, 100)); 
    }

    // Store hover handling (stop other updates when open)
    if (storeOpen)
    {
        if (storeCloseButton.getGlobalBounds().contains(mousePos))
        {
            storeCloseButton.setFillColor(sf::Color(180, 70, 70));
        }
        else
        {
            storeCloseButton.setFillColor(sf::Color(150, 50, 50));
        }

        if (!efficientMeltingPurchased)
        {
            if (efficientMeltingButton.getGlobalBounds().contains(mousePos))
            {
                efficientMeltingButton.setFillColor(money >= efficientMeltingCost ? sf::Color(80, 130, 80) : sf::Color(100, 60, 60));
            }
            else
            {
                efficientMeltingButton.setFillColor(money >= efficientMeltingCost ? sf::Color(60, 100, 60) : sf::Color(80, 50, 50));
            }
        }

        return;
    }

    // Hover effect for upgrades button (use sprite if available)
    if (upgradeBtnSprite.has_value())
    {
        if (upgradeBtnSprite->getGlobalBounds().contains(mousePos))
        {
            upgradeBtnSprite->setScale({upgradeBtnOriginalScale.x * 1.08f, upgradeBtnOriginalScale.y * 1.08f});
        }
        else
        {
            upgradeBtnSprite->setScale(upgradeBtnOriginalScale);
        }
    }
    else
    {
        if (upgradesButton.getGlobalBounds().contains(mousePos))
        {
            upgradesButton.setFillColor(sf::Color(100, 100, 100));
        }
        else
        {
            upgradesButton.setFillColor(sf::Color(80, 80, 80));
        }
    }

    if (isWorking)
    {
        float elapsed = workClock.getElapsedTime().asSeconds();
        
        if (elapsed >= currentTotalTime)
        {
            // Finish: 120 Fe -> 12 Steel (10% conversion)
            isWorking = false;
            long long steelGained = currentBatchIron / 10; 
            steel += steelGained;

            if (infoText) infoText->setString("Gotowe! Otrzymano " + std::to_string(steelGained) + " Stali");
        }
        else
        {
            // Working: Countdown timer
            float remaining = currentTotalTime - elapsed;
            if (infoText) infoText->setString("Przetapianie... " + std::to_string((int)remaining) + "s");
            
            // Gray out buttons while working
            btnSmall.setFillColor(sf::Color(50, 50, 50));
            btnMedium.setFillColor(sf::Color(50, 50, 50));
            btnLarge.setFillColor(sf::Color(50, 50, 50));
        }
    }
    else
    {
        // check affordability 
        auto updateBtn = [&](sf::RectangleShape& btn, int costIron) {
            bool canAfford = (iron >= costIron);

            if (btn.getGlobalBounds().contains(mousePos)) {
                // Hover effect
                btn.setFillColor(canAfford ? sf::Color(200, 80, 80) : sf::Color(80, 40, 40));
            } else {
                // Normal state
                btn.setFillColor(canAfford ? sf::Color(150, 50, 50) : sf::Color(60, 60, 60));
            }
        };

        updateBtn(btnSmall, BATCH_1_IRON);
        updateBtn(btnMedium, BATCH_2_IRON);
        updateBtn(btnLarge, BATCH_3_IRON);

        // Update button labels to reflect efficient melting (50% time) if purchased
        if (txtSmall) {
            int displayTime = (int)(efficientMeltingPurchased ? (BATCH_1_TIME * 0.5f) : BATCH_1_TIME);
            txtSmall->setString("Maly (" + std::to_string(BATCH_1_IRON) + " Fe -> " + std::to_string(displayTime) + "s)");
        }
        if (txtMedium) {
            int displayTime = (int)(efficientMeltingPurchased ? (BATCH_2_TIME * 0.5f) : BATCH_2_TIME);
            txtMedium->setString("Sredni (" + std::to_string(BATCH_2_IRON) + " Fe -> " + std::to_string(displayTime) + "s)");
        }
        if (txtLarge) {
            int displayTime = (int)(efficientMeltingPurchased ? (BATCH_3_TIME * 0.5f) : BATCH_3_TIME);
            txtLarge->setString("Duzy (" + std::to_string(BATCH_3_IRON) + " Fe -> " + std::to_string(displayTime) + "s)");
        }

        if (infoText) infoText->setString("Wybierz opcje (Koszt startu: $10)");
    }
}

// Handle clicks: Start batch if affordable
bool handleIronworksClick(const sf::Vector2f& mousePos, long long& iron, long long& steel, int& money)
{
    // UK flag click
    if (ukFlagSprite && !britishBonusCollected && ukFlagSprite->getGlobalBounds().contains(mousePos)) {
        money += 50;
        britishBonusCollected = true;
        bonusMessageTimer.restart(); // Start timer to show message
        return true; 
    }

    // Store close button
    if (storeOpen && storeCloseButton.getGlobalBounds().contains(mousePos))
    {
        storeOpen = false;
        return true;
    }

    // Store purchases (Efficient Melting)
    if (storeOpen && !efficientMeltingPurchased && efficientMeltingButton.getGlobalBounds().contains(mousePos))
    {
        if (money >= efficientMeltingCost)
        {
            money -= efficientMeltingCost;
            efficientMeltingPurchased = true;
            efficientMeltingButton.setFillColor(sf::Color(40, 40, 40));
            if (efficientMeltingButtonText.has_value())
            {
                efficientMeltingButtonText->setString("Efficient Melting\nPURCHASED");
            }
            return true;
        }
    }

    // Upgrades button opens store
    if (!storeOpen)
    {
        if (upgradeBtnSprite.has_value() && upgradeBtnSprite->getGlobalBounds().contains(mousePos))
        {
            storeOpen = true;
            return true;
        }
        if (upgradesButton.getGlobalBounds().contains(mousePos))
        {
            storeOpen = true;
            return true;
        }
    }

    if (storeOpen) return false;

    if (isWorking) return false;

    // Helper to process click for a specific button
    auto tryClick = [&](sf::RectangleShape& btn, int ironReq, float timeReq) {
        if (btn.getGlobalBounds().contains(mousePos)) {
            if (iron >= ironReq) {
                // Deduct iron batch (no money fee)
                iron -= ironReq;

                // Start Timer: 30s/25s/15s
                currentBatchIron = ironReq;
                currentTotalTime = efficientMeltingPurchased ? timeReq * 0.5f : timeReq;
                isWorking = true;
                workClock.restart();
                return true;
            }
        }
        return false;
    };

    if (tryClick(btnSmall, BATCH_1_IRON, BATCH_1_TIME)) return true;
    if (tryClick(btnMedium, BATCH_2_IRON, BATCH_2_TIME)) return true;
    if (tryClick(btnLarge, BATCH_3_IRON, BATCH_3_TIME)) return true;

    return false;
}

void drawIronworks(sf::RenderWindow& window, int money)
{
    if (hutaTitleSprite) window.draw(*hutaTitleSprite);
    
    //UK flag
    if (ukFlagSprite) window.draw(*ukFlagSprite);

    // Easteregg
    if (britishBonusCollected && bonusMessageTimer.getElapsedTime().asSeconds() < 3.0f) {
        if (bonusMessageText) window.draw(*bonusMessageText);
    }

    // Always draw upgrades button
    if (upgradeBtnSprite.has_value())
    {
        window.draw(*upgradeBtnSprite);
    }
    else
    {
        window.draw(upgradesButton);
    }
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

        if (!efficientMeltingPurchased)
        {
            window.draw(efficientMeltingButton);
            if (efficientMeltingButtonText.has_value())
            {
                efficientMeltingButtonText->setFillColor(money >= efficientMeltingCost ? sf::Color::White : sf::Color(200, 150, 150));
                window.draw(*efficientMeltingButtonText);
            }
        }
        else
        {
            window.draw(efficientMeltingButton);
            if (efficientMeltingButtonText.has_value())
            {
                efficientMeltingButtonText->setFillColor(sf::Color(150, 150, 150));
                window.draw(*efficientMeltingButtonText);
            }
        }
    }
    else
    {
        window.draw(btnSmall); if (txtSmall) window.draw(*txtSmall);
        window.draw(btnMedium); if (txtMedium) window.draw(*txtMedium);
        window.draw(btnLarge); if (txtLarge) window.draw(*txtLarge);
        
        if (infoText) window.draw(*infoText);
    }
}

// Music control for ironworks theme
void playIronworksMusic()
{
    if (ironworksMusicLoaded && !ironworksMusicPlaying)
    {
        ironworksMusic.play();
        ironworksMusicPlaying = true;
    }
}

void stopIronworksMusic()
{
    if (ironworksMusicLoaded && ironworksMusicPlaying)
    {
        ironworksMusic.stop();
        ironworksMusicPlaying = false;
    }
}