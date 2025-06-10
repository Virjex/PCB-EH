// cad_document.cpp

#include "cad_document.h"
#include "entity/line_entity.h"
#include "entity/circle_entity.h"
#include <memory>


CADDocument::CADDocument() {
    // Optionally create default layer
    AddLayer("Default");
}

void CADDocument::AddLayer(const std::string& name) {
    layers.push_back(Layer{ name });
}

void CADDocument::AddEntityToLayer(size_t layerIndex, std::shared_ptr<Entity> entity) {
    if (layerIndex >= layers.size()) return;
    layers[layerIndex].entities.push_back(entity);
}