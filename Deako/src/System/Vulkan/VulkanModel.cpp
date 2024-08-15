#include "VulkanModel.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanUtils.h"

#define BASISU_HAVE_STD_TRIVIALLY_COPYABLE
#include <basisu_transcoder.h>

namespace Deako {

    static Ref<VulkanResources> vr = VulkanBase::GetResources();
    static Ref<VulkanSettings> vs = VulkanBase::GetSettings();

    void RenderNode(Node* node, VkCommandBuffer commandBuffer, Material::AlphaMode alphaMode, uint32_t dynamicOffset)
    {
        if (node->mesh)
        {   // Render mesh primitives
            for (Primitive* primitive : node->mesh->primitives)
            {
                if (primitive->material.alphaMode == alphaMode)
                {
                    std::string pipelineName = "pbr";
                    std::string pipelineVariant = "";

                    if (primitive->material.unlit) // KHR_materials_unlit
                        pipelineName = "unlit";

                    // material properties define if we need to bind a pipeline variant with culling disabled (double sided)
                    if (alphaMode == Material::ALPHAMODE_BLEND)
                        pipelineVariant = "_alpha_blending";
                    else if (primitive->material.doubleSided)
                        pipelineVariant = "_double_sided";

                    const VkPipeline pipeline = vr->pipelines[pipelineName + pipelineVariant];

                    if (pipeline != vr->boundPipeline)
                    {
                        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                        vr->boundPipeline = pipeline;
                    }

                    const std::vector<VkDescriptorSet> descriptorSets = {
                        vr->descriptorSets[vr->currentFrame].scene,
                        primitive->material.descriptorSet,
                        node->mesh->uniform.descriptorSet,
                        vr->shaderMaterialDescriptorSet
                    };
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vr->pipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 1, &dynamicOffset);

                    // pass material index for this primitive and vb address using a push constant, shader uses this to index into the material buffer
                    vkCmdPushConstants(commandBuffer, vr->pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &primitive->material.index);

                    if (primitive->hasIndices)
                        vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
                    else
                        vkCmdDraw(commandBuffer, primitive->vertexCount, 1, 0, 0);
                }
            }

        }

