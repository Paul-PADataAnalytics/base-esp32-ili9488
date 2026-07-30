#ifndef PARTICLE_ENGINE_H
#define PARTICLE_ENGINE_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <vector>

/**
 * 2D Particle Primitive
 */
struct Particle {
    float x, y;
    float vx, vy;
    float ax, ay;
    float life;       // Seconds remaining
    float maxLife;
    float size;
    uint16_t startColor;
    uint16_t endColor;
};

/**
 * ParticleEngine - Retro Particle & FX Subsystem
 * 
 * Supports explosions, sparks, fire, magic dust, and coin pickup effects!
 */
class ParticleEngine {
private:
    std::vector<Particle> _particles;
    int _maxParticles;

public:
    ParticleEngine(int maxParticles = 100) : _maxParticles(maxParticles) {}

    void emitExplosion(float originX, float originY, int count, uint16_t color1, uint16_t color2) {
        for (int i = 0; i < count && _particles.size() < _maxParticles; i++) {
            float angle = random(0, 360) * DEG_TO_RAD;
            float speed = random(15, 80) / 10.0f;
            float life  = random(5, 15) / 10.0f;

            Particle p;
            p.x = originX;
            p.y = originY;
            p.vx = cos(angle) * speed;
            p.vy = sin(angle) * speed;
            p.ax = 0.0f;
            p.ay = 0.15f; // Gravity
            p.life = life;
            p.maxLife = life;
            p.size = random(2, 5);
            p.startColor = color1;
            p.endColor = color2;

            _particles.push_back(p);
        }
    }

    void update(float deltaTime) {
        for (auto it = _particles.begin(); it != _particles.end(); ) {
            it->life -= deltaTime;
            if (it->life <= 0.0f) {
                it = _particles.erase(it);
            } else {
                it->vx += it->ax;
                it->vy += it->ay;
                it->x  += it->vx;
                it->y  += it->vy;
                ++it;
            }
        }
    }

    void render(LovyanGFX* canvas) {
        if (!canvas) return;
        for (const auto& p : _particles) {
            float alpha = p.life / p.maxLife;
            uint16_t col = (alpha > 0.5f) ? p.startColor : p.endColor;
            canvas->fillCircle((int)p.x, (int)p.y, (int)p.size, col);
        }
    }
};

#endif // PARTICLE_ENGINE_H
