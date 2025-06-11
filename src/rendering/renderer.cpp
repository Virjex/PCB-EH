// renderer.cpp

#include "rendering/renderer.h"
#include "core/cad_document.h"
#include "core/entity/line_entity.h"
#include "core/entity/circle_entity.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 viewProjMatrix = glm::mat4(1.0f);

void Renderer::check_vk_result(VkResult err) {
    if (err == 0) return;
    std::cerr << "[Vulkan Error] VkResult = " << err << std::endl;
    if (err < 0) abort();
}

void Renderer::Init(GLFWwindow* w) {
    window = w;
    createInstance();
    check_vk_result(glfwCreateWindowSurface(instance, window, nullptr, &surface));

    pickPhysicalDevice();
    createDevice();
    createSwapchain(window);
    createRenderPass();
    createPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}

void Renderer::RecreateSwapchain() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    for (auto framebuffer : framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    for (auto view : swapchain_image_views)
        vkDestroyImageView(device, view, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    createSwapchain(window);
    createRenderPass();
    createPipeline();
    createFramebuffers();
    markAllDirty();
}


void Renderer::createPipeline() {
    // Load shaders
    VkShaderModule vert_shader_module = loadShaderModule("src/rendering/shaders/vert.spv");
    VkShaderModule frag_shader_module = loadShaderModule("src/rendering/shaders/frag.spv");

    // Shader stages
    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(glm::mat4);

    VkPipelineShaderStageCreateInfo vert_stage_info = {};
    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_shader_module;
    vert_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage_info = {};
    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_shader_module;
    frag_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = { vert_stage_info, frag_stage_info };

    // Vertex input
    VkVertexInputBindingDescription binding_desc = {};
    binding_desc.binding = 0;
    binding_desc.stride = sizeof(Vertex);
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attr_desc[2] = {};
    attr_desc[0].binding = 0;
    attr_desc[0].location = 0;
    attr_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
    attr_desc[0].offset = offsetof(Vertex, pos);

    attr_desc[1].binding = 0;
    attr_desc[1].location = 1;
    attr_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attr_desc[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = 2;
    vertex_input_info.pVertexAttributeDescriptions = attr_desc;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchain_extent.width;
    viewport.height = (float)swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain_extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Color blend
    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;
    check_vk_result(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout));


    // Final pipeline
    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.layout = pipeline_layout;
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;

    check_vk_result(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline));

    // Destroy shader modules (no longer needed after pipeline is created)
    vkDestroyShaderModule(device, vert_shader_module, nullptr);
    vkDestroyShaderModule(device, frag_shader_module, nullptr);
}

