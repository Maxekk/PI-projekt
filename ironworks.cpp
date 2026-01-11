#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <string>

// 40 Fe -> 4 Steel (30s)
static const int BATCH_1_IRON = 40;
static const float BATCH_1_TIME = 30.0f;

// 80 Fe -> 8 Steel (25s)
static const int BATCH_2_IRON = 80;
static const float BATCH_2_TIME = 25.0f;

// 120 Fe -> 12 Steel (15s)
static const int BATCH_3_IRON = 120;
static const float BATCH_3_TIME = 15.0f;

// work cost -> 10
static const int OP_COST = 10;

// UI Elements
static sf::RectangleShape btnSmall({260.f, 60.f});
static sf::RectangleShape btnMedium({260.f, 60.f});
static sf::RectangleShape btnLarge({260.f, 60.f});

static std::optional<sf::Text> txtSmall;
static std::optional<sf::Text> txtMedium;
static std::optional<sf::Text> txtLarge;
static std::optional<sf::Text> infoText;

// HUTA title image at top center
static sf::Texture hutaTitleTexture;
static std::optional<sf::Sprite> hutaTitleSprite;
static bool hutaTitleLoaded = false;

// --- UK FLAG EASTER EGG VARIABLES ---
static sf::Texture ukFlagTexture;
static std::optional<sf::Sprite> ukFlagSprite;
static bool ukFlagLoaded = false;
static bool britishBonusCollected = false; // Has the bonus been claimed?
static std::optional<sf::Text> bonusMessageText;
static sf::Clock bonusMessageTimer; // Timer to hide the message

// Logic Variables
static bool isWorking = false;
static sf::Clock workClock;
static float currentTotalTime = 0.0f;
static int currentBatchIron = 0;

void initIronworks(const sf::Font& font)
{
    if (txtSmall.has_value() && hutaTitleLoaded && ukFlagLoaded) return;

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
            if (ukFlagSprite) ukFlagSprite->setPosition({710.f, 60.f});
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
    infoText.emplace(font, "Koszt startu: $10. Wybierz wsad.", 24);
    infoText->setFillColor(sf::Color::Yellow);
    infoText->setPosition({240.f, 460.f});
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
            bool canAfford = (money >= OP_COST && iron >= costIron);
            
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

    if (isWorking) return false;

    // Helper to process click for a specific button
    auto tryClick = [&](sf::RectangleShape& btn, int ironReq, float timeReq) {
        if (btn.getGlobalBounds().contains(mousePos)) {
            if (money >= OP_COST && iron >= ironReq) {
                // Deduct: $10 + Iron Batch
                money -= OP_COST;
                iron -= ironReq;
                
                // Start Timer: 30s/25s/15s
                currentBatchIron = ironReq;
                currentTotalTime = timeReq;
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

void drawIronworks(sf::RenderWindow& window)
{
    if (hutaTitleSprite) window.draw(*hutaTitleSprite);
    
    //UK flag
    if (ukFlagSprite) window.draw(*ukFlagSprite);

    // Easteregg
    if (britishBonusCollected && bonusMessageTimer.getElapsedTime().asSeconds() < 3.0f) {
        if (bonusMessageText) window.draw(*bonusMessageText);
    }

    window.draw(btnSmall); if (txtSmall) window.draw(*txtSmall);
    window.draw(btnMedium); if (txtMedium) window.draw(*txtMedium);
    window.draw(btnLarge); if (txtLarge) window.draw(*txtLarge);
    
    if (infoText) window.draw(*infoText);
}