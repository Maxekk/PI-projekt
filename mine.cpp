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
// Upgrade button as image
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

// Double Click upgrade
static bool doubleClickPurchased = false;
static sf::RectangleShape doubleClickButton({300.f, 60.f});
static std::optional<sf::Text> doubleClickButtonText;
static const int doubleClickCost = 100;

// Critical Hit upgrade
static bool criticalHitPurchased = false;
static sf::RectangleShape criticalHitButton({300.f, 60.f});
static std::optional<sf::Text> criticalHitButtonText;
static const int criticalHitCost = 50;

// Faster Steps upgrade
static bool fasterStepsPurchased = false;
static sf::RectangleShape fasterStepsButton({300.f, 60.f});
static std::optional<sf::Text> fasterStepsButtonText;
static const int fasterStepsCost = 50;

// Super Ore upgrade
static bool superOrePurchased = false;
static sf::RectangleShape superOreButton({300.f, 60.f});
static std::optional<sf::Text> superOreButtonText;
static const int superOreCost = 50;

// Mini-game: Falling items
enum class ItemType {
    Good,  // Brown - gives iron
    Bad    // Red - takes iron
};

struct FallingItem {
    std::optional<sf::Sprite> sprite;
    float speed;
    bool active;
    ItemType type;
    float rotation;
    float rotationSpeed; // degrees per second
    bool isSuperOre; // Whether this is a super ore (10x value) item

    FallingItem() : speed(100.f), active(false), type(ItemType::Good), rotation(0.f), rotationSpeed(0.f), isSuperOre(false) {
        // sprite texture is assigned later in initMine/setType
    }

    void setType(ItemType itemType);
};