void Renderer::RenderFrame(const CADDocument& doc) {
    vkWaitForFences(device, 1, &frame_fences[frame_index], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &frame_fences[frame_index]);

    uint32_t image_index;
    VkResult acquire_result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
        image_acquired_semaphores[frame_index], VK_NULL_HANDLE, &image_index);

    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    }

    VkCommandBuffer cmd = command_buffers[frame_index];

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    check_vk_result(vkBeginCommandBuffer(cmd, &begin_info));

    VkClearValue clear_value = { 0.1f, 0.1f, 0.1f, 1.0f };

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = framebuffers[image_index];
    render_pass_info.renderArea.extent = swapchain_extent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;

    if (sceneDirty) {
        vertices.clear();
        for (const auto& layer : doc.GetLayers()) {
            if (!layer.visible) continue;
            for (const auto& entity : layer.entities) {
                std::string type = entity->GetType();
                if (type == "Line") {
                    const LineEntity* line = dynamic_cast<const LineEntity*>(entity.get());
                    if (line) {
                        vertices.push_back({ { line->x1, line->y1 }, { 1.0f, 0.0f, 0.0f } });
                        vertices.push_back({ { line->x2, line->y2 }, { 1.0f, 0.0f, 0.0f } });
                    }
                } else if (type == "Circle") {
                    const CircleEntity* circle = dynamic_cast<const CircleEntity*>(entity.get());
                    if (circle) {
                        constexpr int SEGMENTS = 64;
                        for (int i = 0; i < SEGMENTS; ++i) {
                            float angle1 = (float)i / SEGMENTS * 2.0f * 3.1415926f;
                            float angle2 = (float)(i + 1) / SEGMENTS * 2.0f * 3.1415926f;
                            float x1 = circle->cx + cos(angle1) * circle->radius;
                            float y1 = circle->cy + sin(angle1) * circle->radius;
                            float x2 = circle->cx + cos(angle2) * circle->radius;
                            float y2 = circle->cy + sin(angle2) * circle->radius;
                            vertices.push_back({ { x1, y1 }, { 0.0f, 1.0f, 0.0f } });
                            vertices.push_back({ { x2, y2 }, { 0.0f, 1.0f, 0.0f } });
                        }
                    }
                }
            }
        }

        size_t buffer_size = vertices.size() * sizeof(Vertex);
        createVertexBuffer(buffer_size);

        void* data;
        check_vk_result(vkMapMemory(device, vertexMemoryLayers, 0, buffer_size, 0, &data));
        memcpy(data, vertices.data(), buffer_size);
        vkUnmapMemory(device, vertexMemoryLayers);

        sceneDirty = false;
    }

    vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBufferLayers, offsets);
    vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &viewProjMatrix);
    vkCmdDraw(cmd, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRenderPass(cmd);
    check_vk_result(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_acquired_semaphores[frame_index];
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_complete_semaphores[frame_index];
    check_vk_result(vkQueueSubmit(queue, 1, &submit_info, frame_fences[frame_index]));

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphores[frame_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain;
    present_info.pImageIndices = &image_index;

    VkResult present_result = vkQueuePresentKHR(queue, &present_info);
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapchain();
    } else {
        check_vk_result(present_result);
    }

    frame_index = (frame_index + 1) % FRAME_COUNT;
}


void Renderer::Cleanup() {
    vkDeviceWaitIdle(device);

    for (auto framebuffer : framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    for (auto view : swapchain_image_views)
        vkDestroyImageView(device, view, nullptr);

    vkDestroyRenderPass(device, render_pass, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyCommandPool(device, command_pool, nullptr);

    for (int i = 0; i < FRAME_COUNT; i++) {
        vkDestroySemaphore(device, image_acquired_semaphores[i], nullptr);
        vkDestroySemaphore(device, render_complete_semaphores[i], nullptr);
        vkDestroyFence(device, frame_fences[i], nullptr);
    }

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void Renderer::createVertexBuffer(size_t size) {
    // Clean up previous buffer if it exists
    if (vertexBufferLayers != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, vertexBufferLayers, nullptr);
        vertexBufferLayers = VK_NULL_HANDLE;
    }
    if (vertexMemoryLayers != VK_NULL_HANDLE) {
        vkFreeMemory(device, vertexMemoryLayers, nullptr);
        vertexMemoryLayers = VK_NULL_HANDLE;
    }

    // Create the buffer
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    check_vk_result(vkCreateBuffer(device, &buffer_info, nullptr, &vertexBufferLayers));

    // Get memory requirements
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device, vertexBufferLayers, &mem_requirements);

    // Allocate memory
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    uint32_t memory_type_index = (uint32_t)-1;
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((mem_requirements.memoryTypeBits & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
            memory_type_index = i;
            break;
        }
    }

    if (memory_type_index == (uint32_t)-1) {
        throw std::runtime_error("Failed to find suitable memory type for vertex buffer!");
    }

    alloc_info.memoryTypeIndex = memory_type_index;

    check_vk_result(vkAllocateMemory(device, &alloc_info, nullptr, &vertexMemoryLayers));
    check_vk_result(vkBindBufferMemory(device, vertexBufferLayers, vertexMemoryLayers, 0));
}

void Renderer::UpdateCamera(float zoom, glm::vec2 pan) {
    glm::mat4 proj = glm::ortho(-zoom, zoom, -zoom, zoom, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(pan, 0.0f));
    viewProjMatrix = proj * view;
    cameraDirty = true;
}

VkShaderModule Renderer::loadShaderModule(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filepath);
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = buffer.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule shader_module;
    check_vk_result(vkCreateShaderModule(device, &create_info, nullptr, &shader_module));
    return shader_module;
}


void Renderer::createInstance() {
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Vulkan CAD App";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    uint32_t glfw_ext_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = glfw_ext_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    check_vk_result(vkCreateInstance(&create_info, nullptr, &instance));
}

void Renderer::createSwapchain(GLFWwindow* window) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());

    VkSurfaceFormatKHR surface_format = formats[0];
    swapchain_image_format = surface_format.format;

    if (capabilities.currentExtent.width != UINT32_MAX) {
        swapchain_extent = capabilities.currentExtent;
    } else {
        int width = 0, height = 0;
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
        swapchain_extent.width = std::clamp((uint32_t)width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swapchain_extent.height = std::clamp((uint32_t)height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface = surface;
    swapchain_info.minImageCount = FRAME_COUNT;
    swapchain_info.imageFormat = swapchain_image_format;
    swapchain_info.imageColorSpace = surface_format.colorSpace;
    swapchain_info.imageExtent = swapchain_extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.preTransform = capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.oldSwapchain = VK_NULL_HANDLE;

    check_vk_result(vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain));

    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());

    swapchain_image_views.resize(image_count);
    for (size_t i = 0; i < image_count; i++) {
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = swapchain_images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = swapchain_image_format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        check_vk_result(vkCreateImageView(device, &view_info, nullptr, &swapchain_image_views[i]));
    }
}

