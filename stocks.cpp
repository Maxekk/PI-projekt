#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <optional>
#include <string>

// 20 steel -> $30
static const int SELL_1_STEEL = 20;
static const int SELL_1_PRICE = 30;

// 50 steel -> $100
static const int SELL_2_STEEL = 50;
static const int SELL_2_PRICE = 100;

// 100 steel -> $250 
static const int SELL_3_STEEL = 100;
static const int SELL_3_PRICE = 250;

// UI Elements
static sf::RectangleShape btnSell1({280.f, 60.f});
static sf::RectangleShape btnSell2({280.f, 60.f});
static sf::RectangleShape btnSell3({280.f, 60.f});

static std::optional<sf::Text> txtSell1;
static std::optional<sf::Text> txtSell2;
static std::optional<sf::Text> txtSell3;
static std::optional<sf::Text> marketInfoText;

// GIELDA title image
static sf::Texture gieldaTitleTexture;
static std::optional<sf::Sprite> gieldaTitleSprite;
static bool gieldaTitleLoaded = false;

// Music for stocks theme (reuse ironworks file for now)
static sf::Music stocksMusic;
static bool stocksMusicLoaded = false;
static bool stocksMusicPlaying = false;

void initStocks(const sf::Font& font)
{
    if (txtSell1.has_value() && gieldaTitleLoaded) return;

    // Load Title
    if (!gieldaTitleLoaded && gieldaTitleTexture.loadFromFile("images/ui-gielda.png"))
    {
        gieldaTitleSprite.emplace(gieldaTitleTexture);
        sf::Vector2u ts = gieldaTitleTexture.getSize();
        if (ts.x > 0) {
            float scale = 400.f / (float)ts.x;
            gieldaTitleSprite->setScale({scale, scale});
            gieldaTitleSprite->setPosition({(800.f - ts.x * scale) / 2.f, 20.f});
        }
        gieldaTitleLoaded = true;
    }

    float startX = 260.f;
    float startY = 180.f;
    float gap = 85.f;

    // Button 1
    btnSell1.setPosition({startX, startY});
    btnSell1.setOutlineThickness(2.f);
    txtSell1.emplace(font, "Sprzedaj 20 Stali ($30)", 20);
    txtSell1->setPosition({startX + 20.f, startY + 18.f});

    // Button 2
    btnSell2.setPosition({startX, startY + gap});
    btnSell2.setOutlineThickness(2.f);
    txtSell2.emplace(font, "Sprzedaj 50 Stali ($100)", 20);
    txtSell2->setPosition({startX + 20.f, startY + gap + 18.f});

    // Button 3
    btnSell3.setPosition({startX, startY + gap * 2});
    btnSell3.setOutlineThickness(2.f);
    txtSell3.emplace(font, "Sprzedaj 100 Stali ($250)", 20);
    txtSell3->setPosition({startX + 20.f, startY + gap * 2 + 18.f});

    marketInfoText.emplace(font, "Wybierz oferte sprzedazy.", 24);
    marketInfoText->setFillColor(sf::Color::Green);
    marketInfoText->setPosition({260.f, 460.f});

    // Load market/stocks theme (reuse ironworks-theme.wav)
    if (!stocksMusicLoaded)
    {
            try
            {
                bool opened = stocksMusic.openFromFile("stock-theme.wav");
                if (!opened) opened = stocksMusic.openFromFile("sounds/stock-theme.wav");
                if (!opened) opened = stocksMusic.openFromFile("audio/stock-theme.wav");

                if (opened)
                {
                    stocksMusicLoaded = true;
                    stocksMusic.setLooping(true);
                    stocksMusicPlaying = false;
                }
                else
                {
                    std::cerr << "stocks: failed to open music file from tried paths\n";
                }
            }
            catch (const std::exception&)
            {
                stocksMusicLoaded = false;
            }

        
    }
}

// Logic: Check if player has enough steel to sell
void updateStocks(const sf::Vector2f& mousePos, long long& steel, int& money)
{
    // Helper lambda for button logic
    auto updateBtn = [&](sf::RectangleShape& btn, int steelReq) {
        bool canAfford = (steel >= steelReq);
        
        if (btn.getGlobalBounds().contains(mousePos)) {
            // Hover: Bright Green if can afford, Dark Red if cannot
            btn.setFillColor(canAfford ? sf::Color(50, 150, 50) : sf::Color(100, 50, 50));
        } else {
            // Idle: Green if can afford, Grey if cannot
            btn.setFillColor(canAfford ? sf::Color(30, 100, 30) : sf::Color(60, 60, 60));
        }
    };

    updateBtn(btnSell1, SELL_1_STEEL);
    updateBtn(btnSell2, SELL_2_STEEL);
    updateBtn(btnSell3, SELL_3_STEEL);

    // Context info
    if (marketInfoText) {
        if (steel < 20) marketInfoText->setString("Brak stali na sprzedaz!");
        else marketInfoText->setString("Kursy walut stabilne.");
    }
}

// Handle clicks: Sell instant
bool handleStocksClick(const sf::Vector2f& mousePos, long long& steel, int& money)
{
    auto trySell = [&](sf::RectangleShape& btn, int steelReq, int price) {
        if (btn.getGlobalBounds().contains(mousePos)) {
            if (steel >= steelReq) {
                steel -= steelReq;
                money += price;
                if (marketInfoText) marketInfoText->setString("Sprzedano! +$" + std::to_string(price));
                return true;
            } else {
                if (marketInfoText) marketInfoText->setString("Za malo stali!");
            }
        }
        return false;
    };

    if (trySell(btnSell1, SELL_1_STEEL, SELL_1_PRICE)) return true;
    if (trySell(btnSell2, SELL_2_STEEL, SELL_2_PRICE)) return true;
    if (trySell(btnSell3, SELL_3_STEEL, SELL_3_PRICE)) return true;

    return false;
}

void drawStocks(sf::RenderWindow& window)
{
    if (gieldaTitleSprite) window.draw(*gieldaTitleSprite);
    
    window.draw(btnSell1); if (txtSell1) window.draw(*txtSell1);
    window.draw(btnSell2); if (txtSell2) window.draw(*txtSell2);
    window.draw(btnSell3); if (txtSell3) window.draw(*txtSell3);
    
    if (marketInfoText) window.draw(*marketInfoText);
}

// Music control for stocks theme
void playStocksMusic()
{
    if (stocksMusicLoaded && !stocksMusicPlaying)
    {
        stocksMusic.play();
        stocksMusicPlaying = true;
    }
}

void stopStocksMusic()
{
    if (stocksMusicLoaded && stocksMusicPlaying)
    {
        stocksMusic.stop();
        stocksMusicPlaying = false;
    }
}