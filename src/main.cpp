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


int main() {


    // ----- Init GLFW -----
    if (!glfwInit()) throw std::runtime_error("GLFW init failed!");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Vulkan CAD App", nullptr, nullptr);

    // ----- Init Renderer -----
    Renderer renderer;
    renderer.Init(window);

    // ----- Init GUI Header -----
    GUI::Init(window, renderer.GetInstance(), renderer.GetDevice(), renderer.GetPhysicalDevice(),
              renderer.GetQueueFamily(), renderer.GetQueue(), renderer.GetRenderPass());

    // ----- Main Loop -----

    CADDocument doc;

    auto line = std::make_shared<LineEntity>(0.0f, 0.0f, 100.0f, 100.0f);
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