void Renderer::createRenderPass() {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    check_vk_result(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass));
}

void Renderer::createFramebuffers() {
    framebuffers.resize(swapchain_image_views.size());
    for (size_t i = 0; i < swapchain_image_views.size(); i++) {
        VkImageView attachments[1] = { swapchain_image_views[i] };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = swapchain_extent.width;
        framebuffer_info.height = swapchain_extent.height;
        framebuffer_info.layers = 1;

        check_vk_result(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]));
    }
}

void Renderer::createCommandPool() {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family;

    check_vk_result(vkCreateCommandPool(device, &pool_info, nullptr, &command_pool));

    createCommandBuffers();
}

void Renderer::createCommandBuffers() {
    command_buffers.resize(FRAME_COUNT);

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = FRAME_COUNT;

    check_vk_result(vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()));
}


void Renderer::createSyncObjects() {
    image_acquired_semaphores.resize(FRAME_COUNT);
    render_complete_semaphores.resize(FRAME_COUNT);
    frame_fences.resize(FRAME_COUNT);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < FRAME_COUNT; i++) {
        check_vk_result(vkCreateSemaphore(device, &semaphore_info, nullptr, &image_acquired_semaphores[i]));
        check_vk_result(vkCreateSemaphore(device, &semaphore_info, nullptr, &render_complete_semaphores[i]));
        check_vk_result(vkCreateFence(device, &fence_info, nullptr, &frame_fences[i]));
    }
}


void Renderer::pickPhysicalDevice() {
    uint32_t gpu_count = 0;
    vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
    std::vector<VkPhysicalDevice> gpus(gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());
    physical_device = gpus[0];

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    queue_family = (uint32_t)-1;
    for (uint32_t i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && present_support) {
            queue_family = i;
            break;
        }
    }

    if (queue_family == (uint32_t)-1)
        throw std::runtime_error("No suitable queue family found!");
}

void Renderer::createDevice() {
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = queue_family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    const char* device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = 1;
    device_info.ppEnabledExtensionNames = device_extensions;

    check_vk_result(vkCreateDevice(physical_device, &device_info, nullptr, &device));
    vkGetDeviceQueue(device, queue_family, 0, &queue);
}

void Renderer::markAllDirty() {
    sceneDirty = true;
    selectionDirty = true;
    cameraDirty = true;
}