#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <string>

static sf::RectangleShape furnaceButton({260.f, 80.f});
static std::optional<sf::Text> furnaceButtonText;
static std::optional<sf::Text> furnaceInfoText;
static bool isworking = false;
static sf::Clock workclock;
static const float worktime = 20.0f;
static const int ironcost = 10;
static const int steelReward = 10; // 10 iron = 10 steel

// HUTA title image at top center
static sf::Texture hutaTitleTexture;
static std::optional<sf::Sprite> hutaTitleSprite;
static bool hutaTitleLoaded = false;

void initIronworks(const sf::Font& font)
{
    if (furnaceButtonText.has_value() && hutaTitleLoaded) return;

    // Load HUTA title image
    if (!hutaTitleLoaded && hutaTitleTexture.loadFromFile("images/ui-huta.png"))
    {
        hutaTitleSprite = sf::Sprite(hutaTitleTexture);
        // Position at top center (window is 800 wide, so center is 400)
        // Make it pretty big - around 400 pixels wide
        sf::Vector2u textureSize = hutaTitleTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 400.f / static_cast<float>(textureSize.x);
            float scaleY = scaleX; // Maintain aspect ratio
            hutaTitleSprite->setScale({scaleX, scaleY});
            // Center horizontally: (800 - (textureSize.x * scaleX)) / 2
            float scaledWidth = textureSize.x * scaleX;
            float xPos = (800.f - scaledWidth) / 2.f;
            hutaTitleSprite->setPosition({xPos, 20.f});
        }
        hutaTitleLoaded = true;
    }

    if (furnaceButtonText.has_value()) return;

    furnaceButton.setFillColor(sf::Color(150, 50, 50));
    furnaceButton.setPosition({270.f, 350.f});
    furnaceButton.setOutlineColor(sf::Color::White);
    furnaceButton.setOutlineThickness(2.f);

    // POPRAWKA: Najpierw FONT, potem TEKST (wymóg SFML 3.0)
    furnaceButtonText.emplace(font, "Przetop", 24);
    furnaceButtonText->setFillColor(sf::Color::White);
    furnaceButtonText->setPosition({330.f, 375.f});

    // POPRAWKA: Najpierw FONT, potem TEKST
    furnaceInfoText.emplace(font, "Piec gotowy.", 24);
    furnaceInfoText->setFillColor(sf::Color::Yellow);
    furnaceInfoText->setPosition({280.f, 300.f});
}

void updateIronworks(const sf::Vector2f& mousePos, long long& iron, long long& steel)
{
    if (isworking)
    {
        float elapsed = workclock.getElapsedTime().asSeconds();
        if (elapsed >= worktime)
        {
            // Koniec czasu
            isworking = false;
            steel += steelReward; // Give steel instead of money
            if (furnaceInfoText) furnaceInfoText->setString("Przetop gotowy! +" + std::to_string(steelReward) + " steel");
            furnaceButton.setFillColor(sf::Color(150, 50, 50)); // Reset koloru
        }
        else
        {
            // W trakcie odliczania
            float remaining = worktime - elapsed;
            if (furnaceInfoText) furnaceInfoText->setString("Przetapianie... " + std::to_string((int)remaining) + "s");
            furnaceButton.setFillColor(sf::Color(100, 30, 30)); // Ciemniejszy kolor
        }
    }
    else 
    {
        // Stan spoczynku - obsługa hover
        if (furnaceButton.getGlobalBounds().contains(mousePos))
             furnaceButton.setFillColor(sf::Color(200, 70, 70));
        else furnaceButton.setFillColor(sf::Color(150, 50, 50));
        
        if (furnaceInfoText && iron < ironcost)
             furnaceInfoText->setString("Brak rudy! (Wymagane: " + std::to_string(ironcost) + ")");
        else if (furnaceInfoText)
             furnaceInfoText->setString("Gotowy do przetopu.");
    }
}

bool handleIronworksClick(const sf::Vector2f& mousePos, long long& iron, long long& steel)
{
    // Klikamy tylko, gdy NIE pracuje i mamy surowce
    if (!isworking && iron >= ironcost && furnaceButton.getGlobalBounds().contains(mousePos))
    {
        iron -= ironcost; // Pobierz opłatę
        isworking = true;       // Uruchom maszynę
        workclock.restart();    // Resetuj stoper
        return true;
    }
    return false;
}

void drawIronworks(sf::RenderWindow& window)
{
    // Draw HUTA title at top center
    if (hutaTitleSprite.has_value())
    {
        window.draw(*hutaTitleSprite);
    }
    
    window.draw(furnaceButton);
    if (furnaceButtonText) window.draw(*furnaceButtonText);
    if (furnaceInfoText) window.draw(*furnaceInfoText);
}