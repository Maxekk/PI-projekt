#include <SFML/Graphics.hpp>
#include <iostream>
// Ironworks (Furnace) location - simple, just shows title
// No special UI needed for now
static bool isworking = false;
static sf::Clock workclock;
static const float worktime = 20.0f;
static const int ironcost = 1000;
static const int moneyrew = 100;

void updateIronworks(const sf::Vector2f& mousePos, int& money, long long& ironAmount)
{
    if (isworking)
    {
        float elapsed = workclock.getElapsedTime().asSeconds();
        if (elapsed >= worktime)
        {
            // Koniec czasu
            isworking = false;
            money += moneyrew;
            if (furnaceInfoText) furnaceInfoText->setString("Przetop gotowy! +$" + std::to_string(moneyrew));
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


bool handleIronworksClick(const sf::Vector2f& mousePos, int& money, long long& iron)
{
    // Klikamy tylko, gdy NIE pracuje i mamy surowce
    if (!isworking && iron >= ironcost && furnaceButton.getGlobalBounds().contains(mousePos))
    {
        iron -= ironcost; // Pobierz opłatę
        isworking = true;       // Uruchom maszynę
        workclocklock.restart();    // Resetuj stoper
        return true;
    }
    return false;
}

void drawIronworks(sf::RenderWindow& window)
{
    // Nothing to draw beyond the background and title (handled in main)
}

