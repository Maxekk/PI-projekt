#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

// KOPALNIA title image at top center
static sf::Texture kopalniaTitleTexture;
static std::optional<sf::Sprite> kopalniaTitleSprite;
static bool kopalniaTitleLoaded = false;

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

// Critical Hit upgrade
static bool criticalHitPurchased = false;
static sf::RectangleShape criticalHitButton({250.f, 60.f});
static std::optional<sf::Text> criticalHitButtonText;
static const int criticalHitCost = 50;

// Faster Steps upgrade
static bool fasterStepsPurchased = false;
static sf::RectangleShape fasterStepsButton({250.f, 60.f});
static std::optional<sf::Text> fasterStepsButtonText;
static const int fasterStepsCost = 50;

// Mini-game: Falling items
enum class ItemType {
    Good,  // Brown - gives iron
    Bad    // Red - takes iron
};

struct FallingItem {
    sf::RectangleShape shape;
    float speed;
    bool active;
    ItemType type;
    
    FallingItem() : speed(100.f), active(false), type(ItemType::Good) {
        shape.setSize({30.f, 30.f});
        shape.setFillColor(sf::Color(139, 69, 19)); // Brown color for iron ore
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(2.f);
    }
    
    void setType(ItemType itemType) {
        type = itemType;
        if (itemType == ItemType::Bad) {
            shape.setFillColor(sf::Color(200, 50, 50)); // Red color for bad items
        } else {
            shape.setFillColor(sf::Color(139, 69, 19)); // Brown color for good items
        }
    }
};

static std::vector<FallingItem> fallingItems;
static sf::Clock itemSpawnTimer;
static const float itemSpawnInterval = 1.5f; // Spawn new item every 1.5 seconds
static const float itemFallSpeed = 150.f; // Pixels per second

// Bin to collect items (replaced with miner sprite)
static sf::Texture minerTexture;
static sf::Texture minerWalkingTexture;
static std::optional<sf::Sprite> minerSprite;
static bool minerTextureLoaded = false;
static bool minerWalkingTextureLoaded = false;
static sf::Clock animationTimer;
static const float animationInterval = 0.3f; // Switch frames every 0.3 seconds (faster than 0.5)
static bool currentFrame = false; // false = miner.png, true = miner-walking.png
static float binSpeed = 300.f; // Pixels per second (can be upgraded)
static const float binSpeedBase = 300.f; // Base speed
static const float binSpeedMultiplier = 2.0f; // Speed multiplier when upgraded
static const float binY = 520.f; // Fixed Y position at bottom
static bool movingLeft = false; // Track direction for sprite flipping

