// renderer.h

#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <glm/glm.hpp>

#include "core/cad_document.h"
#include "core/entity/line_entity.h"
#include "core/entity/circle_entity.h"


// Vertex structure for rendering
struct Vertex {
    float pos[2];
    float color[3];
};

class CADDocument; // Forward declare

class Renderer {
public:
    void Init(GLFWwindow* window);
    void RenderFrame(const CADDocument& doc);
    void Cleanup();
    void createPipeline();
    void RecreateSwapchain();
    VkInstance GetInstance() const { return instance; }
    VkDevice GetDevice() const { return device; }
    VkPhysicalDevice GetPhysicalDevice() const { return physical_device; }
    VkQueue GetQueue() const { return queue; }
    uint32_t GetQueueFamily() const { return queue_family; }
    VkRenderPass GetRenderPass() const { return render_pass; }
    VkShaderModule loadShaderModule(const std::string& filepath);
    void UpdateCamera(float zoom, glm::vec2 pan);

    // VkBuffer vertex_buffer{};
    // VkDeviceMemory vertex_buffer_memory{};
    VkBuffer vertexBufferLayers = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemoryLayers = VK_NULL_HANDLE;

    VkBuffer vertexBufferSelection;     // Dynamic selection layer
    VkDeviceMemory vertexMemorySelection;

    std::vector<Vertex> vertices;

private:
    // Dirty flags for efficient redraws
    bool sceneDirty = true;      // Set true if geometry changes
    bool selectionDirty = true;  // Set true if selection changes
    bool cameraDirty = true;     // Set true if zoom/pan changes
    GLFWwindow* window = nullptr;

    void createInstance();
    void pickPhysicalDevice();
    void createDevice();
    void createSwapchain(GLFWwindow* window);
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSelectionVertexBuffer(size_t size);
    void createSyncObjects();

    void check_vk_result(VkResult err);
    void createVertexBuffer(size_t size);
    void createLayerVertexBuffer(size_t size);
    void markAllDirty();

    VkInstance instance{};
    VkSurfaceKHR surface{};
    VkPhysicalDevice physical_device{};
    VkDevice device{};
    VkQueue queue{};
    uint32_t queue_family{};

    VkSwapchainKHR swapchain{};
    VkFormat swapchain_image_format{};
    VkExtent2D swapchain_extent{};
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    VkRenderPass render_pass{};
    std::vector<VkFramebuffer> framebuffers;

    VkCommandPool command_pool{};
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> image_acquired_semaphores;
    std::vector<VkSemaphore> render_complete_semaphores;
    std::vector<VkFence> frame_fences;

    uint32_t frame_index{0};

    VkPipelineLayout pipeline_layout{};
    VkPipeline pipeline{};


    static constexpr int FRAME_COUNT = 2;
};