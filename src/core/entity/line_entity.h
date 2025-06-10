// line_entity.h

#pragma once

#include "entity.h"

class LineEntity : public Entity {
public:
    float x1, y1, x2, y2;

    LineEntity(float x1_, float y1_, float x2_, float y2_)
        : x1(x1_), y1(y1_), x2(x2_), y2(y2_) {}

    std::string GetType() const override { return "Line"; }
};
