#include "Example3D.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Example3D::Example3D()
    : Layer("Example3D")
{
}

void Example3D::OnAttach()
{
    m_VertexArray = Deak::VertexArray::Create();

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    Deak::Ref<Deak::VertexBuffer> vertexBuffer = Deak::VertexBuffer::Create(vertices, sizeof(vertices));
    vertexBuffer->SetLayout({ { Deak::ShaderDataType::Float3, "a_Position" } });
    m_VertexArray->AddVertexBuffer(vertexBuffer);

    m_Shader = Deak::Shader::Create("Sandbox/assets/shaders/Example3D.glsl");
}

void Example3D::OnDetach()
{
}

void Example3D::OnUpdate(Deak::Timestep timestep)
{
    m_CameraController.OnUpdate(timestep);

    Deak::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
    Deak::RenderCommand::Clear();

    Deak::Renderer::BeginScene(m_CameraController.GetCamera());

    std::dynamic_pointer_cast<Deak::OpenGLShader>(m_Shader)->Bind();
    std::dynamic_pointer_cast<Deak::OpenGLShader>(m_Shader)->UploadUniformVec3("u_Color", m_ColorModifier);

    Deak::Renderer::Submit(m_Shader, m_VertexArray, glm::mat4(1.0f));

    Deak::Renderer::EndScene();
}

void Example3D::OnEvent(Deak::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Example3D::OnImGuiRender()
{
    ImGui::Begin("Example3D");
    ImGui::ColorEdit3("Square Color", glm::value_ptr(m_ColorModifier));
    ImGui::End();
}
