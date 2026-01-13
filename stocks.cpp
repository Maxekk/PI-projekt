#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <optional>

struct Company {
    std::string name;
    int pricePerUnit;
    sf::RectangleShape box;
    sf::RectangleShape sellBtn;
    std::optional<sf::Text> nameTxt;
    std::optional<sf::Text> priceTxt;
    std::optional<sf::Text> totalPriceTxt; 
    std::optional<sf::Text> btnTxt;
};

struct AmountOption {
    sf::RectangleShape box;
    std::optional<sf::Text> txt;
    long long value; 
};

static std::vector<Company> companies;
static std::vector<AmountOption> amountOptions;
static long long currentMultiplier = 1;
static bool isAllSelected = false;

static sf::Clock priceTimer;
static sf::Texture gieldaTitleTexture;
static std::optional<sf::Sprite> gieldaTitleSprite;
static bool gieldaTitleLoaded = false;
static bool stocksInitialized = false;

static sf::Music stocksMusic;
static bool stocksMusicLoaded = false;
static bool stocksMusicPlaying = false;

void initStocks(const sf::Font& font)
{
    if (stocksInitialized) return;

    // 1. tablica z nazwą
    if (!gieldaTitleLoaded && gieldaTitleTexture.loadFromFile("images/ui-gielda.png")) {
        gieldaTitleSprite.emplace(gieldaTitleTexture);
        sf::Vector2u ts = gieldaTitleTexture.getSize();
        if (ts.x > 0) {
            float scale = 400.f / (float)ts.x;
            gieldaTitleSprite->setScale({scale, scale});
            gieldaTitleSprite->setPosition({(800.f - ts.x * scale) / 2.f, 20.f});
        }
        gieldaTitleLoaded = true;
    }

    // 2. Klienci - firmy
    std::vector<std::string> names = { "PolStal", "Stalex", "Stalium", "ATS" };
    float startX = 35.f;
    float gap = 20.f;
    float sectionWidth = 170.f;

    for (int i = 0; i < 4; ++i) {
        Company c;
        c.name = names[i];
        
        // Cena za sztukę
        c.pricePerUnit = rand() % 9 + 1; 

        float posX = startX + i * (sectionWidth + gap);

        c.box.setSize({ sectionWidth, 270.f });
        c.box.setPosition({ posX, 150.f });
        c.box.setFillColor(sf::Color(40, 40, 45, 220));
        c.box.setOutlineThickness(2.f);
        c.box.setOutlineColor(sf::Color(100, 100, 100));

        c.nameTxt.emplace(font, c.name, 22);
        c.nameTxt->setPosition({ posX + 15.f, 165.f });

        c.priceTxt.emplace(font, "Cena za kg: " + std::to_string(c.pricePerUnit) + " $", 17);
        c.priceTxt->setFillColor(sf::Color::Yellow);
        c.priceTxt->setPosition({ posX + 15.f, 200.f });

        c.totalPriceTxt.emplace(font, "", 15);
        c.totalPriceTxt->setFillColor(sf::Color(200, 200, 200));
        c.totalPriceTxt->setPosition({ posX + 15.f, 230.f });

        c.sellBtn.setSize({ sectionWidth - 30.f, 45.f });
        c.sellBtn.setPosition({ posX + 15.f, 345.f });
        
        c.btnTxt.emplace(font, "SPRZEDAJ", 18);
        c.btnTxt->setStyle(sf::Text::Bold);
        sf::FloatRect bt = c.btnTxt->getLocalBounds();
        c.btnTxt->setOrigin({bt.position.x + bt.size.x/2.f, bt.position.y + bt.size.y/2.f});
        c.btnTxt->setPosition({posX + (sectionWidth/2.f), 345.f + 22.5f});

        companies.push_back(std::move(c));
    }

    // 3. Przyciski ilości 
    std::vector<std::pair<std::string, long long>> opts = { 
        {"1x", 1}, {"5x", 5}, {"10x", 10}, {"100x", 100}, {"ALL", -1} 
    };
    float btnW = 80.f;
    float startXBtn = (800.f - ((btnW * 5) + (10.f * 4))) / 2.f;

    for (int i = 0; i < 5; ++i) {
        AmountOption ao;
        ao.value = opts[i].second;
        ao.box.setSize({ btnW, 40.f });
        ao.box.setPosition({ startXBtn + i * (btnW + 10.f), 440.f });
        ao.box.setOutlineThickness(2.f);
        ao.box.setOutlineColor(sf::Color::White);
        ao.txt.emplace(font, opts[i].first, 18);
        sf::FloatRect tr = ao.txt->getLocalBounds();
        ao.txt->setOrigin({tr.position.x + tr.size.x / 2.f, tr.position.y + tr.size.y / 2.f});
        ao.txt->setPosition({startXBtn + i * (btnW + 10.f) + btnW/2.f, 440.f + 20.f});
        amountOptions.push_back(std::move(ao));
    }

    if (!stocksMusicLoaded) {
        if (stocksMusic.openFromFile("stock-theme.wav") || 
            stocksMusic.openFromFile("sounds/stock-theme.wav") || 
            stocksMusic.openFromFile("audio/stock-theme.wav")) {
            stocksMusicLoaded = true;
            stocksMusic.setLooping(true);
        }
    }

    stocksInitialized = true;
    priceTimer.restart();
}

