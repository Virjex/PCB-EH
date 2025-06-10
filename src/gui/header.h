// header.h

#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace GUI {
    void Init(GLFWwindow* window, VkInstance instance, VkDevice device, VkPhysicalDevice physical_device,
              uint32_t queue_family, VkQueue queue, VkRenderPass render_pass);

    void RenderHeader();

    void Cleanup();
}
