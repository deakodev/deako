#include "VulkanUtils.h"
#include "dkpch.h"

#include "Deako/Core/Application.h"

#include "VulkanBase.h"

#include <GLFW/glfw3.h>

namespace Deako {

    static Ref<VulkanResources> vr = VulkanBase::GetResources();
    static Ref<VulkanSettings> vs = VulkanBase::GetSettings();



    namespace VulkanSwapchain {

        SwapchainDetails QuerySupport(VkPhysicalDevice physicalDevice)
        {
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vr->surface, &vr->swapchain.details.capabilities);

            uint32_t formatCount;  // swap chain formats
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vr->surface, &formatCount, nullptr);
            if (formatCount != 0)
            {
                vr->swapchain.details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vr->surface, &formatCount, vr->swapchain.details.formats.data());
            }

            uint32_t presentModeCount; // swap chain present modes
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vr->surface, &presentModeCount, nullptr);
            if (presentModeCount != 0)
            {
                vr->swapchain.details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vr->surface, &presentModeCount, vr->swapchain.details.presentModes.data());
            }

            return vr->swapchain.details;
        }

        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
        {
            std::vector<VkFormat> preferredImageFormats = {
                VK_FORMAT_B8G8R8A8_SRGB,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_FORMAT_A8B8G8R8_SRGB_PACK32,
            };

            for (auto& availableFormat : formats)
            {
                if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end())
                    return availableFormat;
            }

            return formats[0];
        }

        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
        {
            if (!vs->vsync)
            {
                for (const auto& availablePresentMode : presentModes)
                {
                    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) // better than v-sync, but high energy use
                        return availablePresentMode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR; // guaranteed to be available, vsync
        }

        VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
        {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            }
            else
            {
                GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();

                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }


    } // end namespace VulkanSwapchain
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanDevice {

        void FindQueueFamilies(VkPhysicalDevice physicalDevice)
        {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies)  // find queue families that are supported
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) vr->graphicsFamily = i;

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, vr->surface, &presentSupport);

                if (presentSupport) vr->presentFamily = i;
                if (vr->graphicsFamily.has_value() && vr->presentFamily.has_value()) break;

                i++;
            }
        }

        bool CheckExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<const char*> extensions)
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

            // checks if available extension array satisfies the required extensions array
            for (const auto& extension : availableExtensions)
                requiredExtensions.erase(extension.extensionName);

            return requiredExtensions.empty(); // if empty then required device extensions are supported
        }

        int RatePhysical(VkPhysicalDevice physicalDevice)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

            int score = 0;  // discrete GPUs have a significant performance advantage
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                score += 1000;

            // max possible size of textures affects graphics quality
            score += deviceProperties.limits.maxImageDimension2D;

            if (!deviceFeatures.geometryShader) // app can't function without geometry shaders
                score += 1000;

            FindQueueFamilies(physicalDevice); // look for families, must have required families
            bool queueFamiliesFound = vr->graphicsFamily.has_value() && vr->presentFamily.has_value();

            // must have adequate swap chain details
            SwapchainDetails swapChainSupport = VulkanSwapchain::QuerySupport(physicalDevice);
            bool swapChainAdequate = !swapChainSupport.formats.empty()
                && !swapChainSupport.presentModes.empty();

            bool suitable = swapChainAdequate && queueFamiliesFound;
            return suitable ? score : 0;
        }

        uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
        {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(vr->physicalDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            {
                if ((typeBits & 1) == 1)
                {
                    if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                    {
                        if (memTypeFound) *memTypeFound = true;
                        return i;
                    }
                }
                typeBits >>= 1;
            }

            if (memTypeFound)
            {
                *memTypeFound = false; return 0;
            }
            else
            {
                DK_CORE_ASSERT("Could not find a matching memory type!"); return 0;
            }
        }

    } // end namespace VulkanDevice
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanDepth {

        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
        {
            for (VkFormat format : candidates)
            {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(vr->physicalDevice, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                    return format;
                else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                    return format;
            }
            DK_CORE_ASSERT(false);
            return VK_FORMAT_UNDEFINED;
        }

        VkFormat FindFormat()
        {
            return FindSupportedFormat(
                { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM }, VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }

    } // end namespace VulkanDepth
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanImage {

        AllocatedImage Create(VkExtent3D extent, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, uint32_t mipLevels, VkImageType imageType)
        {
            AllocatedImage allocImage;
            allocImage.extent = extent;
            allocImage.format = format;

            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = imageType;
            imageInfo.format = format;
            imageInfo.extent = extent;
            imageInfo.mipLevels = mipLevels;
            imageInfo.arrayLayers = 1;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.samples = samples;
            imageInfo.usage = usage;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VkCR(vkCreateImage(vr->device, &imageInfo, nullptr, &allocImage.image));

            VkMemoryRequirements memReqs = {};
            vkGetImageMemoryRequirements(vr->device, allocImage.image, &memReqs);

            VkMemoryAllocateInfo memAllocInfo = {};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memAllocInfo.allocationSize = memReqs.size;

            VkBool32 lazyMemTypePresent;
            memAllocInfo.memoryTypeIndex = VulkanDevice::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, &lazyMemTypePresent);

            if (!lazyMemTypePresent)
                memAllocInfo.memoryTypeIndex = VulkanDevice::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkCR(vkAllocateMemory(vr->device, &memAllocInfo, nullptr, &allocImage.memory));

            vkBindImageMemory(vr->device, allocImage.image, allocImage.memory, 0);

            // if format is a depth format, need the correct aspect flag
            VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            if (format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                format == VK_FORMAT_D24_UNORM_S8_UINT)
                aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

            // create image view 
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = allocImage.image;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = format;
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            imageViewInfo.subresourceRange.aspectMask = aspectFlags;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;

            VkCR(vkCreateImageView(vr->device, &imageViewInfo, nullptr, &allocImage.view));

            return allocImage;
        }

        void Destroy(const AllocatedImage& allocImage)
        {
            vkDestroyImageView(vr->device, allocImage.view, nullptr);
            vkDestroyImage(vr->device, allocImage.image, nullptr);
            vkFreeMemory(vr->device, allocImage.memory, nullptr);
        }

        void Transition(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, uint32_t mipLevels, VkImageLayout currentLayout, VkImageLayout newLayout)
        {
            VkImageMemoryBarrier2 imageBarrier{};
            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            imageBarrier.pNext = nullptr;
            imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
            imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
            imageBarrier.oldLayout = currentLayout;
            imageBarrier.newLayout = newLayout;

            imageBarrier.subresourceRange.aspectMask =
                (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                    newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) ?
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
                VK_IMAGE_ASPECT_COLOR_BIT;;
            imageBarrier.subresourceRange.baseMipLevel = 0;
            imageBarrier.subresourceRange.levelCount = mipLevels;
            imageBarrier.subresourceRange.baseArrayLayer = 0;
            imageBarrier.subresourceRange.layerCount = 1;

            imageBarrier.image = image;

            VkDependencyInfo dependInfo{};
            dependInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependInfo.pNext = nullptr;
            dependInfo.imageMemoryBarrierCount = 1;
            dependInfo.pImageMemoryBarriers = &imageBarrier;

            vkCmdPipelineBarrier2KHR(commandBuffer, &dependInfo);
        }

    } // end namespace VulkanImage
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanBuffer {

        AllocatedBuffer Create(size_t allocSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
        {
            AllocatedBuffer allocBuffer;

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = allocSize;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VkCR(vkCreateBuffer(vr->device, &bufferInfo, nullptr, &allocBuffer.buffer));

            vkGetBufferMemoryRequirements(vr->device, allocBuffer.buffer, &allocBuffer.memReqs);

            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memAllocInfo.allocationSize = allocBuffer.memReqs.size;
            memAllocInfo.memoryTypeIndex = VulkanDevice::GetMemoryType(allocBuffer.memReqs.memoryTypeBits, flags);

            VkCR(vkAllocateMemory(vr->device, &memAllocInfo, nullptr, &allocBuffer.memory));
            VkCR(vkBindBufferMemory(vr->device, allocBuffer.buffer, allocBuffer.memory, 0));

            return allocBuffer;
        }

        void Destroy(const AllocatedBuffer& buffer)
        {
            vkFreeMemory(vr->device, buffer.memory, nullptr);
            vkDestroyBuffer(vr->device, buffer.buffer, nullptr);
        }

    } // end namespace VulkanBuffer
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanCommand {

        VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool)
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            VkCR(vkAllocateCommandBuffers(vr->device, &allocInfo, &commandBuffer));

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            VkCR(vkBeginCommandBuffer(commandBuffer, &beginInfo));

            return commandBuffer;
        }

        void EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer)
        {
            VkCR(vkEndCommandBuffer(commandBuffer));

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            VkCR(vkQueueSubmit(vr->graphicsQueue, 1, &submitInfo, nullptr));

            vkQueueWaitIdle(vr->graphicsQueue);

            vkFreeCommandBuffers(vr->device, commandPool, 1, &commandBuffer);
        }

    } // end namespace VulkanImage
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanLoad {

        void Scene(std::string filename)
        {
            DK_CORE_INFO("Loading Scene <{0}>", filename.c_str());

            vr->models.scene.Destroy();
            vr->models.animationIndex = 0;
            vr->models.animationTimer = 0.0f;

            auto tStart = std::chrono::high_resolution_clock::now();

            vr->models.scene.LoadFromFile(filename);

            // We place all materials for the current scene into a shader storage buffer stored on the GPU
            // This allows us to use arbitrary large material defintions
            // The fragment shader then get's the index into this material array from a push constant set per primitive
            CreateMaterialBuffer();

            auto tLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();

            DK_CORE_INFO("Loading Scene took {0} ms", tLoad);

            for (auto& ext : vr->models.scene.extensions)
            {   // check and list unsupported extensions
                if (std::find(supportedGLTFExts.begin(), supportedGLTFExts.end(), ext) == supportedGLTFExts.end())
                    DK_CORE_WARN("Unsupported extension {0}. Scene may not display as intended.", ext);
            }
        }

    } // end namespace VulkanLoad
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    namespace VulkanShader {

        std::vector<char> ReadShaderFile(const std::string& path)
        {
            // Start reading at the end of the file (ate) to determine file size and read as binary file (binary)
            std::ifstream file(path, std::ios::ate | std::ios::binary);

            DK_CORE_ASSERT(file.is_open(), "Failed to open shader file!");

            size_t fileSize = (size_t)file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0); // go back to the beginning of the file
            file.read(buffer.data(), fileSize); // fill buffer with bytes

            file.close();

            return buffer;
        }

        VkShaderModule CreateShaderModule(const std::string& filename)
        {
            DK_CORE_INFO("Reading Shader <{0}>", filename.c_str());
            const std::string path = vs->assetPath + filename;

            auto shaderCode = ReadShaderFile(path);

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = shaderCode.size();
            // Size of the bytecode is in bytes, but bytecode pointer is a uint32_t pointer rather than a char pointer, thus need to reinterpret cast shader data
            createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

            VkShaderModule shaderModule;
            VkResult result = vkCreateShaderModule(vr->device, &createInfo, nullptr, &shaderModule);
            DK_CORE_ASSERT(!result);

            return shaderModule;
        }

    } // end namespace VulkanShader
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



    void GenerateBRDFLookUpTable()
    {
        auto tStart = std::chrono::high_resolution_clock::now();

        const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
        const uint32_t dim = 512;

        AllocatedImage& lutBrdfImage = vr->textures.lutBrdf.GetImage();
        VkSampler& lutBrdfSampler = vr->textures.lutBrdf.GetSampler();
        VkDescriptorImageInfo& lutBrdfDescriptor = vr->textures.lutBrdf.GetDescriptor();

        lutBrdfImage =
            VulkanImage::Create({ dim, dim, 1 }, format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);

        // sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VkCR(vkCreateSampler(vr->device, &samplerInfo, nullptr, &lutBrdfSampler));

        // FB, Att, RP, Pipe, etc.
        VkAttachmentDescription attachment{};
        // Color attachment
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;

        // use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // renderpass
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies.data();

        VkRenderPass renderPass;
        VkCR(vkCreateRenderPass(vr->device, &renderPassInfo, nullptr, &renderPass));

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &lutBrdfImage.view;
        framebufferInfo.width = dim;
        framebufferInfo.height = dim;
        framebufferInfo.layers = 1;

        VkFramebuffer framebuffer;
        VkCR(vkCreateFramebuffer(vr->device, &framebufferInfo, nullptr, &framebuffer));

        // desriptors
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        VkCR(vkCreateDescriptorSetLayout(vr->device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));

        // pipeline layout
        VkPipelineLayout pipelineLayout;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        VkCR(vkCreatePipelineLayout(vr->device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        // pipeline states
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterization{};
        rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization.cullMode = VK_CULL_MODE_NONE;
        rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &blendAttachment;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.front = depthStencil.back;
        depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = dynamicStateEnables.data();
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

        VkPipelineVertexInputStateCreateInfo emptyInput{};
        emptyInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // shaders
        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].pName = "main";
        shaderStages[0].module = VulkanShader::CreateShaderModule("shaders/bin/genbrdflut.vert.spv");

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].pName = "main";
        shaderStages[1].module = VulkanShader::CreateShaderModule("shaders/bin/genbrdflut.frag.spv");

        // pipline info
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pVertexInputState = &emptyInput;
        pipelineInfo.pRasterizationState = &rasterization;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages.data();

        VkPipeline pipeline;
        VkCR(vkCreateGraphicsPipelines(vr->device, vr->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

        for (auto shaderStage : shaderStages)
            vkDestroyShaderModule(vr->device, shaderStage.module, nullptr);

        // render
        VkClearValue clearValues[1];
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.extent = { dim, dim };
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.framebuffer = framebuffer;

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.width = (float)dim;
        viewport.height = (float)dim;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.extent.width = dim;
        scissor.extent.height = dim;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);

        VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);

        vkDestroyPipeline(vr->device, pipeline, nullptr);
        vkDestroyPipelineLayout(vr->device, pipelineLayout, nullptr);
        vkDestroyRenderPass(vr->device, renderPass, nullptr);
        vkDestroyFramebuffer(vr->device, framebuffer, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, descriptorSetLayout, nullptr);

        lutBrdfDescriptor.imageView = lutBrdfImage.view;
        lutBrdfDescriptor.sampler = lutBrdfSampler;
        lutBrdfDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        DK_CORE_INFO("Generating BRDF LUT took {0} ms", tDiff);
    }

}