void initMine(const sf::Font& font)
{
    if (kopalniaTitleLoaded) return;
    
    // Load KOPALNIA title image
    if (!kopalniaTitleLoaded && kopalniaTitleTexture.loadFromFile("images/ui-kopalnia.png"))
    {
        kopalniaTitleSprite = sf::Sprite(kopalniaTitleTexture);
        // Position at top center (window is 800 wide, so center is 400)
        // Make it pretty big - around 400 pixels wide
        sf::Vector2u textureSize = kopalniaTitleTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 400.f / static_cast<float>(textureSize.x);
            float scaleY = scaleX; // Maintain aspect ratio
            kopalniaTitleSprite->setScale({scaleX, scaleY});
            // Center horizontally: (800 - (textureSize.x * scaleX)) / 2
            float scaledWidth = textureSize.x * scaleX;
            float xPos = (800.f - scaledWidth) / 2.f;
            kopalniaTitleSprite->setPosition({xPos, 20.f});
        }
        kopalniaTitleLoaded = true;
    }

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

    // Critical Hit upgrade button
    criticalHitButton.setFillColor(sf::Color(100, 60, 100));
    criticalHitButton.setOutlineColor(sf::Color::White);
    criticalHitButton.setOutlineThickness(2.f);
    criticalHitButton.setPosition({175.f, 260.f});

    criticalHitButtonText = sf::Text(font, "Critical Hit\nCost: 50$", 20);
    criticalHitButtonText->setFillColor(sf::Color::White);
    criticalHitButtonText->setOutlineColor(sf::Color::Black);
    criticalHitButtonText->setOutlineThickness(1.f);
    criticalHitButtonText->setPosition({200.f, 270.f});

    // Faster Steps upgrade button
    fasterStepsButton.setFillColor(sf::Color(60, 100, 150));
    fasterStepsButton.setOutlineColor(sf::Color::White);
    fasterStepsButton.setOutlineThickness(2.f);
    fasterStepsButton.setPosition({175.f, 340.f});

    fasterStepsButtonText = sf::Text(font, "Faster Steps\nCost: 50$", 20);
    fasterStepsButtonText->setFillColor(sf::Color::White);
    fasterStepsButtonText->setOutlineColor(sf::Color::Black);
    fasterStepsButtonText->setOutlineThickness(1.f);
    fasterStepsButtonText->setPosition({200.f, 350.f});

    // Load miner textures (both frames for animation)
    if (!minerTextureLoaded && minerTexture.loadFromFile("images/miner.png"))
    {
        minerTextureLoaded = true;
    }
    if (!minerWalkingTextureLoaded && minerWalkingTexture.loadFromFile("images/miner-walking.png"))
    {
        minerWalkingTextureLoaded = true;
    }
    
    // Initialize miner sprite with first texture
    if (minerTextureLoaded && !minerSprite.has_value())
    {
        minerSprite = sf::Sprite(minerTexture);
        // Scale to much bigger size (200x150 - significantly larger)
        sf::Vector2u textureSize = minerTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 200.f / static_cast<float>(textureSize.x);
            float scaleY = 150.f / static_cast<float>(textureSize.y);
            // Set origin to center for proper flipping
            minerSprite->setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
            minerSprite->setScale({scaleX, scaleY});
        }
        minerSprite->setPosition({360.f + 100.f, binY -50.f}); // Start in center (adjusted for origin: 200/2=100, 150/2=75)
        animationTimer.restart();
    }

    // Initialize falling items vector with some capacity
    fallingItems.resize(20);
    itemSpawnTimer.restart();
}

