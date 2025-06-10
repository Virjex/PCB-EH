// circle_entity.h

#pragma once

#include "entity.h"

class CircleEntity : public Entity {
public:
    float cx, cy;    // Center position
    float radius;    // Radius

    CircleEntity(float cx_, float cy_, float radius_)
        : cx(cx_), cy(cy_), radius(radius_) {}

    std::string GetType() const override { return "Circle"; }

    // Optional: You can add future methods here:
    // float GetArea() const;
    // void MoveTo(float newX, float newY);
};
