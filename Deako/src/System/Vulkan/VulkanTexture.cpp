#include "VulkanTexture.h"
#include "dkpch.h"

#include "VulkanBase.h"

#include <gli/gli.hpp>

namespace Deako {

    static Ref<VulkanResources> vr = VulkanBase::GetResources();
    static Ref<VulkanSettings> vs = VulkanBase::GetSettings();

    void LoadEnvironment(std::string filename)
    {
        DK_CORE_INFO("Loading Environment <{0}>", filename.c_str());

        if (vr->textures.environmentCube.GetImage().image)
        {
            vr->textures.environmentCube.Destroy();
            vr->textures.irradianceCube.Destroy();
            vr->textures.prefilteredCube.Destroy();
        }

        vr->textures.environmentCube.LoadFromFile(filename, VK_FORMAT_R16G16B16A16_SFLOAT);
        vr->textures.irradianceCube.GenerateCubeMap();
        vr->textures.prefilteredCube.GenerateCubeMap();
    }

    void Texture::UpdateDescriptor()
    {
        m_Descriptor.sampler = m_Sampler;
        m_Descriptor.imageView = m_Image.view;
        m_Descriptor.imageLayout = m_ImageLayout;
    }

    void Texture::Destroy()
    {
        if (m_Sampler) vkDestroySampler(vr->device, m_Sampler, nullptr);
        VulkanImage::Destroy(m_Image);
    }

