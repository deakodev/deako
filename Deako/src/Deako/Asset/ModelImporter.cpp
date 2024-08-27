#include "ModelImporter.h"
#include "dkpch.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#define BASISU_HAVE_STD_TRIVIALLY_COPYABLE
#include <basisu_transcoder.h>

namespace Deako {

    Ref<Model> ModelImporter::ImportModel(const AssetMetadata& metadata)
    {
        DK_CORE_INFO("Importing Model");


        return nullptr;
    }

    void ModelImporter::LoadTextureSamplers(tinygltf::Model& tinyModel, Ref<Model> model)
    {
        // for (tinygltf::Sampler tinySampler : tinyModel.samplers)
        // {
        //     TextureSampler sampler{};
        //     sampler.SetFilterModes(tinySampler.minFilter, tinySampler.magFilter);
        //     sampler.SetWrapModes(tinySampler.wrapS, tinySampler.wrapT);
        //     sampler.addressModeW = sampler.addressModeV;

        //     model->textureSamplers.push_back(sampler);
        // }
    }

    void ModelImporter::LoadTextures(tinygltf::Model& tinyModel, Ref<Model> model)
    {
        // for (tinygltf::Texture& tinyTex : tinyModel.textures)
        // {
        //     int source = tinyTex.source;
        //     if (tinyTex.extensions.find("KHR_texture_basisu") != tinyTex.extensions.end())
        //     {    // if a texture uses KHR_texture_basisu, get source index from extension structure
        //         auto ext = tinyTex.extensions.find("KHR_texture_basisu");
        //         auto value = ext->second.Get("source");
        //         source = value.Get<int>();
        //     }

        //     tinygltf::Image tinyImage = tinyModel.images[source];
        //     TextureSampler textureSampler;
        //     if (tinyTex.sampler == -1)
        //     {    // no sampler specified, use a default one
        //         textureSampler.magFilter = VK_FILTER_LINEAR;
        //         textureSampler.minFilter = VK_FILTER_LINEAR;
        //         textureSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        //         textureSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        //         textureSampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        //     }
        //     else
        //     {
        //         textureSampler = model->textureSamplers[tinyTex.sampler];
        //     }

        //     Ref<Texture2D> texture = CreateRef<Texture2D>();
        //     texture->LoadFromGLTFImage(tinyImage, model->path, textureSampler);
        //     model->textures.push_back(texture);
        // }
    }

    void ModelImporter::LoadMaterials(tinygltf::Model& tinyModel, Ref<Model> model)
    {
        // for (tinygltf::Material& tinyMaterial : tinyModel.materials)
        // {
        //     Material material{ tinyMaterial, model->textures };
        //     material.index = static_cast<uint32_t>(model->materials.size());
        //     model->materials.push_back(material);
        // }
        // // push a default material at the end of the list for meshes with no material assigned
        // model->materials.push_back(Material());
    }

    void ModelImporter::LoadNode(Node* parent, const tinygltf::Node& tinyNode, uint32_t nodeIndex, const tinygltf::Model& tinyModel, Ref<Model> model, float globalscale)
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
                LoadNode(node, tinyModel.nodes[tinyNode.children[i]], tinyNode.children[i], tinyModel, model, globalscale);
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

                uint32_t vertexStart = static_cast<uint32_t>(model->vertexData.position);
                uint32_t indexStart = static_cast<uint32_t>(model->indexData.position);
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

                    model->vertexData.count = static_cast<uint32_t>(posAccessor.count);

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
                        Model::Vertex& vertex = *(Model::Vertex*)(model->vertexData.buffer.data + model->vertexData.position * sizeof(Model::Vertex));

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

                        model->vertexData.position++;
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

                    model->indexData.count = static_cast<uint32_t>(accessor.count);
                    const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

                    switch (accessor.componentType)
                    {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            ((uint32_t*)model->indexData.buffer.data)[model->indexData.position] = buf[index] + vertexStart;
                            model->indexData.position++;
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            ((uint32_t*)model->indexData.buffer.data)[model->indexData.position] = buf[index] + vertexStart;
                            model->indexData.position++;
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            ((uint32_t*)model->indexData.buffer.data)[model->indexData.position] = buf[index] + vertexStart;
                            model->indexData.position++;
                        }
                        break;
                    }
                    default:
                        DK_CORE_ERROR("Index component type {0} not supported!", accessor.componentType);
                        return;
                    }
                }

                Primitive* primitive = new Primitive(indexStart, model->indexData.count, model->vertexData.count, tinyPrimitive.material > -1 ? *model->materials[tinyPrimitive.material] : *model->materials.back());

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
        else model->nodes.push_back(node);

        model->linearNodes.push_back(node);
    }

    void ModelImporter::LoadAnimations(tinygltf::Model& tinyModel, Ref<Model> model)
    {
        for (tinygltf::Animation& tinyAnimation : tinyModel.animations)
        {
            Animation animation{};
            animation.name = tinyAnimation.name;
            if (tinyAnimation.name.empty())
                animation.name = std::to_string(model->animations.size());

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
                channel.node = NodeFromIndex(source.target_node, model);

                if (!channel.node) continue;

                animation.channels.push_back(channel);
            }

            model->animations.push_back(animation);
        }
    }

    void ModelImporter::LoadSkins(tinygltf::Model& tinyModel, Ref<Model> model)
    {
        for (tinygltf::Skin& source : tinyModel.skins)
        {
            Skin* skin = new Skin{};
            skin->name = source.name;

            // find skeleton root node
            if (source.skeleton > -1)
                skin->skeletonRoot = NodeFromIndex(source.skeleton, model);

            // find joint nodes
            for (int jointIndex : source.joints)
            {
                Node* node = NodeFromIndex(jointIndex, model);
                if (node) skin->joints.push_back(NodeFromIndex(jointIndex, model));
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

            model->skins.push_back(skin);
        }
    }

    void ModelImporter::GetNodeProps(const tinygltf::Node& tinyNode, const tinygltf::Model& tinyModel, Ref<Model> model)
    {
        if (tinyNode.children.size() > 0)
        {
            for (size_t i = 0; i < tinyNode.children.size(); i++)
                GetNodeProps(tinyModel.nodes[tinyNode.children[i]], tinyModel, model);
        }
        if (tinyNode.mesh > -1)
        {
            const tinygltf::Mesh tinyMesh = tinyModel.meshes[tinyNode.mesh];
            for (size_t i = 0; i < tinyMesh.primitives.size(); i++)
            {
                auto tinyPrimitive = tinyMesh.primitives[i];
                model->vertexData.count += tinyModel.accessors[tinyPrimitive.attributes.find("POSITION")->second].count;
                if (tinyPrimitive.indices > -1)
                    model->indexData.count += tinyModel.accessors[tinyPrimitive.indices].count;
            }
        }
    }

    Node* ModelImporter::FindNode(Node* parent, uint32_t index)
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

    Node* ModelImporter::NodeFromIndex(uint32_t index, Ref<Model> model)
    {
        Node* nodeFound = nullptr;
        for (auto& node : model->nodes)
        {
            nodeFound = FindNode(node, index);
            if (nodeFound) break;
        }
        return nodeFound;
    }

}
