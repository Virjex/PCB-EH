// cad_document.h

#pragma once

#include <vector>
#include <string>
#include <memory>

class Entity; // Forward declaration (you'll create this!)

class Layer {
public:
    std::string name;
    bool visible = true;

    std::vector<std::shared_ptr<Entity>> entities;
};

class CADDocument {
public:
    CADDocument();

    void AddLayer(const std::string& name);
    void AddEntityToLayer(size_t layerIndex, std::shared_ptr<Entity> entity);

    const std::vector<Layer>& GetLayers() const { return layers; }

    // Future:
    // void Save(const std::string& path);
    // void Load(const std::string& path);

private:
    std::vector<Layer> layers;
};