static std::vector<FallingItem> fallingItems;
// Textures for falling items
static sf::Texture ironIngotTexture;
static bool ironIngotTextureLoaded = false;
static sf::Texture furnaceIconTexture;
static bool furnaceIconTextureLoaded = false;
// Music for mine theme
static sf::Music mineMusic;
static bool mineMusicLoaded = false;
static bool mineMusicPlaying = false;
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
    upgradesButton.setPosition({690.f, 70.f});

    // Try to load image for upgrades button
    if (!upgradeBtnTextureLoaded && upgradeBtnTexture.loadFromFile("images/upgrade-btn-icon.png"))
    {
        upgradeBtnTextureLoaded = true;
        upgradeBtnSprite = sf::Sprite(upgradeBtnTexture);
        // Scale to a square button (use the larger dimension to make a square)
        sf::Vector2u ts = upgradeBtnTexture.getSize();
        if (ts.x > 0 && ts.y > 0)
        {
            float desiredSize = 72.f; // square size (72x72) — slightly smaller
            float scale = desiredSize / static_cast<float>(std::max(ts.x, ts.y));
            upgradeBtnOriginalScale = {scale, scale};
            upgradeBtnSprite->setScale(upgradeBtnOriginalScale);
            // Rotate/scale origin to texture center so we can position by center
            upgradeBtnSprite->setOrigin({ts.x / 2.f, ts.y / 2.f});
        }
        // Position sprite centered on the upgrades rectangle area
        sf::Vector2f rectPos = upgradesButton.getPosition();
        sf::Vector2f rectSize = upgradesButton.getSize();
        upgradeBtnSprite->setPosition({rectPos.x + rectSize.x / 2.f, rectPos.y + rectSize.y / 2.f});
    }


    // Store UI
    storeBackground.setFillColor(sf::Color(40, 40, 50));
    storeBackground.setOutlineColor(sf::Color::White);
    storeBackground.setOutlineThickness(3.f);
    storeBackground.setPosition({160.f, 70.f}); // Centered

    storeCloseButton.setFillColor(sf::Color(150, 50, 50));
    storeCloseButton.setPosition({540.f, 80.f}); // Top right of store

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

    // Double Click upgrade button (centered horizontally in store)
    doubleClickButton.setFillColor(sf::Color(60, 100, 60));
    doubleClickButton.setOutlineColor(sf::Color::White);
    doubleClickButton.setOutlineThickness(2.f);
    doubleClickButton.setPosition({180.f, 160.f});

    doubleClickButtonText = sf::Text(font, "Double Click\nCost: 100$", 18);
    doubleClickButtonText->setFillColor(sf::Color::White);
    doubleClickButtonText->setOutlineColor(sf::Color::Black);
    doubleClickButtonText->setOutlineThickness(1.f);
    doubleClickButtonText->setPosition({200.f, 170.f});

    // Critical Hit upgrade button
    criticalHitButton.setFillColor(sf::Color(100, 60, 100));
    criticalHitButton.setOutlineColor(sf::Color::White);
    criticalHitButton.setOutlineThickness(2.f);
    criticalHitButton.setPosition({180.f, 235.f});

    criticalHitButtonText = sf::Text(font, "Critical Hit\nCost: 50$", 18);
    criticalHitButtonText->setFillColor(sf::Color::White);
    criticalHitButtonText->setOutlineColor(sf::Color::Black);
    criticalHitButtonText->setOutlineThickness(1.f);
    criticalHitButtonText->setPosition({200.f, 245.f});

    // Faster Steps upgrade button
    fasterStepsButton.setFillColor(sf::Color(60, 100, 150));
    fasterStepsButton.setOutlineColor(sf::Color::White);
    fasterStepsButton.setOutlineThickness(2.f);
    fasterStepsButton.setPosition({180.f, 310.f});

    fasterStepsButtonText = sf::Text(font, "Faster Steps\nCost: 50$", 18);
    fasterStepsButtonText->setFillColor(sf::Color::White);
    fasterStepsButtonText->setOutlineColor(sf::Color::Black);
    fasterStepsButtonText->setOutlineThickness(1.f);
    fasterStepsButtonText->setPosition({200.f, 320.f});

    // Super Ore upgrade button
    superOreButton.setFillColor(sf::Color(150, 100, 60));
    superOreButton.setOutlineColor(sf::Color::White);
    superOreButton.setOutlineThickness(2.f);
    superOreButton.setPosition({180.f, 385.f});

    superOreButtonText = sf::Text(font, "Super Ore\nCost: 50$", 18);
    superOreButtonText->setFillColor(sf::Color::White);
    superOreButtonText->setOutlineColor(sf::Color::Black);
    superOreButtonText->setOutlineThickness(1.f);
    superOreButtonText->setPosition({200.f, 395.f});

    // Load miner textures (both frames for animation)
    if (!minerTextureLoaded && minerTexture.loadFromFile("images/miner.png"))
    {
        minerTextureLoaded = true;
    }
    if (!minerWalkingTextureLoaded && minerWalkingTexture.loadFromFile("images/miner-walking.png"))
    {
        minerWalkingTextureLoaded = true;
    }
    // Load falling item textures
    if (!ironIngotTextureLoaded && ironIngotTexture.loadFromFile("images/iron-ingot.png"))
    {
        ironIngotTextureLoaded = true;
    }
    if (!furnaceIconTextureLoaded && furnaceIconTexture.loadFromFile("images/sulfur.png"))
    {
        furnaceIconTextureLoaded = true;
    }
    // Load mine theme music (try a few common locations)
    if (!mineMusicLoaded)
    {
        try
        {
            if (mineMusic.openFromFile("sounds/mine-theme.wav") || mineMusic.openFromFile("audio/mine-theme.wav") || mineMusic.openFromFile("mine-theme.wav"))
            {
                mineMusicLoaded = true;
                mineMusic.setLooping(true);
                // Do not auto-play here; play when entering mine location
                mineMusicPlaying = false;
            }
        }
        catch (const std::exception&)
        {
            // Ignore audio initialization failures
            mineMusicLoaded = false;
        }
    }
    
    // Initialize miner sprite with first texture
    if (minerTextureLoaded && !minerSprite.has_value())
    {
        minerSprite = sf::Sprite(minerTexture);
        // Scale to target display size (100x120)
        sf::Vector2u textureSize = minerTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 100.f / static_cast<float>(textureSize.x);
            float scaleY = 120.f / static_cast<float>(textureSize.y);
            // Set origin to center for proper flipping
            minerSprite->setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
            minerSprite->setScale({scaleX, scaleY});
        }
        minerSprite->setPosition({360.f + 50.f, binY -60.f}); // Start in center (adjusted for origin: 100/2=50, 120/2=60)
        animationTimer.restart();
    }

    // Initialize falling items vector with some capacity
    fallingItems.resize(20);
    itemSpawnTimer.restart();
}

