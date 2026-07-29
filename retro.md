# 🎮 Retro Game Engine Subsystem (v1.3)

The **Retro Game Engine Subsystem** provides a multi-layer 2D compositing engine, tileset graphics, 2D tilemaps, 2D affine matrix sprite transformations, particle effects, and an on-screen touch controller.

---

## 🏛️ Layer Classification & Depth Ordering

Layers are assigned explicit **`LayerRole`** depth categories. Background layers are guaranteed to render below everything else, while Foreground layers are guaranteed to render above level geometry and entities for tree canopy occlusion, darkness masks, and parallax effects!

| Layer Role | Z-Index Range | Description | Transform Capabilities |
|---|---|---|---|
| **`LayerRole::BACKGROUND`** | `-1000+` | Rendered **first** below everything | Translation (Scroll), Scale, Rotation |
| **`LayerRole::WORLD_MAP`** | `0+` | Level tilemaps & terrain | Translation (Scroll) |
| **`LayerRole::ENTITIES`** | `1000+` | Player, enemies, items, particles | Full Sprite Affine Matrix |
| **`LayerRole::FOREGROUND`** | `5000+` | Rendered **above** world & entities (tree canopies) | Parallax Scroll, Scale, Rotation |
| **`LayerRole::UI_OVERLAY`** | `10000+` | Top-level HUD, score, touch D-Pad | Fixed screen overlay |

---

## 🎨 2D Affine Sprite Transformations (`RetroSprite` & `AnimatedSprite`)

All sprites support full 2D matrix transformations:

- **Translation**: `sprite.setPosition(x, y)` / `sprite.move(dx, dy)`
- **Scaling**: `sprite.setScale(scaleX, scaleY)`
- **Rotation**: `sprite.setRotation(angleDeg)` / `sprite.rotate(deltaDeg)`
- **Skewing / Shear**: `sprite.setSkew(skewX, skewY)`
- **Pivot Point**: `sprite.setPivot(pivotX, pivotY)`
- **Frame Animation**: `AnimatedSprite` supports sprite sheet frame stepping, FPS, and auto-looping!

```cpp
#include "RetroSprite.h"

// Load hero sprite with black (0x0000) transparent color key
RetroSprite hero(ICON_POWER_16x16, 16, 16, 0x0000);

void updateGame(float deltaTime) {
    hero.setPosition(180, 120);
    hero.setScale(2.0f, 2.0f);
    hero.setRotation(45.0f); // 45 degrees rotation
    hero.setPivot(8, 8);    // Rotate around center
}
```

---

## 🗺️ TileSets & TileMaps (`TileSet.h` & `TileMap.h`)

```cpp
#include "TileSet.h"
#include "TileMap.h"

// Define 16x16 tileset atlas (32x32 total size)
TileSet tileSet(RETRO_TILESET_32x32, 16, 16, 32, 32);

// Define 16x10 level grid map
TileMap tileMap(&tileSet, RETRO_LEVEL_MAP, 16, 10);

void renderLevel(LGFX_Sprite* buffer, float scrollX) {
    // Render 2D level grid with smooth scroll offset
    tileMap.render(buffer, (int)scrollX, 0, 0x0000);
}
```

---

## 💥 Particle System & Virtual Touch Gamepad

- **`ParticleEngine`**: Emits particle explosion effects, sparks, and coin pickup dust (`particleEngine.emitExplosion(...)`).
- **`VirtualGamepad`**: Renders on-screen touch D-Pad (Up, Down, Left, Right) and Action Buttons (A, B) on touch screens.
