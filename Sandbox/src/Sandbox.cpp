#include "Deak.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class EditorLayer : public Deak::Layer
{
public:
    EditorLayer()
        : Layer("Example"), m_CameraController()
    {
        float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };

        m_VertexArray = Deak::VertexArray::Create();

        Deak::Ref<Deak::VertexBuffer> vertexBuffer = Deak::VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({ { Deak::ShaderDataType::Float3, "a_Position" }, { Deak::ShaderDataType::Float2, "a_TexCoord" } });

        m_VertexArray->AddVertexBuffer(vertexBuffer);

        auto shader = m_ShaderLibrary.Load("Sandbox/assets/shaders/Texture.glsl");
        m_Texture = Deak::Texture2D::Create("Sandbox/assets/textures/container2.png");

        std::dynamic_pointer_cast<Deak::OpenGLShader>(shader)->Bind();
        std::dynamic_pointer_cast<Deak::OpenGLShader>(shader)->setUniformInt("u_Texture", 0);
    }

    void OnUpdate(Deak::Timestep timestep) override
    {
        m_CameraController.OnUpdate(timestep);

        Deak::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Deak::RenderCommand::Clear();

        Deak::Renderer::BeginScene(m_CameraController.GetCamera());

        auto shader = m_ShaderLibrary.Get("Texture");

        m_Texture->Bind();
        glm::mat4 model = glm::mat4(1.0f);
        Deak::Renderer::Submit(shader, m_VertexArray, model);

        Deak::Renderer::EndScene();
    }

    virtual void OnImGuiRender() override
    {
        ImGui::Begin("Editor");
        auto mousePosition = Deak::Input::GetMousePosition();
        ImGui::Text("Mouse Position: (%.3f, %.3f)", mousePosition.first, mousePosition.second);
        ImGui::End();
    }


    void OnEvent(Deak::Event& event) override
    {
        m_CameraController.OnEvent(event);
    }

private:
    Deak::PerspectiveCameraController  m_CameraController;
    Deak::ShaderLibrary m_ShaderLibrary;
    Deak::Ref<Deak::VertexArray> m_VertexArray;
    Deak::Ref<Deak::Texture2D> m_Texture;
};

class Sandbox : public Deak::Application
{
public:
    Sandbox()
    {
        PushLayer(new EditorLayer());
    }

    ~Sandbox()
    {
    }
};

Deak::Application* Deak::CreateApplication()
{
    return new Sandbox();
}