// FallingItem::setType implementation (placed after initMine)
void FallingItem::setType(ItemType itemType)
{
    type = itemType;
    if (itemType == ItemType::Bad)
    {
        if (furnaceIconTextureLoaded)
        {
            sprite = sf::Sprite(furnaceIconTexture);
            sf::Vector2u ts = furnaceIconTexture.getSize();
            if (ts.x > 0 && ts.y > 0)
            {
                float scale = 50.f / static_cast<float>(std::max(ts.x, ts.y));
                sprite->setScale({scale, scale});
                // Rotate around texture center
                sprite->setOrigin({ts.x / 2.f, ts.y / 2.f});
                // Randomize rotation and rotation speed
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> rotInitDis(0.f, 360.f);
                std::uniform_real_distribution<float> rotSpeedDis(-180.f, 180.f);
                rotation = rotInitDis(gen);
                rotationSpeed = rotSpeedDis(gen);
                sprite->setRotation(sf::degrees(rotation));
            }
        }
    }
    else
    {
        if (ironIngotTextureLoaded)
        {
            sprite = sf::Sprite(ironIngotTexture);
            sf::Vector2u ts = ironIngotTexture.getSize();
            if (ts.x > 0 && ts.y > 0)
            {
                // Make super ore items 2x larger for visibility
                float baseScale = isSuperOre ? 100.f : 50.f;
                float scale = baseScale / static_cast<float>(std::max(ts.x, ts.y));
                sprite->setScale({scale, scale});
                // Rotate around texture center
                sprite->setOrigin({ts.x / 2.f, ts.y / 2.f});
                // Randomize rotation and rotation speed
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> rotInitDis(0.f, 360.f);
                std::uniform_real_distribution<float> rotSpeedDis(-180.f, 180.f);
                rotation = rotInitDis(gen);
                rotationSpeed = rotSpeedDis(gen);
                sprite->setRotation(sf::degrees(rotation));
            }
        }
    }
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

        // Hover effect for super ore upgrade button
        if (!superOrePurchased)
        {
            if (superOreButton.getGlobalBounds().contains(mousePos))
            {
                if (money >= superOreCost)
                {
                    superOreButton.setFillColor(sf::Color(180, 130, 80));
                }
                else
                {
                    superOreButton.setFillColor(sf::Color(100, 60, 60));
                }
            }
            else
            {
                if (money >= superOreCost)
                {
                    superOreButton.setFillColor(sf::Color(150, 100, 60));
                }
                else
                {
                    superOreButton.setFillColor(sf::Color(80, 50, 50));
                }
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
    float binX = 410.f; // Default center position (adjusted for origin: 360 + 50)
    if (minerSprite.has_value())
    {
        binX = minerSprite->getPosition().x;
    }
    
    if (keyA && binX > 50.f) // 50 = half of miner width (100/2)
    {
        binX -= binSpeed * deltaTime;
        if (binX < 50.f) binX = 50.f;
        movingLeft = true;
    }
    else if (keyD && binX < 750.f) // 800 - 50 (half miner width)
    {
        binX += binSpeed * deltaTime;
        if (binX > 750.f) binX = 750.f;
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
                    float scaleX = 100.f / static_cast<float>(textureSize.x);
                    float scaleY = 120.f / static_cast<float>(textureSize.y);
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
                    float scaleX = 100.f / static_cast<float>(textureSize.x);
                    float scaleY = 120.f / static_cast<float>(textureSize.y);
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
        minerSprite->setPosition({binX, binY + 10.f}); // Adjusted for origin (120/2=60)
        // Flip horizontally when moving left
        // Use current texture to get size
        const sf::Texture& currentTexture = minerSprite->getTexture();
        sf::Vector2u textureSize = currentTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 100.f / static_cast<float>(textureSize.x);
            float scaleY = 120.f / static_cast<float>(textureSize.y);
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
        std::uniform_int_distribution<> typeDis(0, 99); // 0-79 = good, 80-99 = bad (20% chance for bad)
        std::uniform_int_distribution<> superOreDis(0, 99); // 0-9 = super ore (10% chance if purchased)
        
        // Find an inactive item or create a new one
        bool found = false;
        for (auto& item : fallingItems)
        {
            if (!item.active)
            {
                item.active = true;
                // Randomly assign type (80% good, 20% bad)
                int roll = typeDis(gen);
                ItemType itemType = (roll >= 80) ? ItemType::Bad : ItemType::Good;
                
                // Check for super ore BEFORE calling setType so texture scaling is correct
                item.isSuperOre = false;
                if (superOrePurchased && itemType == ItemType::Good)
                {
                    int superOreRoll = superOreDis(gen);
                    if (superOreRoll < 5) // 5% chance
                    {
                        item.isSuperOre = true;
                    }
                }
                
                // Now call setType with isSuperOre already set
                item.setType(itemType);
                if (item.sprite.has_value()) item.sprite->setPosition({posDis(gen), -30.f}); // Start above screen
                item.speed = itemFallSpeed;
                
                found = true;
                break;
            }
        }
        
        // If all items are active, add a new one
        if (!found)
        {
            FallingItem newItem;
            newItem.active = true;
            // Randomly assign type (80% good, 20% bad)
            int roll = typeDis(gen);
            ItemType itemType = (roll >= 80) ? ItemType::Bad : ItemType::Good;
            
            // Check for super ore BEFORE calling setType so texture scaling is correct
            newItem.isSuperOre = false;
            if (superOrePurchased && itemType == ItemType::Good)
            {
                int superOreRoll = superOreDis(gen);
                if (superOreRoll < 5) // 5% chance
                {
                    newItem.isSuperOre = true;
                }
            }
            
            // Now call setType with isSuperOre already set
            newItem.setType(itemType);
            if (newItem.sprite.has_value()) newItem.sprite->setPosition({posDis(gen), -30.f});
            newItem.speed = itemFallSpeed;
            
            fallingItems.push_back(newItem);
        }
    }

    // Update falling items
    sf::Vector2f binPos = {360.f, binY};
    sf::Vector2f binSize = {100.f, 120.f}; // Updated size for miner
    if (minerSprite.has_value())
    {
        binPos = minerSprite->getPosition();
        // Adjust position for origin (origin is at center)
        // Get actual texture size from current sprite
        const sf::Texture& currentTexture = minerSprite->getTexture();
        sf::Vector2u textureSize = currentTexture.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            float scaleX = 100.f / static_cast<float>(textureSize.x);
            float scaleY = 120.f / static_cast<float>(textureSize.y);
            binPos.x -= (textureSize.x * scaleX) / 2.f;
            binPos.y -= (textureSize.y * scaleY) / 2.f;
            binSize = {100.f, 120.f}; // Updated size
        }
    }
    
    for (auto& item : fallingItems)
    {
        if (item.active)
        {
            // Move item down
            if (item.sprite.has_value()) {
                sf::Vector2f pos = item.sprite->getPosition();
                pos.y += item.speed * deltaTime;
                item.sprite->setPosition(pos);
                // Apply rotation per-frame
                item.sprite->rotate(sf::degrees(item.rotationSpeed * deltaTime));
            }

            // Check collision with bin
            sf::FloatRect itemBounds = item.sprite.has_value() ? item.sprite->getGlobalBounds() : sf::FloatRect();
            // SFML3 Rect members are `position` and `size` — do manual intersection check
            float itemPosX = itemBounds.position.x;
            float itemPosY = itemBounds.position.y;
            float itemW = itemBounds.size.x;
            float itemH = itemBounds.size.y;
            bool collides = (itemPosX < binPos.x + binSize.x &&
                           itemPosX + itemW > binPos.x &&
                           itemPosY < binPos.y + binSize.y &&
                           itemPosY + itemH > binPos.y);
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
                    
                    // Check if this is a super ore item (100x value)
                    if (item.isSuperOre)
                    {
                        baseGain = 50;
                    }
                    else if (doubleClickPurchased)
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
            else
            {
                float itemY = 0.f;
                if (item.sprite.has_value())
                    itemY = item.sprite->getPosition().y;
                else
                    itemY = itemBounds.position.y;

                if (itemY > 600.f)
                {
                    item.active = false;
                }
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

    // Handle super ore upgrade purchase
    if (storeOpen && !superOrePurchased && superOreButton.getGlobalBounds().contains(mousePos))
    {
        if (money >= superOreCost)
        {
            money -= superOreCost;
            superOrePurchased = true;
            superOreButton.setFillColor(sf::Color(40, 40, 40));
            if (superOreButtonText.has_value())
            {
                superOreButtonText->setString("Super Ore\nPURCHASED");
            }
            return true;
        }
    }

    // Handle upgrades button
    if (!storeOpen) {
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

        // Draw super ore upgrade button
        if (!superOrePurchased)
        {
            window.draw(superOreButton);
            if (superOreButtonText.has_value())
            {
                // Update text color based on affordability
                if (money >= superOreCost)
                {
                    superOreButtonText->setFillColor(sf::Color::White);
                }
                else
                {
                    superOreButtonText->setFillColor(sf::Color(200, 150, 150));
                }
                window.draw(*superOreButtonText);
            }
        }
        else
        {
            // Draw purchased upgrade (grayed out)
            window.draw(superOreButton);
            if (superOreButtonText.has_value())
            {
                superOreButtonText->setFillColor(sf::Color(150, 150, 150));
                window.draw(*superOreButtonText);
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
                if (item.sprite.has_value())
                    window.draw(*item.sprite);
            }
        }
    }
}

// Music control for mine theme
void playMineMusic()
{
    if (mineMusicLoaded && !mineMusicPlaying)
    {
        mineMusic.play();
        mineMusicPlaying = true;
    }
}

void stopMineMusic()
{
    if (mineMusicLoaded && mineMusicPlaying)
    {
        mineMusic.stop();
        mineMusicPlaying = false;
    }
}
