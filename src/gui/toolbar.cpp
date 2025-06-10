#include "toolbar.h"
#include "imgui.h"

namespace gui {

void Toolbar::render() {
    // Create a simple main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) { /* TODO: Add open action */ }
            if (ImGui::MenuItem("Save")) { /* TODO: Add save action */ }
            if (ImGui::MenuItem("Exit")) { /* TODO: Add exit action */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo")) { /* TODO: Add undo */ }
            if (ImGui::MenuItem("Redo")) { /* TODO: Add redo */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Zoom In")) { /* TODO: Add zoom */ }
            if (ImGui::MenuItem("Zoom Out")) { /* TODO: Add zoom */ }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

} // namespace gui