void updateMine(const sf::Vector2f& mousePos, int money, float deltaTime, bool keyA, bool keyD, long long& collectedIron, int& xp)
{
    // Don't update mini-game when store is open
    if (storeOpen)
    {
        // Hover effect for store close button
        if (storeCloseButton.getGlobalBounds().contains(mousePos))
        {
            storeCloseButton.setFillColor(sf::Color(180, 70, 70));
        }
        else
        {
            storeCloseButton.setFillColor(sf::Color(150, 50, 50));
        }

        // Hover effect for double click upgrade button
        if (!doubleClickPurchased)
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

        // Hover effect for critical hit upgrade button
        if (!criticalHitPurchased)
        {
            if (criticalHitButton.getGlobalBounds().contains(mousePos))
            {
                if (money >= criticalHitCost)
                {
                    criticalHitButton.setFillColor(sf::Color(130, 80, 130));
                }
                else
                {
                    criticalHitButton.setFillColor(sf::Color(100, 60, 60));
                }
            }
            else
            {
                if (money >= criticalHitCost)
                {
                    criticalHitButton.setFillColor(sf::Color(100, 60, 100));
                }
                else
                {
                    criticalHitButton.setFillColor(sf::Color(80, 50, 50));
                }
            }
        }

        // Hover effect for faster steps upgrade button
        if (!fasterStepsPurchased)
        {
            if (fasterStepsButton.getGlobalBounds().contains(mousePos))
            {
                if (money >= fasterStepsCost)
                {
                    fasterStepsButton.setFillColor(sf::Color(80, 130, 180));
                }
                else
                {
                    fasterStepsButton.setFillColor(sf::Color(100, 60, 60));
                }
            }
            else
            {
                if (money >= fasterStepsCost)
                {
                    fasterStepsButton.setFillColor(sf::Color(60, 100, 150));
                }
                else
                {
                    fasterStepsButton.setFillColor(sf::Color(80, 50, 50));
                }
            }
        }
        return;
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

    // Update bin speed based on upgrade
    if (fasterStepsPurchased)
    {
        binSpeed = binSpeedBase * binSpeedMultiplier;
    }
    else
    {
        binSpeed = binSpeedBase;
    }

    // Update bin movement (A and D keys)
    float binX = 460.f; // Default center position (adjusted for origin: 360 + 100)
    if (minerSprite.has_value())
    {
        binX = minerSprite->getPosition().x;
    }
    
    if (keyA && binX > 100.f) // 100 = half of miner width (200/2)
    {
        binX -= binSpeed * deltaTime;
        if (binX < 100.f) binX = 100.f;
        movingLeft = true;
    }
    else if (keyD && binX < 700.f) // 800 - 100 (half miner width)
    {
        binX += binSpeed * deltaTime;
        if (binX > 700.f) binX = 700.f;
        movingLeft = false;
    }
    
    // Update walking animation (only when moving)
    bool isMoving = (keyA || keyD);
    
    if (minerSprite.has_value())
    {
        if (isMoving)
        {
            // Animate when moving
            if (animationTimer.getElapsedTime().asSeconds() >= animationInterval)
            {
                animationTimer.restart();
                currentFrame = !currentFrame; // Switch frame
                
                // Get current scale to preserve flip direction
                sf::Vector2f currentScale = minerSprite->getScale();
                bool isFlipped = (currentScale.x < 0.f);
                
                // Switch texture based on current frame
                if (currentFrame && minerWalkingTextureLoaded)
                {
                    minerSprite->setTexture(minerWalkingTexture);
                }
                else if (!currentFrame && minerTextureLoaded)
                {
                    minerSprite->setTexture(minerTexture);
                }
                
                // Recalculate scale and origin for new texture
                const sf::Texture& currentTexture = minerSprite->getTexture();
                sf::Vector2u textureSize = currentTexture.getSize();
                if (textureSize.x > 0 && textureSize.y > 0)
                {
                    float scaleX = 200.f / static_cast<float>(textureSize.x);
                    float scaleY = 150.f / static_cast<float>(textureSize.y);
                    minerSprite->setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
                    // Preserve flip direction
                    if (isFlipped)
                    {
                        minerSprite->setScale({-scaleX, scaleY});
                    }
                    else
                    {
                        minerSprite->setScale({scaleX, scaleY});
                    }
                }
            }
        }
        else
        {
            // When not moving, reset to idle frame (miner.png)
            if (currentFrame && minerTextureLoaded)
            {
                currentFrame = false;
                // Get current scale to preserve flip direction
                sf::Vector2f currentScale = minerSprite->getScale();
                bool isFlipped = (currentScale.x < 0.f);
                
                minerSprite->setTexture(minerTexture);
                
                // Recalculate scale and origin
                sf::Vector2u textureSize = minerTexture.getSize();
                if (textureSize.x > 0 && textureSize.y > 0)
                {
                    float scaleX = 200.f / static_cast<float>(textureSize.x);
                    float scaleY = 150.f / static_cast<float>(textureSize.y);
                    minerSprite->setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
                    // Preserve flip direction
                    if (isFlipped)
                    {
                        minerSprite->setScale({-scaleX, scaleY});
                    }
                    else
                    {
                        minerSprite->setScale({scaleX, scaleY});
                    }
                }
                animationTimer.restart(); // Reset timer when stopping
            }
        }
    }
    
    // Update miner sprite position and flip based on direction
    if (minerSprite.has_value())
    {
        minerSprite->setPosition({binX, binY + 10.f}); // Adjusted for origin (150/2=75)
        // Flip horizontally when moving left
        // Use current texture to get size
        const sf::Texture& currentTexture = minerSprite->getTexture();
        sf::Vector2u textureSize = currentTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 200.f / static_cast<float>(textureSize.x);
            float scaleY = 150.f / static_cast<float>(textureSize.y);
            if (movingLeft)
            {
                // Flip by making scale X negative
                minerSprite->setScale({-scaleX, scaleY});
            }
            else
            {
                // Normal orientation (scale X positive)
                minerSprite->setScale({scaleX, scaleY});
            }
        }
    }

    // Spawn new items
    if (itemSpawnTimer.getElapsedTime().asSeconds() >= itemSpawnInterval)
    {
        itemSpawnTimer.restart();
        
        // Random number generator for item type and position
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDis(0.f, 770.f);
        std::uniform_int_distribution<> typeDis(0, 4); // 0-3 = good, 4 = bad (20% chance for bad)
        
        // Find an inactive item or create a new one
        bool found = false;
        for (auto& item : fallingItems)
        {
            if (!item.active)
            {
                item.active = true;
                // Random X position (0 to 770, leaving space for bin width)
                item.shape.setPosition({posDis(gen), -30.f}); // Start above screen
                item.speed = itemFallSpeed;
                // Randomly assign type (80% good, 20% bad)
                ItemType itemType = (typeDis(gen) == 4) ? ItemType::Bad : ItemType::Good;
                item.setType(itemType);
                found = true;
                break;
            }
        }
        
        // If all items are active, add a new one
        if (!found)
        {
            FallingItem newItem;
            newItem.active = true;
            newItem.shape.setPosition({posDis(gen), -30.f});
            newItem.speed = itemFallSpeed;
            // Randomly assign type (80% good, 20% bad)
            ItemType itemType = (typeDis(gen) == 4) ? ItemType::Bad : ItemType::Good;
            newItem.setType(itemType);
            fallingItems.push_back(newItem);
        }
    }

    // Update falling items
    sf::Vector2f binPos = {360.f, binY};
    sf::Vector2f binSize = {200.f, 150.f}; // Updated size for bigger miner
    if (minerSprite.has_value())
    {
        binPos = minerSprite->getPosition();
        // Adjust position for origin (origin is at center)
        // Get actual texture size from current sprite
        const sf::Texture& currentTexture = minerSprite->getTexture();
        sf::Vector2u textureSize = currentTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 200.f / static_cast<float>(textureSize.x);
            float scaleY = 150.f / static_cast<float>(textureSize.y);
            binPos.x -= (textureSize.x * scaleX) / 2.f;
            binPos.y -= (textureSize.y * scaleY) / 2.f;
            binSize = {200.f, 150.f}; // Updated size
        }
    }
    
    for (auto& item : fallingItems)
    {
        if (item.active)
        {
            // Move item down
            sf::Vector2f pos = item.shape.getPosition();
            pos.y += item.speed * deltaTime;
            item.shape.setPosition(pos);

            // Check collision with bin
            sf::Vector2f itemPos = item.shape.getPosition();
            sf::Vector2f itemSize = item.shape.getSize();
            // Manual collision check using positions and sizes
            bool collides = (itemPos.x < binPos.x + binSize.x &&
                           itemPos.x + itemSize.x > binPos.x &&
                           itemPos.y < binPos.y + binSize.y &&
                           itemPos.y + itemSize.y > binPos.y);
            if (collides)
            {
                item.active = false;
                
                if (item.type == ItemType::Bad)
                {
                    // Bad item: subtract 10 iron
                    collectedIron -= 10;
                    // Ensure iron doesn't go below 0
                    if (collectedIron < 0)
                    {
                        collectedIron = 0;
                    }
                    // Bad items don't give XP
                }
                else
                {
                    // Good item: increment iron
                    int baseGain = 1;
                    if (doubleClickPurchased)
                    {
                        baseGain = 2;
                    }
                    
                    // Check for critical hit (20% chance if purchased)
                    if (criticalHitPurchased)
                    {
                        static std::random_device rd;
                        static std::mt19937 gen(rd());
                        std::uniform_int_distribution<> dis(1, 100);
                        int roll = dis(gen);
                        if (roll <= 20) // 20% chance
                        {
                            baseGain *= 3;
                        }
                    }
                    
                    collectedIron += baseGain;
                    xp++; // Each collected good item gives 1 XP
                }
            }
            // Remove item if it falls off screen
            else if (pos.y > 600.f)
            {
                item.active = false;
            }
        }
    }
}

