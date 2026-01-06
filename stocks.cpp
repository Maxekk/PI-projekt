#include <SFML/Graphics.hpp>
#include <optional>

// Stocks (Market) location - simple, just shows title
// No special UI needed for now

// GIELDA title image at top center
static sf::Texture gieldaTitleTexture;
static std::optional<sf::Sprite> gieldaTitleSprite;
static bool gieldaTitleLoaded = false;

void initStocks(const sf::Font& font)
{
    // Load GIELDA title image
    if (!gieldaTitleLoaded && gieldaTitleTexture.loadFromFile("images/ui-gielda.png"))
    {
        gieldaTitleSprite = sf::Sprite(gieldaTitleTexture);
        // Position at top center (window is 800 wide, so center is 400)
        // Make it pretty big - around 400 pixels wide
        sf::Vector2u textureSize = gieldaTitleTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 400.f / static_cast<float>(textureSize.x);
            float scaleY = scaleX; // Maintain aspect ratio
            gieldaTitleSprite->setScale({scaleX, scaleY});
            // Center horizontally: (800 - (textureSize.x * scaleX)) / 2
            float scaledWidth = textureSize.x * scaleX;
            float xPos = (800.f - scaledWidth) / 2.f;
            gieldaTitleSprite->setPosition({xPos, 20.f});
        }
        gieldaTitleLoaded = true;
    }
}

void updateStocks(const sf::Vector2f& mousePos)
{
    // Nothing to update for now
}

bool handleStocksClick(const sf::Vector2f& mousePos)
{
    // No clickable elements for now
    return false;
}

void drawStocks(sf::RenderWindow& window)
{
    // Draw GIELDA title at top center
    if (gieldaTitleSprite.has_value())
    {
        window.draw(*gieldaTitleSprite);
    }
}

