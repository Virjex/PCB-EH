// main.cpp

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>

#include "gui/header.h"
#include "rendering/renderer.h"
#include "core/cad_document.h"
#include "core/entity/line_entity.h"
#include "core/entity/circle_entity.h"

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

// Global camera state
glm::vec2 pan = glm::vec2(0.0f);
float zoom = 100.0f;
bool dragging = false;
double lastMouseX = 0.0, lastMouseY = 0.0;
Renderer* g_renderer = nullptr;

// Mouse scroll to zoom
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoom *= (yoffset > 0) ? 0.9f : 1.1f;
    zoom = std::clamp(zoom, 1.0f, 1000.0f);
    if (g_renderer) g_renderer->UpdateCamera(zoom, pan);
}

// Left click drag to pan
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            dragging = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        } else if (action == GLFW_RELEASE) {
            dragging = false;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!dragging) return;

    double dx = xpos - lastMouseX;
    double dy = ypos - lastMouseY;

    lastMouseX = xpos;
    lastMouseY = ypos;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    float aspect = (float)width / height;
    pan.x += dx * (0.95f * zoom * aspect / width);
    pan.y += dy * (0.95f * zoom / height);

    if (g_renderer) g_renderer->UpdateCamera(zoom, pan);
}

int main() {
    if (!glfwInit()) throw std::runtime_error("GLFW init failed!");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "TCADO", nullptr, nullptr);

    // Set up input
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    // ----- Init Renderer -----
    Renderer renderer;
    g_renderer = &renderer; // needed for input callbacks
    renderer.Init(window);
    renderer.UpdateCamera(zoom, pan);

    // ----- Init GUI Header -----
    GUI::Init(window, renderer.GetInstance(), renderer.GetDevice(), renderer.GetPhysicalDevice(),
              renderer.GetQueueFamily(), renderer.GetQueue(), renderer.GetRenderPass());

    // ----- Create CAD Document -----
    CADDocument doc;

    auto line = std::make_shared<LineEntity>(0.0f, 0.0f, 25.0f, 100.0f);
    doc.AddEntityToLayer(0, line);
    line = std::make_shared<LineEntity>(0.0f, 0.0f, 25.0f, -100.0f);
    doc.AddEntityToLayer(0, line);
    auto circle = std::make_shared<CircleEntity>(50.0f, 50.0f, 25.0f);
    doc.AddEntityToLayer(0, circle);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        GUI::RenderHeader();
        renderer.RenderFrame(doc);
    }

    // ----- Cleanup -----
    vkDeviceWaitIdle(renderer.GetDevice());

    GUI::Cleanup();
    renderer.Cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