    void Texture2D::LoadFromFile(std::string filename, VkFormat format, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
    {
        DK_CORE_INFO("Loading Texture2D <{0}>", filename.c_str());

        const std::string path = vs->assetPath + filename;
        gli::texture2d texture(gli::load(path.c_str()));
        DK_CORE_ASSERT(!texture.empty(), "Unable to load texture from file!");

        uint32_t width = static_cast<uint32_t>(texture[0].extent().x);
        uint32_t height = static_cast<uint32_t>(texture[0].extent().y);
        m_ImageLayout = imageLayout;
        m_MipLevels = static_cast<uint32_t>(texture.levels());

        // create host-visible staging buffer that contains the raw image data
        AllocatedBuffer staging =
            VulkanBuffer::Create(texture.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // copy texture data into staging buffer
        VkCR(vkMapMemory(vr->device, staging.memory, 0, staging.memReqs.size, 0, &staging.mapped));
        memcpy(staging.mapped, texture.data(), texture.size());
        vkUnmapMemory(vr->device, staging.memory);

        // create optimal tiled target image
        m_Image =
            VulkanImage::Create({ width, height, 1 }, format, VK_SAMPLE_COUNT_1_BIT, imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT, m_MipLevels);

        // setup buffer copy regions for each mip level
        std::vector<VkBufferImageCopy> copyRegions;
        uint32_t offset = 0;

        for (uint32_t i = 0; i < m_MipLevels; i++)
        {
            VkBufferImageCopy copyRegion = {};
            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = i;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageExtent.width = static_cast<uint32_t>(texture[i].extent().x);
            copyRegion.imageExtent.height = static_cast<uint32_t>(texture[i].extent().y);
            copyRegion.imageExtent.depth = 1;
            copyRegion.bufferOffset = offset;

            copyRegions.push_back(copyRegion);
            offset += static_cast<uint32_t>(texture[i].size());
        }

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

        VulkanImage::Transition(commandBuffer, m_Image.image, m_Image.format, m_MipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // copy mip levels from staging buffer
        vkCmdCopyBufferToImage(commandBuffer, staging.buffer, m_Image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(copyRegions.size()), copyRegions.data());

        VulkanImage::Transition(commandBuffer, m_Image.image, m_Image.format, m_MipLevels, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_ImageLayout);

        VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);

        VulkanBuffer::Destroy(staging);

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vr->physicalDevice, &deviceProperties);

        // create the sampler
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(m_MipLevels);
        samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VkCR(vkCreateSampler(vr->device, &samplerInfo, nullptr, &m_Sampler));

        UpdateDescriptor();
    }

    // loads image for texture. Supports jpg, png, embedded and external files as well as external KTX2 files with basis universal texture compression
    void Texture2D::LoadFromGLTFImage(tinygltf::Image& gltfimage, std::string path, TextureSampler textureSampler)
    {
        bool isKtx2 = false; // KTX2 files need to be handled explicitly
        if (gltfimage.uri.find_last_of(".") != std::string::npos)
        {
            if (gltfimage.uri.substr(gltfimage.uri.find_last_of(".") + 1) == "ktx2")
                isKtx2 = true;
        }

        m_Image.format = VK_FORMAT_R8G8B8A8_UNORM;

        if (isKtx2)
        {  // image is KTX2 using basis universal compression, needs to be loaded from disk and will be transcoded to a native GPU format
            basist::ktx2_transcoder ktxTranscoder;
            const std::string filename = path + "\\" + gltfimage.uri;
            std::ifstream ifs(filename, std::ios::binary | std::ios::in | std::ios::ate);
            if (!ifs.is_open())
                throw std::runtime_error("Could not load requested image file: " + filename);

            uint32_t inputDataSize = static_cast<uint32_t>(ifs.tellg());
            char* inputData = new char[inputDataSize];

            ifs.seekg(0, std::ios::beg);
            ifs.read(inputData, inputDataSize);

            bool success = ktxTranscoder.init(inputData, inputDataSize);
            if (!success)
                throw std::runtime_error("Could not initialize ktx2 transcoder for image file: " + filename);

            // select target format based on device features (use uncompressed if none supported)
            auto targetFormat = basist::transcoder_texture_format::cTFRGBA32;

            auto FormatSupported = [](VkFormat format)
                {
                    VkFormatProperties formatProperties;
                    vkGetPhysicalDeviceFormatProperties(vr->physicalDevice, format, &formatProperties);
                    return ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) && (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT));
                };

            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(vr->physicalDevice, &deviceFeatures);

            if (deviceFeatures.textureCompressionBC)
            {   // BC7 is the preferred block compression if available
                if (FormatSupported(VK_FORMAT_BC7_UNORM_BLOCK))
                {
                    targetFormat = basist::transcoder_texture_format::cTFBC7_RGBA;
                    m_Image.format = VK_FORMAT_BC7_UNORM_BLOCK;
                }
                else
                {
                    if (FormatSupported(VK_FORMAT_BC3_SRGB_BLOCK))
                    {
                        targetFormat = basist::transcoder_texture_format::cTFBC3_RGBA;
                        m_Image.format = VK_FORMAT_BC3_SRGB_BLOCK;
                    }
                }
            }

            if (deviceFeatures.textureCompressionASTC_LDR)
            {    // adaptive scalable texture compression
                if (FormatSupported(VK_FORMAT_ASTC_4x4_SRGB_BLOCK))
                {
                    targetFormat = basist::transcoder_texture_format::cTFASTC_4x4_RGBA;
                    m_Image.format = VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
                }
            }

            // ericsson texture compression
            if (deviceFeatures.textureCompressionETC2)
            {
                if (FormatSupported(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK))
                {
                    targetFormat = basist::transcoder_texture_format::cTFETC2_RGBA;
                    m_Image.format = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
                }
            }
            // TODO: PowerVR texture compression support needs to be checked via an extension (VK_IMG_FORMAT_PVRTC_EXTENSION_NAME)

            const bool targetFormatIsUncompressed = basist::basis_transcoder_format_is_uncompressed(targetFormat);

            std::vector<basist::ktx2_image_level_info> levelInfos(ktxTranscoder.get_levels());
            m_MipLevels = ktxTranscoder.get_levels();

            // query image level information that we need later on for several calculations
            // we only support 2D images (no cube maps or layered images)
            for (uint32_t i = 0; i < m_MipLevels; i++)
                ktxTranscoder.get_image_level_info(levelInfos[i], i, 0, 0);

            uint32_t width = levelInfos[0].m_orig_width;;
            uint32_t height = levelInfos[0].m_orig_height;
            m_Image.extent = { width, height, 1 };

            // create one staging buffer large enough to hold all uncompressed image levels
            const uint32_t bytesPerBlockOrPixel = basist::basis_get_bytes_per_block_or_pixel(targetFormat);
            uint32_t numBlocksOrPixels = 0;
            VkDeviceSize totalBufferSize = 0;
            for (uint32_t i = 0; i < m_MipLevels; i++)
            {   // size calculations differ for compressed/uncompressed formats
                numBlocksOrPixels = targetFormatIsUncompressed ? levelInfos[i].m_orig_width * levelInfos[i].m_orig_height : levelInfos[i].m_total_blocks;
                totalBufferSize += numBlocksOrPixels * bytesPerBlockOrPixel;
            }

            // create host-visible staging buffer that contains the raw image data
            AllocatedBuffer staging =
                VulkanBuffer::Create(totalBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VkCR(vkMapMemory(vr->device, staging.memory, 0, staging.memReqs.size, 0, &staging.mapped));

            unsigned char* buffer = new unsigned char[totalBufferSize];
            unsigned char* bufferPtr = &buffer[0];

            success = ktxTranscoder.start_transcoding();
            if (!success)
                throw std::runtime_error("Could not start transcoding for image file " + filename);

            // transcode all mip levels into the staging buffer
            for (uint32_t i = 0; i < m_MipLevels; i++)
            {
                // Size calculations differ for compressed/uncompressed formats
                numBlocksOrPixels = targetFormatIsUncompressed ? levelInfos[i].m_orig_width * levelInfos[i].m_orig_height : levelInfos[i].m_total_blocks;
                uint32_t outputSize = numBlocksOrPixels * bytesPerBlockOrPixel;
                if (!ktxTranscoder.transcode_image_level(i, 0, 0, bufferPtr, numBlocksOrPixels, targetFormat, 0)) {
                    throw std::runtime_error("Could not transcode the requested image file " + filename);
                }
                bufferPtr += outputSize;
            }

            memcpy(staging.mapped, buffer, totalBufferSize);

            // create optimal tiled target image
            m_Image =
                VulkanImage::Create(m_Image.extent, m_Image.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_MipLevels);

            VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

            VulkanImage::Transition(commandBuffer, m_Image.image, m_Image.format, m_MipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            // transcode and copy all image levels
            VkDeviceSize bufferOffset = 0;
            for (uint32_t i = 0; i < m_MipLevels; i++)
            {   // size calculations differ for compressed/uncompressed formats
                numBlocksOrPixels = targetFormatIsUncompressed ? levelInfos[i].m_orig_width * levelInfos[i].m_orig_height : levelInfos[i].m_total_blocks;
                uint32_t outputSize = numBlocksOrPixels * bytesPerBlockOrPixel;

                VkBufferImageCopy copyRegion = {};
                copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.imageSubresource.mipLevel = i;
                copyRegion.imageSubresource.baseArrayLayer = 0;
                copyRegion.imageSubresource.layerCount = 1;
                copyRegion.imageExtent.width = levelInfos[i].m_orig_width;
                copyRegion.imageExtent.height = levelInfos[i].m_orig_height;
                copyRegion.imageExtent.depth = 1;
                copyRegion.bufferOffset = bufferOffset;

                vkCmdCopyBufferToImage(commandBuffer, staging.buffer, m_Image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                bufferOffset += outputSize;
            }

            VulkanImage::Transition(commandBuffer, m_Image.image, m_Image.format, m_MipLevels, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);

            VulkanBuffer::Destroy(staging);

            delete[] buffer;
            delete[] inputData;
        }
        else
        {   // image is a basic glTF format like png or jpg and can be loaded directly via tinyglTF
            unsigned char* buffer = nullptr;
            VkDeviceSize bufferSize = 0;
            bool deleteBuffer = false;

            if (gltfimage.component == 3)
            {   // most devices don't support RGB only on Vulkan so convert if necessary
                bufferSize = gltfimage.width * gltfimage.height * 4;
                buffer = new unsigned char[bufferSize];
                unsigned char* rgba = buffer;
                unsigned char* rgb = &gltfimage.image[0];
                for (int32_t i = 0; i < gltfimage.width * gltfimage.height; ++i)
                {
                    for (int32_t j = 0; j < 3; ++j)
                        rgba[j] = rgb[j];

                    rgba += 4;
                    rgb += 3;
                }
                deleteBuffer = true;
            }
            else
            {
                buffer = &gltfimage.image[0];
                bufferSize = gltfimage.image.size();
            }

            uint32_t width = gltfimage.width;
            uint32_t height = static_cast<uint32_t>(gltfimage.height);
            m_Image.extent = { width, height, 1 };
            m_MipLevels = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);

            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(vr->physicalDevice, m_Image.format, &formatProperties);
            DK_CORE_ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
            DK_CORE_ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

            // create host-visible staging buffer that contains the raw image data
            AllocatedBuffer staging =
                VulkanBuffer::Create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VkCR(vkMapMemory(vr->device, staging.memory, 0, staging.memReqs.size, 0, &staging.mapped));
            memcpy(staging.mapped, buffer, bufferSize);
            vkUnmapMemory(vr->device, staging.memory);

            if (deleteBuffer) delete[] buffer;

            // create optimal tiled target image
            m_Image =
                VulkanImage::Create(m_Image.extent, m_Image.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_MipLevels);

            VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

            VulkanImage::Transition(commandBuffer, m_Image.image, m_Image.format, m_MipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkBufferImageCopy copyRegion = {};
            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageExtent = m_Image.extent;

            vkCmdCopyBufferToImage(commandBuffer, staging.buffer, m_Image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            VulkanImage::Transition(commandBuffer, m_Image.image, m_Image.format, m_MipLevels, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);

            VulkanBuffer::Destroy(staging);

            // generate the mip chain (glTF uses jpg and png, so we need to create this manually)
            VkCommandBuffer blitCommandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

            for (uint32_t i = 1; i < m_MipLevels; i++)
            {
                VkImageBlit imageBlit = {};
                imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.srcSubresource.layerCount = 1;
                imageBlit.srcSubresource.mipLevel = i - 1;
                imageBlit.srcOffsets[1].x = int32_t(m_Image.extent.width >> (i - 1));
                imageBlit.srcOffsets[1].y = int32_t(m_Image.extent.height >> (i - 1));
                imageBlit.srcOffsets[1].z = 1;
                imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.dstSubresource.layerCount = 1;
                imageBlit.dstSubresource.mipLevel = i;
                imageBlit.dstOffsets[1].x = int32_t(m_Image.extent.width >> i);
                imageBlit.dstOffsets[1].y = int32_t(m_Image.extent.height >> i);
                imageBlit.dstOffsets[1].z = 1;

                VkImageSubresourceRange mipSubRange = {};
                mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                mipSubRange.baseMipLevel = i;
                mipSubRange.levelCount = 1;
                mipSubRange.layerCount = 1;

                {
                    VkImageMemoryBarrier imageBarrier = {};
                    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    imageBarrier.srcAccessMask = 0;
                    imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    imageBarrier.image = m_Image.image;
                    imageBarrier.subresourceRange = mipSubRange;
                    vkCmdPipelineBarrier(blitCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
                }

                vkCmdBlitImage(blitCommandBuffer, m_Image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

                {
                    VkImageMemoryBarrier imageBarrier = {};
                    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    imageBarrier.image = m_Image.image;
                    imageBarrier.subresourceRange = mipSubRange;
                    vkCmdPipelineBarrier(blitCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
                }
            }

            VulkanImage::Transition(blitCommandBuffer, m_Image.image, m_Image.format, m_MipLevels, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, blitCommandBuffer);
        }

        m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = textureSampler.magFilter;
        samplerInfo.minFilter = textureSampler.minFilter;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = textureSampler.addressModeU;
        samplerInfo.addressModeV = textureSampler.addressModeV;
        samplerInfo.addressModeW = textureSampler.addressModeW;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.maxAnisotropy = 1.0;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxLod = (float)m_MipLevels;
        samplerInfo.maxAnisotropy = 8.0f;
        samplerInfo.anisotropyEnable = VK_TRUE;
        VkCR(vkCreateSampler(vr->device, &samplerInfo, nullptr, &m_Sampler));

        UpdateDescriptor();
    }

    void TextureCubeMap::LoadFromFile(std::string filename, VkFormat format, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
    {
        DK_CORE_INFO("Loading TextureCube <{0}>", filename.c_str());

        const std::string path = vs->assetPath + filename;
        gli::texture_cube textureCube(gli::load(path.c_str()));
        DK_CORE_ASSERT(!textureCube.empty(), "Unable to load texture cube from file!");

        uint32_t width = static_cast<uint32_t>(textureCube.extent().x);
        uint32_t height = static_cast<uint32_t>(textureCube.extent().y);
        m_Image.extent = { width, height, 1 };
        m_Image.format = format;
        m_ImageLayout = imageLayout;
        m_MipLevels = static_cast<uint32_t>(textureCube.levels());

        // create host-visible staging buffer that contains the raw image data
        AllocatedBuffer staging =
            VulkanBuffer::Create(textureCube.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // copy texture data into staging buffer
        VkCR(vkMapMemory(vr->device, staging.memory, 0, staging.memReqs.size, 0, &staging.mapped));
        memcpy(staging.mapped, textureCube.data(), textureCube.size());
        vkUnmapMemory(vr->device, staging.memory);

        // create optimal tiled target image
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = m_Image.format;
        imageInfo.mipLevels = m_MipLevels;
        imageInfo.arrayLayers = 6;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.extent = m_Image.extent;
        imageInfo.usage = imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        VkCR(vkCreateImage(vr->device, &imageInfo, nullptr, &m_Image.image));

        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(vr->device, m_Image.image, &memReqs);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = VulkanDevice::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkCR(vkAllocateMemory(vr->device, &allocInfo, nullptr, &m_Image.memory));
        VkCR(vkBindImageMemory(vr->device, m_Image.image, m_Image.memory, 0));

        // setup buffer copy regions for each mip level
        std::vector<VkBufferImageCopy> copyRegions;
        uint32_t offset = 0;

        for (uint32_t face = 0; face < 6; face++)
        {
            for (uint32_t level = 0; level < m_MipLevels; level++)
            {
                VkBufferImageCopy copyRegion = {};
                copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.imageSubresource.mipLevel = level;
                copyRegion.imageSubresource.baseArrayLayer = face;
                copyRegion.imageSubresource.layerCount = 1;
                copyRegion.imageExtent.width = static_cast<uint32_t>(textureCube[face][level].extent().x);
                copyRegion.imageExtent.height = static_cast<uint32_t>(textureCube[face][level].extent().y);
                copyRegion.imageExtent.depth = 1;
                copyRegion.bufferOffset = offset;

                copyRegions.push_back(copyRegion);
                // increase offset into staging buffer for next level / face
                offset += textureCube[face][level].size();
            }
        }

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = m_MipLevels;
        subresourceRange.layerCount = 6;

        {
            VkImageMemoryBarrier imageBarrier{};
            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier.srcAccessMask = 0;
            imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier.image = m_Image.image;
            imageBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
        }

        // copy mip levels from staging buffer
        vkCmdCopyBufferToImage(commandBuffer, staging.buffer, m_Image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(copyRegions.size()), copyRegions.data());

        {
            VkImageMemoryBarrier imageBarrier{};
            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier.newLayout = m_ImageLayout;
            imageBarrier.srcAccessMask = 0;
            imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier.image = m_Image.image;
            imageBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
        }

        VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);

        VulkanBuffer::Destroy(staging);

        // Create image view
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        imageViewInfo.format = format;
        imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        imageViewInfo.subresourceRange.layerCount = 6;
        imageViewInfo.subresourceRange.levelCount = m_MipLevels;
        imageViewInfo.image = m_Image.image;
        VkCR(vkCreateImageView(vr->device, &imageViewInfo, nullptr, &m_Image.view));

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vr->physicalDevice, &deviceProperties);

        // create the sampler
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(m_MipLevels);
        samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VkCR(vkCreateSampler(vr->device, &samplerInfo, nullptr, &m_Sampler));

        UpdateDescriptor();
    }


    void TextureCubeMap::GenerateCubeMap()
    {
        auto tStart = std::chrono::high_resolution_clock::now();

        VkFormat format;
        uint32_t dim;

        switch (m_Target)
        {
        case IRRADIANCE:
            format = VK_FORMAT_R32G32B32A32_SFLOAT;
            dim = 64; break;
        case PREFILTERED:
            format = VK_FORMAT_R16G16B16A16_SFLOAT;
            dim = 512; break;
        };

        const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

        {   // create target cubemap
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = format;
            imageInfo.extent = { dim, dim, 1 };
            imageInfo.mipLevels = numMips;
            imageInfo.arrayLayers = 6;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            VkCR(vkCreateImage(vr->device, &imageInfo, nullptr, &m_Image.image));

            VkMemoryRequirements memReqs;
            vkGetImageMemoryRequirements(vr->device, m_Image.image, &memReqs);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReqs.size;
            allocInfo.memoryTypeIndex = VulkanDevice::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkCR(vkAllocateMemory(vr->device, &allocInfo, nullptr, &m_Image.memory));
            VkCR(vkBindImageMemory(vr->device, m_Image.image, m_Image.memory, 0));

            // image-view
            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            imageViewInfo.image = m_Image.image;
            imageViewInfo.format = format;
            imageViewInfo.subresourceRange = {};
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.levelCount = numMips;
            imageViewInfo.subresourceRange.layerCount = 6;
            VkCR(vkCreateImageView(vr->device, &imageViewInfo, nullptr, &m_Image.view));
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
            samplerInfo.maxLod = static_cast<float>(numMips);
            samplerInfo.maxAnisotropy = 1.0f;
            samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            VkCR(vkCreateSampler(vr->device, &samplerInfo, nullptr, &m_Sampler));
        }

        // color attachment
        VkAttachmentDescription attachment{};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

        struct Offscreen
        {
            AllocatedImage image;
            VkFramebuffer framebuffer;
        } offscreen;

        {   // create offscreen framebuffer
            offscreen.image = VulkanImage::Create(VkExtent3D{ dim, dim, 1 }, format,
                VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

            // framebuffer
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &offscreen.image.view;
            framebufferInfo.width = dim;
            framebufferInfo.height = dim;
            framebufferInfo.layers = 1;
            VkCR(vkCreateFramebuffer(vr->device, &framebufferInfo, nullptr, &offscreen.framebuffer));

            VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

            VulkanImage::Transition(commandBuffer, offscreen.image.image, format, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);
        }

        // descriptors
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSetLayoutBinding setLayoutBinding = { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pBindings = &setLayoutBinding;
        layoutInfo.bindingCount = 1;
        VkCR(vkCreateDescriptorSetLayout(vr->device, &layoutInfo, nullptr, &descriptorSetLayout));

        // descriptor Pool
        VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = 1;
        descriptorPoolInfo.pPoolSizes = &poolSize;
        descriptorPoolInfo.maxSets = 2;

        VkDescriptorPool descriptorPool;
        VkCR(vkCreateDescriptorPool(vr->device, &descriptorPoolInfo, nullptr, &descriptorPool));

        // descriptor sets
        VkDescriptorSet descriptorSet;
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = descriptorPool;
        descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
        descriptorSetAllocInfo.descriptorSetCount = 1;
        VkCR(vkAllocateDescriptorSets(vr->device, &descriptorSetAllocInfo, &descriptorSet));

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.pImageInfo = &vr->textures.environmentCube.GetDescriptor();
        vkUpdateDescriptorSets(vr->device, 1, &writeDescriptorSet, 0, nullptr);

        struct PushBlockIrradiance
        {
            glm::mat4 mvp;
            float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
            float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
        } pushBlockIrradiance;

        struct PushBlockPrefilter
        {
            glm::mat4 mvp;
            float roughness;
            uint32_t numSamples = 32u;
        } pushBlockPrefilter;

        // Pipeline layout
        VkPipelineLayout pipelineLayout;
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        switch (m_Target)
        {
        case IRRADIANCE:
            pushConstantRange.size = sizeof(PushBlockIrradiance); break;
        case PREFILTERED:
            pushConstantRange.size = sizeof(pushBlockPrefilter); break;
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        VkCR(vkCreatePipelineLayout(vr->device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        // pipeline
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationInfo.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachmentInfo{};
        blendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentInfo.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &blendAttachmentInfo;

        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilInfo.depthTestEnable = VK_FALSE;
        depthStencilInfo.depthWriteEnable = VK_FALSE;
        depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilInfo.front = depthStencilInfo.back;
        depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.scissorCount = 1;

        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicInfo{};
        dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicInfo.pDynamicStates = dynamicStateEnables.data();
        dynamicInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

        // vertex input state
        VkVertexInputBindingDescription vertexInputBinding = { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };
        VkVertexInputAttributeDescription vertexInputAttribute = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexInputBinding;
        vertexInputInfo.vertexAttributeDescriptionCount = 1;
        vertexInputInfo.pVertexAttributeDescriptions = &vertexInputAttribute;

        std::array < VkPipelineShaderStageCreateInfo, 2 > shaderStages = {};
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].pName = "main";
        shaderStages[0].module = VulkanShader::CreateShaderModule("shaders/bin/filtercube.vert.spv");

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].pName = "main";

        switch (m_Target)
        {
        case IRRADIANCE:
            shaderStages[1].module = VulkanShader::CreateShaderModule("shaders/bin/irradiancecube.frag.spv"); break;
        case PREFILTERED:
            shaderStages[1].module = VulkanShader::CreateShaderModule("shaders/bin/prefilterenvmap.frag.spv"); break;
        };

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pRasterizationState = &rasterizationInfo;
        pipelineInfo.pColorBlendState = &colorBlendInfo;
        pipelineInfo.pMultisampleState = &multisampleInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pDepthStencilState = &depthStencilInfo;
        pipelineInfo.pDynamicState = &dynamicInfo;
        pipelineInfo.stageCount = (uint32_t)shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.pNext = nullptr;

        VkPipeline pipeline;
        VkCR(vkCreateGraphicsPipelines(vr->device, vr->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

        for (auto shaderStage : shaderStages)
            vkDestroyShaderModule(vr->device, shaderStage.module, nullptr);

        // render cubemap
        VkClearValue clearValues[1];
        clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = offscreen.framebuffer;
        renderPassBeginInfo.renderArea.extent = { dim, dim };
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = clearValues;

        std::vector<glm::mat4> matrices = {
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

        VkViewport viewport{};
        viewport.width = (float)dim;
        viewport.height = (float)dim;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.extent = { dim, dim };

        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = numMips;
        subresourceRange.layerCount = 6;

        {   // change image layout for all cubemap faces to transfer destination
            VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.image = m_Image.image;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);
        }

        for (uint32_t m = 0; m < numMips; m++)
        {
            for (uint32_t f = 0; f < 6; f++)
            {
                VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

                viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
                viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                // render scene from cube face's point of view
                vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                // pass parameters for current pass using a push constant block
                switch (m_Target)
                {
                case IRRADIANCE:
                    pushBlockIrradiance.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
                    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockIrradiance), &pushBlockIrradiance);
                    break;
                case PREFILTERED:
                    pushBlockPrefilter.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
                    pushBlockPrefilter.roughness = (float)m / (float)(numMips - 1);
                    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockPrefilter), &pushBlockPrefilter);
                    break;
                };

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

                vr->scene.skybox.Draw(commandBuffer);

                vkCmdEndRenderPass(commandBuffer);

                VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresourceRange.baseMipLevel = 0;
                subresourceRange.levelCount = numMips;
                subresourceRange.layerCount = 6;

                VulkanImage::Transition(commandBuffer, offscreen.image.image, format, 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                // copy region for transfer from framebuffer to cube face
                VkImageCopy copyRegion{};
                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = { 0, 0, 0 };
                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = f;
                copyRegion.dstSubresource.mipLevel = m;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = { 0, 0, 0 };
                copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
                copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
                copyRegion.extent.depth = 1;

                vkCmdCopyImage(commandBuffer, offscreen.image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    m_Image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                VulkanImage::Transition(commandBuffer, offscreen.image.image, format, 1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

                VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);
            }
        }

        {
            VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.image = m_Image.image;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);
        }

        vkDestroyRenderPass(vr->device, renderPass, nullptr);
        vkDestroyFramebuffer(vr->device, offscreen.framebuffer, nullptr);
        VulkanImage::Destroy(offscreen.image);
        vkDestroyDescriptorPool(vr->device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(vr->device, descriptorSetLayout, nullptr);
        vkDestroyPipeline(vr->device, pipeline, nullptr);
        vkDestroyPipelineLayout(vr->device, pipelineLayout, nullptr);

        m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        UpdateDescriptor();

        if (m_Target == PREFILTERED)
        {
            vr->shaderValuesParams.prefilteredCubeMipLevels = static_cast<float>(numMips);
        }

        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        DK_CORE_INFO("Generating cube map with {0} mip levels took {1} ms", numMips, tDiff);
    }

    void TextureSampler::SetFilterModes(int32_t min, int32_t mag)
    {
        switch (min)
        {
        case -1:
        case 9728: minFilter = VK_FILTER_NEAREST; break;
        case 9729: minFilter = VK_FILTER_LINEAR; break;
        case 9984: minFilter = VK_FILTER_NEAREST; break;
        case 9985: minFilter = VK_FILTER_NEAREST; break;
        case 9986: minFilter = VK_FILTER_LINEAR; break;
        case 9987: minFilter = VK_FILTER_LINEAR; break;
        default: DK_CORE_WARN("Unknown filter mode: {0} and/or {1}", min, mag); break;
        }

        switch (mag)
        {
        case -1:
        case 9728: magFilter = VK_FILTER_NEAREST; break;
        case 9729: magFilter = VK_FILTER_LINEAR; break;
        case 9984: magFilter = VK_FILTER_NEAREST; break;
        case 9985: magFilter = VK_FILTER_NEAREST; break;
        case 9986: magFilter = VK_FILTER_LINEAR; break;
        case 9987: magFilter = VK_FILTER_LINEAR; break;
        default: DK_CORE_WARN("Unknown filter mode: {0} and/or {1}", min, mag); break;
        }
    }

    void TextureSampler::SetWrapModes(int32_t wrapS, int32_t wrapT)
    {
        switch (wrapS)
        {
        case -1:
        case 10497: addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
        case 33071: addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
        case 33648: addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
        default: DK_CORE_WARN("Unknown wrap mode: {0} and/or {1}", wrapS, wrapT); break;
        }

        switch (wrapT)
        {
        case -1:
        case 10497: addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
        case 33071: addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
        case 33648: addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
        default: DK_CORE_WARN("Unknown wrap mode: {0} and/or {1}", wrapS, wrapT); break;
        }
    }


}
