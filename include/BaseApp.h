#ifndef BASE_APP_H
#define BASE_APP_H

// Forward declaration
class GFXContext;

/**
 * Base Application Interface
 * 
 * Inherit from this class to build new programs.
 * Hardware details (SPI pins, DMA, LCD init) are completely isolated.
 */
class BaseApp {
public:
    virtual ~BaseApp() {}

    // Called once when application initializes
    virtual void setup(GFXContext& gfx) = 0;

    // Called every frame to update state & physics
    // @param deltaTime Time elapsed in seconds since last frame
    virtual void update(float deltaTime) = 0;

    // Called every frame to render graphics
    virtual void render(GFXContext& gfx) = 0;
};

#endif // BASE_APP_H
