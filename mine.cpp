#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

// Mine location UI elements
static sf::RectangleShape mineButton({200.f, 80.f});
static std::optional<sf::Text> mineButtonText;
static std::optional<sf::Text> mineCounterText;

void initMine(const sf::Font& font)
{
    if (mineButtonText.has_value()) return;
    
    mineButton.setFillColor(sf::Color(100, 100, 150));
    mineButton.setPosition({300.f, 350.f});
    mineButton.setOutlineColor(sf::Color::White);
    mineButton.setOutlineThickness(2.f);

    mineButtonText = sf::Text(font, "Kop", 32);
    mineButtonText->setFillColor(sf::Color::White);
    mineButtonText->setOutlineColor(sf::Color::Black);
    mineButtonText->setOutlineThickness(1.f);
    mineButtonText->setPosition({350.f, 365.f});

    mineCounterText = sf::Text(font);
    mineCounterText->setCharacterSize(24);
    mineCounterText->setFillColor(sf::Color::White);
    mineCounterText->setOutlineColor(sf::Color::Black);
    mineCounterText->setOutlineThickness(1.f);
    mineCounterText->setPosition({300.f, 280.f});
}

void updateMine(int mineClicks, const sf::Vector2f& mousePos)
{
    if (mineCounterText.has_value())
    {
        mineCounterText->setString("Klikniecia: " + std::to_string(mineClicks));
    }
    
    // Hover effect
    if (mineButton.getGlobalBounds().contains(mousePos))
    {
        mineButton.setFillColor(sf::Color(120, 120, 180));
    }
    else
    {
        mineButton.setFillColor(sf::Color(100, 100, 150));
    }
}

bool handleMineClick(const sf::Vector2f& mousePos, int& mineClicks, long long& iron)
{
    if (mineButton.getGlobalBounds().contains(mousePos))
    {
        mineClicks++;
        iron++;
        return true;
    }
    return false;
}

void drawMine(sf::RenderWindow& window)
{
    if (mineCounterText.has_value())
        window.draw(*mineCounterText);
    window.draw(mineButton);
    if (mineButtonText.has_value())
        window.draw(*mineButtonText);
}

