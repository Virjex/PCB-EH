// entity.h

#pragma once

#include <string>

class Entity {
public:
    virtual ~Entity() = default;

    virtual std::string GetType() const = 0;  // <--- THIS MUST EXIST!
};
