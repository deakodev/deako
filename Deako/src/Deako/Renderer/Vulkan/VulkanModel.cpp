#include "VulkanModel.h"
#include "dkpch.h"

#include "VulkanBase.h"
#include "VulkanScene.h"
#include "VulkanResource.h"

namespace Deako {

    static Ref<VulkanBaseResources> vbr = VulkanBase::GetResources();
    static Ref<VulkanBaseContext> vbc = VulkanBase::GetContext();
    static Ref<VulkanSceneResources> vsr = VulkanScene::GetResources();
    static Ref<VulkanSceneContext> vsc = VulkanScene::GetContext();

    void RenderNode(Node* node, VkCommandBuffer commandBuffer, Material::AlphaMode alphaMode, uint32_t dynamicOffset)
    {
        if (node->mesh)
        {   // Render mesh primitives
            for (Primitive* primitive : node->mesh->primitives)
            {
                if (primitive->material.alphaMode == alphaMode)
                {
                    VkPipeline pipelineToUse = vsr->pipelines.pbr;

                    if (primitive->material.unlit) // KHR_materials_unlit
                        pipelineToUse = vsr->pipelines.unlit;

                    if (alphaMode == Material::ALPHAMODE_BLEND)
                        pipelineToUse = vsr->pipelines.unlitAlphaBlending;
                    else if (primitive->material.doubleSided)
                        pipelineToUse = primitive->material.unlit ? vsr->pipelines.unlitDoubleSided : vsr->pipelines.pbrDoubleSided;

                    if (pipelineToUse != vsc->boundPipeline)
                    {
                        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineToUse);
                        vsc->boundPipeline = pipelineToUse;
                    }

                    const std::vector<VkDescriptorSet> descriptorSets = {
                        vbc->frames[vbc->currentFrame].sceneDescriptorSet,
                        primitive->material.descriptorSet,
                        node->mesh->uniform.descriptorSet,
                        vsr->materialBuffer.descriptorSet
                    };
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vsr->scenePipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 1, &dynamicOffset);

                    // pass material index for this primitive using a push constant, shader uses this to index in the material buffer
                    vkCmdPushConstants(commandBuffer, vsr->scenePipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &primitive->material.index);

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

    void Model::SetMaterials(const std::vector<Ref<Material>>& materials)
    {
        this->materials = materials;
    }

    void Model::SetVertices()
    {
        // create host-visible staging buffers
        VulkanBuffer::AllocatedBuffer vertexStaging =
            VulkanBuffer::Create(vertexData.buffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vbr->device, vertexStaging.memory, 0, vertexStaging.memReqs.size, 0, &vertexStaging.mapped));
        memcpy(vertexStaging.mapped, vertexData.buffer.data, vertexData.buffer.size);
        vkUnmapMemory(vbr->device, vertexStaging.memory);

        vertices =
            VulkanBuffer::Create(vertexData.buffer.size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vbr->singleUseCommandPool);

        VkBufferCopy copyRegion = {};
        copyRegion.size = vertexData.buffer.size;
        vkCmdCopyBuffer(commandBuffer, vertexStaging.buffer, vertices.buffer, 1, &copyRegion);

        VulkanCommand::EndSingleTimeCommands(vbr->singleUseCommandPool, commandBuffer);

        VulkanBuffer::Destroy(vertexStaging);
    }

    void Model::SetIndices()
    {
        if (indexData.buffer.size > 0)
        {
            // create host-visible staging buffers
            VulkanBuffer::AllocatedBuffer indexStaging =
                VulkanBuffer::Create(indexData.buffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VkCR(vkMapMemory(vbr->device, indexStaging.memory, 0, indexStaging.memReqs.size, 0, &indexStaging.mapped));
            memcpy(indexStaging.mapped, indexData.buffer.data, indexData.buffer.size);
            vkUnmapMemory(vbr->device, indexStaging.memory);

            indices =
                VulkanBuffer::Create(indexData.buffer.size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkCommandBuffer commandBuffer = VulkanCommand::BeginSingleTimeCommands(vbr->singleUseCommandPool);

            VkBufferCopy copyRegion = {};
            copyRegion.size = indexData.buffer.size;
            vkCmdCopyBuffer(commandBuffer, indexStaging.buffer, indices.buffer, 1, &copyRegion);

            VulkanCommand::EndSingleTimeCommands(vbr->singleUseCommandPool, commandBuffer);

            VulkanBuffer::Destroy(indexStaging);
        }
    }

    void Model::Destroy()
    {
        if (vertices.buffer != VK_NULL_HANDLE)
            VulkanBuffer::Destroy(vertices);
        if (indices.buffer != VK_NULL_HANDLE)
            VulkanBuffer::Destroy(indices);

        for (auto node : nodes) delete node;
        for (auto skin : skins) delete skin;

        materials.resize(0);
        animations.resize(0);
        nodes.resize(0);
        linearNodes.resize(0);
        extensions.resize(0);
        skins.resize(0);
    };

    void Model::DetermineDimensions()
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

    Mesh::Mesh(glm::mat4 matrix)
    {
        this->uniformBlock.matrix = matrix;

        uniform.buffer = VulkanBuffer::Create(sizeof(uniformBlock),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkCR(vkMapMemory(vbr->device, uniform.buffer.memory, 0, sizeof(uniformBlock), 0, &uniform.mapped));
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