        for (auto child : node->children)
            RenderNode(child, commandBuffer, alphaMode, dynamicOffset);
    }

    void Model::DrawNode(Node* node, VkCommandBuffer commandBuffer)
    {
        if (node->mesh)
        {
            for (Primitive* primitive : node->mesh->primitives)
                vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
        }

        for (auto& child : node->children) DrawNode(child, commandBuffer);
    }

    void Model::Draw(VkCommandBuffer commandBuffer)
    {
        const VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);

        for (auto& node : nodes) DrawNode(node, commandBuffer);
    }

    void Model::UpdateAnimation(uint32_t index, float time)
    {
        if (animations.empty()) return;
        if (index > static_cast<uint32_t>(animations.size()) - 1) return;

        Animation& animation = animations[index];

        bool updated = false;
        for (auto& channel : animation.channels)
        {
            AnimationSampler& sampler = animation.samplers[channel.samplerIndex];

            if (sampler.inputs.size() > sampler.outputsVec4.size()) continue;

            for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
            {
                if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1]))
                {
                    float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
                    if (u <= 1.0f)
                    {
                        switch (channel.path)
                        {
                        case AnimationChannel::PathType::TRANSLATION:
                            sampler.Translate(i, time, channel.node); break;
                        case AnimationChannel::PathType::SCALE:
                            sampler.Scale(i, time, channel.node); break;
                        case AnimationChannel::PathType::ROTATION:
                            sampler.Rotate(i, time, channel.node); break;
                        }
                        updated = true;
                    }
                }
            }
        }

        if (updated)
            for (auto& node : nodes) node->Update();
    }

    // custom loading ktx textures function used with tinyglTF
    bool LoadImageDataFunc(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData)
    {
        if (image->uri.find_last_of(".") != std::string::npos)
        {    // ktx files will be handled by our own code
            if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx2")
                return true;
        }

        return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
    }

    void Model::LoadFromFile(float scale)
    {
        DK_CORE_INFO("Loading Model <{0}>", path.string());

        std::filesystem::path fullPath = vs->assetPath + path.string();

        bool binary = false;
        if (path.extension().string() == ".glb")
            binary = true;

        // prepare to load modal
        tinygltf::Model tinyModel;
        tinygltf::TinyGLTF gltfContext;

        std::string error;
        std::string warning;

        gltfContext.SetImageLoader(LoadImageDataFunc, nullptr);

        bool fileLoaded = binary ?
            gltfContext.LoadBinaryFromFile(&tinyModel, &error, &warning, fullPath.string()) :
            gltfContext.LoadASCIIFromFile(&tinyModel, &error, &warning, fullPath.string());

        LoaderInfo loaderInfo{};
        size_t vertexCount = 0;
        size_t indexCount = 0;

        if (fileLoaded)
        {
            extensions = tinyModel.extensionsUsed;
            for (auto& extension : extensions)
            {   // if model uses basis universal compressed textures, we need to transcode them
                // So we need to initialize that transcoder once
                if (extension == "KHR_texture_basisu")
                {
                    DK_CORE_INFO("Model uses KHR_texture_basisu, initializing basisu transcoder");
                    basist::basisu_transcoder_init();
                }
            }

            LoadTextureSamplers(tinyModel);
            LoadTextures(tinyModel);
            LoadMaterials(tinyModel);

            const tinygltf::Scene& tinyScene = tinyModel.scenes[tinyModel.defaultScene > -1 ? tinyModel.defaultScene : 0];

            // get vertex and index buffer sizes up-front
            for (size_t i = 0; i < tinyScene.nodes.size(); i++)
                GetNodeProps(tinyModel.nodes[tinyScene.nodes[i]], tinyModel, vertexCount, indexCount);

            loaderInfo.vertexBuffer = new Vertex[vertexCount];
            loaderInfo.indexBuffer = new uint32_t[indexCount];

            // TODO: scene handling with no default scene
            for (size_t i = 0; i < tinyScene.nodes.size(); i++)
            {
                const tinygltf::Node tinyNode = tinyModel.nodes[tinyScene.nodes[i]];
                LoadNode(nullptr, tinyNode, tinyScene.nodes[i], tinyModel, loaderInfo, scale);
            }

            if (tinyModel.animations.size() > 0)
                LoadAnimations(tinyModel);

            LoadSkins(tinyModel);

            for (auto linearNode : linearNodes)
            {   // assign skins
                if (linearNode->skinIndex > -1) linearNode->skin = skins[linearNode->skinIndex];
                // initial pose
                if (linearNode->mesh) linearNode->Update();
            }
        }
        else
        {
            DK_CORE_ERROR("Could not load gltf file: {0}", error); return;
        }

        size_t vertexBufferSize = vertexCount * sizeof(Vertex);
        size_t indexBufferSize = indexCount * sizeof(uint32_t);

        DK_CORE_ASSERT(vertexBufferSize > 0);

        // create host-visible staging buffers
        AllocatedBuffer vertexStaging =
            VulkanBuffer::Create(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vr->device, vertexStaging.memory, 0, vertexStaging.memReqs.size, 0, &vertexStaging.mapped));
        memcpy(vertexStaging.mapped, loaderInfo.vertexBuffer, vertexBufferSize);
        vkUnmapMemory(vr->device, vertexStaging.memory);

        vertices =
            VulkanBuffer::Create(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        AllocatedBuffer indexStaging;
        if (indexBufferSize > 0)
        {
            indexStaging =
                VulkanBuffer::Create(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VkCR(vkMapMemory(vr->device, indexStaging.memory, 0, indexStaging.memReqs.size, 0, &indexStaging.mapped));
            memcpy(indexStaging.mapped, loaderInfo.indexBuffer, indexBufferSize);
            vkUnmapMemory(vr->device, indexStaging.memory);

            indices =
                VulkanBuffer::Create(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vr->singleUseCommandPool);

        VkBufferCopy copyRegion = {};
        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(commandBuffer, vertexStaging.buffer, vertices.buffer, 1, &copyRegion);

        if (indexBufferSize > 0)
        {
            copyRegion.size = indexBufferSize;
            vkCmdCopyBuffer(commandBuffer, indexStaging.buffer, indices.buffer, 1, &copyRegion);
        }

        VulkanCommand::EndSingleTimeCommands(vr->singleUseCommandPool, commandBuffer);

        VulkanBuffer::Destroy(vertexStaging);
        if (indexBufferSize > 0)
            VulkanBuffer::Destroy(indexStaging);

        delete[] loaderInfo.vertexBuffer;
        delete[] loaderInfo.indexBuffer;

        GetSceneDimensions();
    }

    void Model::LoadTextureSamplers(tinygltf::Model& tinyModel)
    {
        for (tinygltf::Sampler tinySampler : tinyModel.samplers)
        {
            TextureSampler sampler{};
            sampler.SetFilterModes(tinySampler.minFilter, tinySampler.magFilter);
            sampler.SetWrapModes(tinySampler.wrapS, tinySampler.wrapT);
            sampler.addressModeW = sampler.addressModeV;

            textureSamplers.push_back(sampler);
        }
    }

    void Model::LoadTextures(tinygltf::Model& tinyModel)
    {
        for (tinygltf::Texture& tinyTex : tinyModel.textures)
        {
            int source = tinyTex.source;
            if (tinyTex.extensions.find("KHR_texture_basisu") != tinyTex.extensions.end())
            {    // if a texture uses KHR_texture_basisu, get source index from extension structure
                auto ext = tinyTex.extensions.find("KHR_texture_basisu");
                auto value = ext->second.Get("source");
                source = value.Get<int>();
            }

            tinygltf::Image tinyImage = tinyModel.images[source];
            TextureSampler textureSampler;
            if (tinyTex.sampler == -1)
            {    // no sampler specified, use a default one
                textureSampler.magFilter = VK_FILTER_LINEAR;
                textureSampler.minFilter = VK_FILTER_LINEAR;
                textureSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                textureSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                textureSampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            }
            else
            {
                textureSampler = textureSamplers[tinyTex.sampler];
            }

            Texture2D texture;
            texture.LoadFromGLTFImage(tinyImage, path, textureSampler);
            textures.push_back(texture);
        }
    }

    void Model::LoadMaterials(tinygltf::Model& tinyModel)
    {
        for (tinygltf::Material& tinyMaterial : tinyModel.materials)
        {
            Material material{ tinyMaterial, textures };
            material.index = static_cast<uint32_t>(materials.size());
            materials.push_back(material);
        }
        // push a default material at the end of the list for meshes with no material assigned
        materials.push_back(Material());
    }

    void Model::LoadNode(Node* parent, const tinygltf::Node& tinyNode, uint32_t nodeIndex, const tinygltf::Model& tinyModel, LoaderInfo& loaderInfo, float globalscale)
    {
        Node* node = new Node{};

        node->index = nodeIndex;
        node->parent = parent;
        node->name = tinyNode.name;
        node->skinIndex = tinyNode.skin;
        node->matrix = glm::mat4(1.0f);

        // generate local node matrix
        glm::vec3 translation = glm::vec3(0.0f);
        if (tinyNode.translation.size() == 3)
        {
            translation = glm::make_vec3(tinyNode.translation.data());
            node->translation = translation;
        }

        glm::mat4 rotation = glm::mat4(1.0f);
        if (tinyNode.rotation.size() == 4)
        {
            glm::quat q = glm::make_quat(tinyNode.rotation.data());
            node->rotation = glm::mat4(q);
        }

        glm::vec3 scale = glm::vec3(1.0f);
        if (tinyNode.scale.size() == 3)
        {
            scale = glm::make_vec3(tinyNode.scale.data());
            node->scale = scale;
        }

        if (tinyNode.matrix.size() == 16)
            node->matrix = glm::make_mat4x4(tinyNode.matrix.data());

        // node with children
        if (tinyNode.children.size() > 0)
        {
            for (size_t i = 0; i < tinyNode.children.size(); i++)
            {
                LoadNode(node, tinyModel.nodes[tinyNode.children[i]], tinyNode.children[i], tinyModel, loaderInfo, globalscale);
            }
        }

        // if node contains mesh data
        if (tinyNode.mesh > -1)
        {
            const tinygltf::Mesh tinyMesh = tinyModel.meshes[tinyNode.mesh];
            Mesh* mesh = new Mesh(node->matrix);

            for (size_t i = 0; i < tinyMesh.primitives.size(); i++)
            {
                const tinygltf::Primitive& tinyPrimitive = tinyMesh.primitives[i];

                uint32_t vertexStart = static_cast<uint32_t>(loaderInfo.vertexPos);
                uint32_t indexStart = static_cast<uint32_t>(loaderInfo.indexPos);
                uint32_t indexCount = 0;
                uint32_t vertexCount = 0;
                glm::vec3 posMin{};
                glm::vec3 posMax{};
                bool hasSkin = false;
                bool hasIndices = tinyPrimitive.indices > -1;

                {   // vertices
                    const float* bufferPos = nullptr;
                    const float* bufferNormals = nullptr;
                    const float* bufferTexCoordSet0 = nullptr;
                    const float* bufferTexCoordSet1 = nullptr;
                    const float* bufferColorSet0 = nullptr;
                    const void* bufferJoints = nullptr;
                    const float* bufferWeights = nullptr;

                    int posByteStride;
                    int normByteStride;
                    int uv0ByteStride;
                    int uv1ByteStride;
                    int color0ByteStride;
                    int jointByteStride;
                    int weightByteStride;

                    int jointComponentType;

                    // position attribute (required)
                    DK_CORE_ASSERT(tinyPrimitive.attributes.find("POSITION") != tinyPrimitive.attributes.end());

                    const tinygltf::Accessor& posAccessor =
                        tinyModel.accessors[tinyPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& posView =
                        tinyModel.bufferViews[posAccessor.bufferView];

                    bufferPos = reinterpret_cast<const float*>(
                        &(tinyModel.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));

                    posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
                    posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

                    vertexCount = static_cast<uint32_t>(posAccessor.count);

                    posByteStride = posAccessor.ByteStride(posView) ?
                        (posAccessor.ByteStride(posView) / sizeof(float)) :
                        tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

                    if (tinyPrimitive.attributes.find("NORMAL") != tinyPrimitive.attributes.end())
                    {
                        const tinygltf::Accessor& normAccessor =
                            tinyModel.accessors[tinyPrimitive.attributes.find("NORMAL")->second];
                        const tinygltf::BufferView& normView =
                            tinyModel.bufferViews[normAccessor.bufferView];

                        bufferNormals = reinterpret_cast<const float*>(
                            &(tinyModel.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));

                        normByteStride = normAccessor.ByteStride(normView) ?
                            (normAccessor.ByteStride(normView) / sizeof(float)) :
                            tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
                    }

                    if (tinyPrimitive.attributes.find("TEXCOORD_0") != tinyPrimitive.attributes.end())
                    {   //uv0
                        const tinygltf::Accessor& uvAccessor =
                            tinyModel.accessors[tinyPrimitive.attributes.find("TEXCOORD_0")->second];
                        const tinygltf::BufferView& uvView =
                            tinyModel.bufferViews[uvAccessor.bufferView];

                        bufferTexCoordSet0 = reinterpret_cast<const float*>(
                            &(tinyModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));

                        uv0ByteStride = uvAccessor.ByteStride(uvView) ?
                            (uvAccessor.ByteStride(uvView) / sizeof(float)) :
                            tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
                    }

                    if (tinyPrimitive.attributes.find("TEXCOORD_1") != tinyPrimitive.attributes.end())
                    {    //uv1
                        const tinygltf::Accessor& uvAccessor =
                            tinyModel.accessors[tinyPrimitive.attributes.find("TEXCOORD_1")->second];
                        const tinygltf::BufferView& uvView =
                            tinyModel.bufferViews[uvAccessor.bufferView];

                        bufferTexCoordSet1 = reinterpret_cast<const float*>(
                            &(tinyModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));

                        uv1ByteStride = uvAccessor.ByteStride(uvView) ?
                            (uvAccessor.ByteStride(uvView) / sizeof(float)) :
                            tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
                    }

                    if (tinyPrimitive.attributes.find("COLOR_0") != tinyPrimitive.attributes.end())
                    {
                        const tinygltf::Accessor& accessor =
                            tinyModel.accessors[tinyPrimitive.attributes.find("COLOR_0")->second];
                        const tinygltf::BufferView& view =
                            tinyModel.bufferViews[accessor.bufferView];

                        bufferColorSet0 = reinterpret_cast<const float*>(
                            &(tinyModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));

                        color0ByteStride = accessor.ByteStride(view) ?
                            (accessor.ByteStride(view) / sizeof(float)) :
                            tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
                    }

                    if (tinyPrimitive.attributes.find("JOINTS_0") != tinyPrimitive.attributes.end())
                    {
                        const tinygltf::Accessor& jtAccessor =
                            tinyModel.accessors[tinyPrimitive.attributes.find("JOINTS_0")->second];
                        const tinygltf::BufferView& jtView =
                            tinyModel.bufferViews[jtAccessor.bufferView];

                        bufferJoints =
                            &(tinyModel.buffers[jtView.buffer].data[jtAccessor.byteOffset + jtView.byteOffset]);

                        jointComponentType = jtAccessor.componentType;

                        jointByteStride = jtAccessor.ByteStride(jtView) ?
                            (jtAccessor.ByteStride(jtView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
                    }

                    if (tinyPrimitive.attributes.find("WEIGHTS_0") != tinyPrimitive.attributes.end())
                    {
                        const tinygltf::Accessor& wtAccessor =
                            tinyModel.accessors[tinyPrimitive.attributes.find("WEIGHTS_0")->second];
                        const tinygltf::BufferView& wtView =
                            tinyModel.bufferViews[wtAccessor.bufferView];

                        bufferWeights = reinterpret_cast<const float*>(
                            &(tinyModel.buffers[wtView.buffer].data[wtAccessor.byteOffset + wtView.byteOffset]));

                        weightByteStride = wtAccessor.ByteStride(wtView) ?
                            (wtAccessor.ByteStride(wtView) / sizeof(float)) :
                            tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
                    }

                    hasSkin = (bufferJoints && bufferWeights);

                    for (size_t v = 0; v < posAccessor.count; v++)
                    {
                        Vertex& vertex = loaderInfo.vertexBuffer[loaderInfo.vertexPos];

                        vertex.pos =
                            glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);

                        vertex.normal = glm::normalize(glm::vec3(bufferNormals ?
                            glm::make_vec3(&bufferNormals[v * normByteStride]) :
                            glm::vec3(0.0f)));

                        vertex.uv0 = bufferTexCoordSet0 ?
                            glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) :
                            glm::vec3(0.0f);

                        vertex.uv1 = bufferTexCoordSet1 ?
                            glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) :
                            glm::vec3(0.0f);

                        vertex.color = bufferColorSet0 ?
                            glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) :
                            glm::vec4(1.0f);

                        if (hasSkin)
                        {
                            switch (jointComponentType)
                            {
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            {
                                const uint16_t* buf = static_cast<const uint16_t*>(bufferJoints);
                                vertex.joint0 =
                                    glm::uvec4(glm::make_vec4(&buf[v * jointByteStride])); break;
                            }
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            {
                                const uint8_t* buf = static_cast<const uint8_t*>(bufferJoints);
                                vertex.joint0 =
                                    glm::vec4(glm::make_vec4(&buf[v * jointByteStride])); break;
                            }
                            default: // not supported by spec
                                DK_CORE_ERROR("Joint component type {0} not supported!", jointComponentType);
                                break;
                            }

                            vertex.weight0 =
                                glm::make_vec4(&bufferWeights[v * weightByteStride]);
                        }
                        else
                        {
                            vertex.joint0 = glm::vec4(0.0f);
                            vertex.weight0 = glm::vec4(0.0f);
                        }

                        // fix for all zero weights
                        if (glm::length(vertex.weight0) == 0.0f)
                            vertex.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

                        loaderInfo.vertexPos++;
                    }
                }

                if (hasIndices)
                {
                    const tinygltf::Accessor& accessor =
                        tinyModel.accessors[tinyPrimitive.indices > -1 ? tinyPrimitive.indices : 0];
                    const tinygltf::BufferView& bufferView =
                        tinyModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer =
                        tinyModel.buffers[bufferView.buffer];

                    indexCount = static_cast<uint32_t>(accessor.count);
                    const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

                    switch (accessor.componentType)
                    {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
                            loaderInfo.indexPos++;
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
                            loaderInfo.indexPos++;
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
                            loaderInfo.indexPos++;
                        }
                        break;
                    }
                    default:
                        DK_CORE_ERROR("Index component type {0} not supported!", accessor.componentType);
                        return;
                    }
                }

                Primitive* primitive = new Primitive(indexStart, indexCount, vertexCount, tinyPrimitive.material > -1 ? materials[tinyPrimitive.material] : materials.back());

                primitive->SetBoundingBox(posMin, posMax);

                mesh->primitives.push_back(primitive);
            }

            for (auto p : mesh->primitives)
            {   // mesh bounds from BBs of primitives
                if (p->boundingBox.valid && !mesh->boundingBox.valid)
                {
                    mesh->boundingBox = p->boundingBox;
                    mesh->boundingBox.valid = true;
                }

                mesh->boundingBox.min = glm::min(mesh->boundingBox.min, p->boundingBox.min);
                mesh->boundingBox.max = glm::max(mesh->boundingBox.max, p->boundingBox.max);
            }

            node->mesh = mesh;
        }

        if (parent) parent->children.push_back(node);
        else nodes.push_back(node);

        linearNodes.push_back(node);
    }

    void Model::LoadAnimations(tinygltf::Model& tinyModel)
    {
        for (tinygltf::Animation& tinyAnimation : tinyModel.animations)
        {
            Animation animation{};
            animation.name = tinyAnimation.name;
            if (tinyAnimation.name.empty())
                animation.name = std::to_string(animations.size());

            // samplers
            for (auto& tinySampler : tinyAnimation.samplers)
            {
                AnimationSampler sampler{};

                if (tinySampler.interpolation == "LINEAR")
                    sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
                if (tinySampler.interpolation == "STEP")
                    sampler.interpolation = AnimationSampler::InterpolationType::STEP;
                if (tinySampler.interpolation == "CUBICSPLINE")
                    sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;

                {   // read sampler input time values
                    const tinygltf::Accessor& accessor =
                        tinyModel.accessors[tinySampler.input];
                    const tinygltf::BufferView& bufferView =
                        tinyModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer =
                        tinyModel.buffers[bufferView.buffer];

                    DK_CORE_ASSERT(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                    const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    const float* buf = static_cast<const float*>(dataPtr);

                    for (size_t index = 0; index < accessor.count; index++)
                        sampler.inputs.push_back(buf[index]);

                    for (auto input : sampler.inputs)
                    {
                        if (input < animation.start) animation.start = input;
                        if (input > animation.end) animation.end = input;
                    }
                }


                {    // read sampler output T/R/S values 
                    const tinygltf::Accessor& accessor =
                        tinyModel.accessors[tinySampler.output];
                    const tinygltf::BufferView& bufferView =
                        tinyModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer =
                        tinyModel.buffers[bufferView.buffer];

                    DK_CORE_ASSERT(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                    const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

                    switch (accessor.type)
                    {
                    case TINYGLTF_TYPE_VEC3:
                    {
                        const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
                            sampler.outputs.push_back(buf[index][0]);
                            sampler.outputs.push_back(buf[index][1]);
                            sampler.outputs.push_back(buf[index][2]);
                        }
                        break;
                    }
                    case TINYGLTF_TYPE_VEC4:
                    {
                        const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            sampler.outputsVec4.push_back(buf[index]);
                            sampler.outputs.push_back(buf[index][0]);
                            sampler.outputs.push_back(buf[index][1]);
                            sampler.outputs.push_back(buf[index][2]);
                            sampler.outputs.push_back(buf[index][3]);
                        }
                        break;
                    }
                    default:
                    {
                        DK_CORE_WARN("Unknown animation sampler accessor type!");
                        break;
                    }
                    }
                }

                animation.samplers.push_back(sampler);
            }

            // channels
            for (auto& source : tinyAnimation.channels)
            {
                AnimationChannel channel{};

                if (source.target_path == "rotation")
                    channel.path = AnimationChannel::PathType::ROTATION;
                if (source.target_path == "translation")
                    channel.path = AnimationChannel::PathType::TRANSLATION;
                if (source.target_path == "scale")
                    channel.path = AnimationChannel::PathType::SCALE;
                if (source.target_path == "weights")
                {
                    DK_CORE_WARN("Animation weights not yet supported, skipping channel!");
                    continue;
                }

                channel.samplerIndex = source.sampler;
                channel.node = NodeFromIndex(source.target_node);

                if (!channel.node) continue;

                animation.channels.push_back(channel);
            }

            animations.push_back(animation);
        }
    }

    void Model::LoadSkins(tinygltf::Model& tinyModel)
    {
        for (tinygltf::Skin& source : tinyModel.skins)
        {
            Skin* skin = new Skin{};
            skin->name = source.name;

            // find skeleton root node
            if (source.skeleton > -1)
                skin->skeletonRoot = NodeFromIndex(source.skeleton);

            // find joint nodes
            for (int jointIndex : source.joints)
            {
                Node* node = NodeFromIndex(jointIndex);
                if (node) skin->joints.push_back(NodeFromIndex(jointIndex));
            }

            // get inverse bind matrices from buffer
            if (source.inverseBindMatrices > -1)
            {
                const tinygltf::Accessor& accessor =
                    tinyModel.accessors[source.inverseBindMatrices];
                const tinygltf::BufferView& bufferView =
                    tinyModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer =
                    tinyModel.buffers[bufferView.buffer];

                skin->inverseBindMatrices.resize(accessor.count);
                memcpy(skin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
            }

            skins.push_back(skin);
        }
    }

    void Model::GetNodeProps(const tinygltf::Node& tinyNode, const tinygltf::Model& tinyModel, size_t& vertexCount, size_t& indexCount)
    {
        if (tinyNode.children.size() > 0)
        {
            for (size_t i = 0; i < tinyNode.children.size(); i++)
                GetNodeProps(tinyModel.nodes[tinyNode.children[i]], tinyModel, vertexCount, indexCount);
        }
        if (tinyNode.mesh > -1)
        {
            const tinygltf::Mesh tinyMesh = tinyModel.meshes[tinyNode.mesh];
            for (size_t i = 0; i < tinyMesh.primitives.size(); i++)
            {
                auto tinyPrimitive = tinyMesh.primitives[i];
                vertexCount += tinyModel.accessors[tinyPrimitive.attributes.find("POSITION")->second].count;
                if (tinyPrimitive.indices > -1)
                    indexCount += tinyModel.accessors[tinyPrimitive.indices].count;
            }
        }
    }

    Node* Model::FindNode(Node* parent, uint32_t index)
    {
        Node* nodeFound = nullptr;
        if (parent->index == index) return parent;
        for (auto& child : parent->children)
        {
            nodeFound = FindNode(child, index);
            if (nodeFound) break;
        }
        return nodeFound;
    }

    Node* Model::NodeFromIndex(uint32_t index)
    {
        Node* nodeFound = nullptr;
        for (auto& node : nodes)
        {
            nodeFound = FindNode(node, index);
            if (nodeFound) break;
        }
        return nodeFound;
    }

    void Model::GetSceneDimensions()
    {
        // calculate binary volume hierarchy for all nodes in the scene
        for (auto node : linearNodes) CalculateBoundingBox(node, nullptr);

        dimensions.min = glm::vec3(FLT_MAX);
        dimensions.max = glm::vec3(-FLT_MAX);

        for (auto node : linearNodes)
        {
            if (node->boundingVH.valid)
            {
                dimensions.min = glm::min(dimensions.min, node->boundingVH.min);
                dimensions.max = glm::max(dimensions.max, node->boundingVH.max);
            }
        }

        // Calculate scene aabb
        aaBoundingBox = glm::scale(glm::mat4(1.0f), glm::vec3(
            dimensions.max[0] - dimensions.min[0],
            dimensions.max[1] - dimensions.min[1],
            dimensions.max[2] - dimensions.min[2]));

        aaBoundingBox[3][0] = dimensions.min[0];
        aaBoundingBox[3][1] = dimensions.min[1];
        aaBoundingBox[3][2] = dimensions.min[2];
    }

    void Model::CalculateBoundingBox(Node* node, Node* parent)
    {
        BoundingBox parentBoundingVH = parent ? parent->boundingVH : BoundingBox(dimensions.min, dimensions.max);

        if (node->mesh)
        {
            if (node->mesh->boundingBox.valid)
            {
                node->aaBoundingBox = node->mesh->boundingBox.GetAABB(node->GetMatrix());
                if (node->children.size() == 0)
                {
                    node->boundingVH.min = node->aaBoundingBox.min;
                    node->boundingVH.max = node->aaBoundingBox.max;
                    node->boundingVH.valid = true;
                }
            }
        }

        parentBoundingVH.min = glm::min(parentBoundingVH.min, node->boundingVH.min);
        parentBoundingVH.max = glm::min(parentBoundingVH.max, node->boundingVH.max);

        for (auto& child : node->children) CalculateBoundingBox(child, node);
    }

    void Model::Destroy()
    {
        if (vertices.buffer != VK_NULL_HANDLE)
            VulkanBuffer::Destroy(vertices);
        if (indices.buffer != VK_NULL_HANDLE)
            VulkanBuffer::Destroy(indices);

        for (auto texture : textures) texture.Destroy();
        for (auto node : nodes) delete node;
        for (auto skin : skins) delete skin;

        textures.resize(0);
        textureSamplers.resize(0);
        materials.resize(0);
        animations.resize(0);
        nodes.resize(0);
        linearNodes.resize(0);
        extensions.resize(0);
        skins.resize(0);
    };

    void Node::Update()
    {
        useCachedMatrix = false;

        if (mesh)
        {
            glm::mat4 matrix = GetMatrix();

            if (skin)
            {
                mesh->uniformBlock.matrix = matrix;
                glm::mat4 inverseTransform = glm::inverse(matrix);

                // update joint matrices
                size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);
                for (size_t i = 0; i < numJoints; i++)
                {
                    Node* jointNode = skin->joints[i];
                    glm::mat4 jointMat = jointNode->GetMatrix() * skin->inverseBindMatrices[i];
                    jointMat = inverseTransform * jointMat;
                    mesh->uniformBlock.jointMatrix[i] = jointMat;
                }

                mesh->uniformBlock.jointcount = static_cast<uint32_t>(numJoints);

                memcpy(mesh->uniform.mapped, &mesh->uniformBlock, sizeof(mesh->uniformBlock));
            }
            else
            {
                memcpy(mesh->uniform.mapped, &matrix, sizeof(glm::mat4));
            }
        }

        for (auto& child : children) child->Update();
    }

    Node::~Node()
    {
        if (mesh) delete mesh;
        for (auto& child : children) delete child;
    }

    glm::mat4 Node::LocalMatrix()
    {
        if (!useCachedMatrix)
        {
            cachedLocalMatrix =
                glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
        }

        return cachedLocalMatrix;
    }

    glm::mat4 Node::GetMatrix()
    {   // use a simple caching algorithm to avoid recalculating matrices to often
        if (!useCachedMatrix)
        {
            glm::mat4 m = LocalMatrix();
            Node* p = parent;
            while (p)
            {
                m = p->LocalMatrix() * m;
                p = p->parent;
            }
            cachedMatrix = m;
            useCachedMatrix = true;
            return m;
        }
        else
        {
            return cachedMatrix;
        }
    }

    void Node::SetDescriptorSet()
    {
        if (mesh)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = vr->descriptorPool;
            allocInfo.pSetLayouts = &vr->descriptorSetLayouts.node;
            allocInfo.descriptorSetCount = 1;
            VkCR(vkAllocateDescriptorSets(vr->device, &allocInfo, &mesh->uniform.descriptorSet));

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.descriptorCount = 1;
            write.dstSet = mesh->uniform.descriptorSet;
            write.dstBinding = 0;
            write.pBufferInfo = &mesh->uniform.descriptor;

            vkUpdateDescriptorSets(vr->device, 1, &write, 0, nullptr);
        }

        for (auto& child : children)
            child->SetDescriptorSet();
    }

    Mesh::Mesh(glm::mat4 matrix)
    {
        this->uniformBlock.matrix = matrix;

        uniform.buffer = VulkanBuffer::Create(sizeof(uniformBlock),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vr->device, uniform.buffer.memory, 0, sizeof(uniformBlock), 0, &uniform.mapped));
        memcpy(uniform.mapped, &uniformBlock, sizeof(uniformBlock));

        uniform.descriptor = { uniform.buffer.buffer, 0, sizeof(uniformBlock) };
    };

    Mesh::~Mesh()
    {
        VulkanBuffer::Destroy(uniform.buffer);
        for (Primitive* p : primitives) delete p;
    }

    void Mesh::SetBoundingBox(glm::vec3 min, glm::vec3 max)
    {
        boundingBox.min = min;
        boundingBox.max = max;
        boundingBox.valid = true;
    }

    Primitive::Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material& material)
        : firstIndex(firstIndex), indexCount(indexCount), vertexCount(vertexCount), material(material)
    {
        hasIndices = indexCount > 0;
    };

    void Primitive::SetBoundingBox(glm::vec3 min, glm::vec3 max)
    {
        boundingBox.min = min;
        boundingBox.max = max;
        boundingBox.valid = true;
    }

    BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max)
        : min(min), max(max)
    {
    };

    BoundingBox BoundingBox::GetAABB(glm::mat4 m)
    {
        glm::vec3 min = glm::vec3(m[3]);
        glm::vec3 max = min;
        glm::vec3 v0, v1;

        glm::vec3 right = glm::vec3(m[0]);
        v0 = right * this->min.x;
        v1 = right * this->max.x;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        glm::vec3 up = glm::vec3(m[1]);
        v0 = up * this->min.y;
        v1 = up * this->max.y;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        glm::vec3 back = glm::vec3(m[2]);
        v0 = back * this->min.z;
        v1 = back * this->max.z;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        return BoundingBox(min, max);
    }

    // Cube spline interpolation function used for translate/scale/rotate with cubic spline animation samples
    // Details on how this works can be found in the specs https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
    glm::vec4 AnimationSampler::CubicSplineInterpolation(size_t index, float time, uint32_t stride)
    {
        float delta = inputs[index + 1] - inputs[index];
        float t = (time - inputs[index]) / delta;
        const size_t current = index * stride * 3;
        const size_t next = (index + 1) * stride * 3;
        const size_t A = 0;
        const size_t V = stride * 1;
        const size_t B = stride * 2;

        float t2 = powf(t, 2);
        float t3 = powf(t, 3);
        glm::vec4 pt{ 0.0f };
        for (uint32_t i = 0; i < stride; i++) {
            float p0 = outputs[current + i + V];			// starting point at t = 0
            float m0 = delta * outputs[current + i + A];	// scaled starting tangent at t = 0
            float p1 = outputs[next + i + V];				// ending point at t = 1
            float m1 = delta * outputs[next + i + B];		// scaled ending tangent at t = 1
            pt[i] = ((2.f * t3 - 3.f * t2 + 1.f) * p0) + ((t3 - 2.f * t2 + t) * m0) + ((-2.f * t3 + 3.f * t2) * p1) + ((t3 - t2) * m0);
        }
        return pt;
    }

    // Calculates the translation of this sampler for the given node at a given time point depending on the interpolation type
    void AnimationSampler::Translate(size_t index, float time, Node* node)
    {
        switch (interpolation)
        {
        case AnimationSampler::InterpolationType::LINEAR:
        {
            float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
            node->translation = glm::mix(outputsVec4[index], outputsVec4[index + 1], u); break;
        }
        case AnimationSampler::InterpolationType::STEP:
        {
            node->translation = outputsVec4[index]; break;
        }
        case AnimationSampler::InterpolationType::CUBICSPLINE:
        {
            node->translation = CubicSplineInterpolation(index, time, 3); break;
        }
        }
    }

    // Calculates the scale of this sampler for the given node at a given time point depending on the interpolation type
    void AnimationSampler::Scale(size_t index, float time, Node* node)
    {
        switch (interpolation)
        {
        case AnimationSampler::InterpolationType::LINEAR:
        {
            float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
            node->scale = glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
            break;
        }
        case AnimationSampler::InterpolationType::STEP:
        {
            node->scale = outputsVec4[index]; break;
        }
        case AnimationSampler::InterpolationType::CUBICSPLINE:
        {
            node->scale = CubicSplineInterpolation(index, time, 3); break;
        }
        }
    }

    // Calculates the rotation of this sampler for the given node at a given time point depending on the interpolation type
    void AnimationSampler::Rotate(size_t index, float time, Node* node)
    {
        switch (interpolation)
        {
        case AnimationSampler::InterpolationType::LINEAR:
        {
            float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
            glm::quat q1;
            q1.x = outputsVec4[index].x;
            q1.y = outputsVec4[index].y;
            q1.z = outputsVec4[index].z;
            q1.w = outputsVec4[index].w;
            glm::quat q2;
            q2.x = outputsVec4[index + 1].x;
            q2.y = outputsVec4[index + 1].y;
            q2.z = outputsVec4[index + 1].z;
            q2.w = outputsVec4[index + 1].w;
            node->rotation = glm::normalize(glm::slerp(q1, q2, u));
            break;
        }
        case AnimationSampler::InterpolationType::STEP:
        {
            glm::quat q1;
            q1.x = outputsVec4[index].x;
            q1.y = outputsVec4[index].y;
            q1.z = outputsVec4[index].z;
            q1.w = outputsVec4[index].w;
            node->rotation = q1;
            break;
        }
        case AnimationSampler::InterpolationType::CUBICSPLINE:
        {
            glm::vec4 rot = CubicSplineInterpolation(index, time, 4);
            glm::quat q;
            q.x = rot.x;
            q.y = rot.y;
            q.z = rot.z;
            q.w = rot.w;
            node->rotation = glm::normalize(q);
            break;
        }
        }
    }
}