void updateStocks(const sf::Vector2f& mousePos, long long& steel, int& money)
{
    // Aktualizacja cen =5 sekund
    if (priceTimer.getElapsedTime().asSeconds() >= 5.0f) {
        for (auto& c : companies) {
            // ZMIANA: Cena od 1 do 9 $
            c.pricePerUnit = rand() % 9 + 1; 
            if (c.priceTxt) c.priceTxt->setString("Cena za kg: " + std::to_string(c.pricePerUnit) + " $");
        }
        priceTimer.restart();
    }

    for (auto& ao : amountOptions) {
        bool isActive = (ao.value == currentMultiplier && !isAllSelected) || (ao.value == -1 && isAllSelected);
        if (isActive) ao.box.setFillColor(sf::Color(0, 100, 200));
        else if (ao.box.getGlobalBounds().contains(mousePos)) ao.box.setFillColor(sf::Color(80, 80, 80));
        else ao.box.setFillColor(sf::Color(30, 30, 30));
    }

    long long actualAmount = isAllSelected ? steel : currentMultiplier;
    for (auto& c : companies) {
        if (c.totalPriceTxt) {
            long long totalVal = actualAmount * c.pricePerUnit;
            std::string info = "Za " + std::to_string(actualAmount) + " sztuk:\n" + std::to_string(totalVal) + " $";
            c.totalPriceTxt->setString(info);
        }

        bool canAfford = (steel >= actualAmount && actualAmount > 0);
        if (c.sellBtn.getGlobalBounds().contains(mousePos)) {
            c.sellBtn.setFillColor(canAfford ? sf::Color(70, 200, 70) : sf::Color(150, 50, 50));
        } else {
            c.sellBtn.setFillColor(canAfford ? sf::Color(50, 150, 50) : sf::Color(80, 80, 80));
        }
    }
}

bool handleStocksClick(const sf::Vector2f& mousePos, long long& steel, int& money)
{
    for (auto& ao : amountOptions) {
        if (ao.box.getGlobalBounds().contains(mousePos)) {
            if (ao.value == -1) { isAllSelected = true; currentMultiplier = -1; }
            else { isAllSelected = false; currentMultiplier = ao.value; }
            return true;
        }
    }

    for (auto& c : companies) {
        if (c.sellBtn.getGlobalBounds().contains(mousePos)) {
            long long amountToSell = isAllSelected ? steel : currentMultiplier;
            if (amountToSell > 0 && steel >= amountToSell) {
                int profit = (int)(amountToSell * (long long)c.pricePerUnit);
                steel -= amountToSell;
                money += profit;
                return true;
            }
        }
    }
    return false;
}

void drawStocks(sf::RenderWindow& window)
{
    if (gieldaTitleSprite) window.draw(*gieldaTitleSprite);
    for (const auto& c : companies) {
        window.draw(c.box);
        if (c.nameTxt) window.draw(*c.nameTxt);
        if (c.priceTxt) window.draw(*c.priceTxt);
        if (c.totalPriceTxt) window.draw(*c.totalPriceTxt);
        window.draw(c.sellBtn);
        if (c.btnTxt) window.draw(*c.btnTxt);
    }
    for (const auto& ao : amountOptions) {
        window.draw(ao.box);
        if (ao.txt) window.draw(*ao.txt);
    }
}

void playStocksMusic() { if (stocksMusicLoaded && !stocksMusicPlaying) { stocksMusic.play(); stocksMusicPlaying = true; } }
void stopStocksMusic() { if (stocksMusicLoaded && stocksMusicPlaying) { stocksMusic.stop(); stocksMusicPlaying = false; } }