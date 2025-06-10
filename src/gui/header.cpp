// header.cpp

#include "header.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <stdexcept>

namespace GUI {

    static VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

    void Init(GLFWwindow* window, VkInstance instance, VkDevice device, VkPhysicalDevice physical_device,
              uint32_t queue_family, VkQueue queue, VkRenderPass render_pass)
    {
        // Create descriptor pool for ImGui
        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        };

        VkDescriptorPoolCreateInfo descriptor_pool_info = {};
        descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        descriptor_pool_info.maxSets = 1000;
        descriptor_pool_info.poolSizeCount = 1;
        descriptor_pool_info.pPoolSizes = pool_sizes;

        if (vkCreateDescriptorPool(device, &descriptor_pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create ImGui descriptor pool!");
        }

        // Init ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = physical_device;
        init_info.Device = device;
        init_info.QueueFamily = queue_family;
        init_info.Queue = queue;
        init_info.DescriptorPool = descriptor_pool;
        init_info.MinImageCount = 2; // matches Renderer::FRAME_COUNT
        init_info.ImageCount = 2;
        init_info.RenderPass = render_pass;

        ImGui_ImplVulkan_Init(&init_info);

        // Upload fonts
        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void RenderHeader() {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();

        // Header bar
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 40));
        ImGui::SetNextWindowBgAlpha(1.0f);

        ImGuiWindowFlags toolbar_flags = 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("Toolbar", nullptr, toolbar_flags);

        if (ImGui::Button("Select")) {
            // Handle select tool
        }

        ImGui::SameLine();

        if (ImGui::Button("Move")) {
            // Handle move tool
        }

        ImGui::SameLine();

        if (ImGui::Button("Rotate")) {
            // Handle rotate tool
        }

        ImGui::SameLine();

        if (ImGui::Button("Zoom")) {
            // Handle zoom tool
        }

        ImGui::End();

        // Finalize ImGui frame
        ImGui::Render();

        // Note: drawing happens in Renderer::RenderFrame()
        // ImGui_ImplVulkan_RenderDrawData() will be called from Renderer
    }

    void Cleanup() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Destroy descriptor pool
        // Note: device is already idle when calling Cleanup
        // (main.cpp calls vkDeviceWaitIdle before Cleanup)
        // You could pass device here if needed
    }

}