bool handleMineClick(const sf::Vector2f& mousePos, long long& collectedIron, int& money, int& xp)
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

    // Handle critical hit upgrade purchase
    if (storeOpen && !criticalHitPurchased && criticalHitButton.getGlobalBounds().contains(mousePos))
    {
        if (money >= criticalHitCost)
        {
            money -= criticalHitCost;
            criticalHitPurchased = true;
            criticalHitButton.setFillColor(sf::Color(40, 40, 40));
            if (criticalHitButtonText.has_value())
            {
                criticalHitButtonText->setString("Critical Hit\nPURCHASED");
            }
            return true;
        }
    }

    // Handle faster steps upgrade purchase
    if (storeOpen && !fasterStepsPurchased && fasterStepsButton.getGlobalBounds().contains(mousePos))
    {
        if (money >= fasterStepsCost)
        {
            money -= fasterStepsCost;
            fasterStepsPurchased = true;
            fasterStepsButton.setFillColor(sf::Color(40, 40, 40));
            if (fasterStepsButtonText.has_value())
            {
                fasterStepsButtonText->setString("Faster Steps\nPURCHASED");
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

    return false;
}

void drawMine(sf::RenderWindow& window, int money)
{
    // Draw KOPALNIA title at top center
    if (kopalniaTitleSprite.has_value())
    {
        window.draw(*kopalniaTitleSprite);
    }
    
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

        // Draw critical hit upgrade button
        if (!criticalHitPurchased)
        {
            window.draw(criticalHitButton);
            if (criticalHitButtonText.has_value())
            {
                // Update text color based on affordability
                if (money >= criticalHitCost)
                {
                    criticalHitButtonText->setFillColor(sf::Color::White);
                }
                else
                {
                    criticalHitButtonText->setFillColor(sf::Color(200, 150, 150));
                }
                window.draw(*criticalHitButtonText);
            }
        }
        else
        {
            // Draw purchased upgrade (grayed out)
            window.draw(criticalHitButton);
            if (criticalHitButtonText.has_value())
            {
                criticalHitButtonText->setFillColor(sf::Color(150, 150, 150));
                window.draw(*criticalHitButtonText);
            }
        }

        // Draw faster steps upgrade button
        if (!fasterStepsPurchased)
        {
            window.draw(fasterStepsButton);
            if (fasterStepsButtonText.has_value())
            {
                // Update text color based on affordability
                if (money >= fasterStepsCost)
                {
                    fasterStepsButtonText->setFillColor(sf::Color::White);
                }
                else
                {
                    fasterStepsButtonText->setFillColor(sf::Color(200, 150, 150));
                }
                window.draw(*fasterStepsButtonText);
            }
        }
        else
        {
            // Draw purchased upgrade (grayed out)
            window.draw(fasterStepsButton);
            if (fasterStepsButtonText.has_value())
            {
                fasterStepsButtonText->setFillColor(sf::Color(150, 150, 150));
                window.draw(*fasterStepsButtonText);
            }
        }
    }
    else
    {
        // Draw mini-game: miner sprite and falling items
        if (minerSprite.has_value())
        {
            window.draw(*minerSprite);
        }
        
        for (const auto& item : fallingItems)
        {
            if (item.active)
            {
                window.draw(item.shape);
            }
        }
    }
}
