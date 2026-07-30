#ifndef PHYSICS_2D_H
#define PHYSICS_2D_H

#include <Arduino.h>

/**
 * Axis-Aligned Bounding Box (AABB)
 */
struct Rect2D {
    float x, y, w, h;
};

/**
 * Bounding Circle
 */
struct Circle2D {
    float x, y, radius;
};

/**
 * Physics2D - 2D Collision Detection & Vector Math Utilities
 */
class Physics2D {
public:
    // Axis-Aligned Bounding Box (AABB) Collision Check
    static bool checkAABB(const Rect2D& a, const Rect2D& b) {
        return (a.x < b.x + b.w &&
                a.x + a.w > b.x &&
                a.y < b.y + b.h &&
                a.y + a.h > b.y);
    }

    // Circle-to-Circle Collision Check
    static bool checkCircleCollision(const Circle2D& c1, const Circle2D& c2) {
        float dx = c1.x - c2.x;
        float dy = c1.y - c2.y;
        float distSq = dx * dx + dy * dy;
        float radSum = c1.radius + c2.radius;
        return (distSq <= radSum * radSum);
    }

    // Point inside Rectangle Check
    static bool pointInRect(float px, float py, const Rect2D& rect) {
        return (px >= rect.x && px <= rect.x + rect.w &&
                py >= rect.y && py <= rect.y + rect.h);
    }

    // Circle and Box Intersection Check
    static bool checkCircleRect(const Circle2D& circle, const Rect2D& rect) {
        float closestX = constrain(circle.x, rect.x, rect.x + rect.w);
        float closestY = constrain(circle.y, rect.y, rect.y + rect.h);

        float dx = circle.x - closestX;
        float dy = circle.y - closestY;

        return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
    }
};

#endif // PHYSICS_2D_H
