#include "Deak.h"

#include "imgui/imgui.h"

class EditorLayer : public Deak::Layer
{
public:
    EditorLayer()
        : Layer("Example"), m_CameraController()
    {
        float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 0.8f, 0.5f,
         0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 0.8f, 0.5f,
         0.5f,  0.5f, -0.5f,  0.2f, 0.2f, 0.8f, 0.5f,
         0.5f,  0.5f, -0.5f,  0.2f, 0.2f, 0.8f, 0.5f,
        -0.5f,  0.5f, -0.5f,  0.2f, 0.2f, 0.8f, 0.5f,
        -0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 0.8f, 0.5f,

        -0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.2f, 0.5f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.2f, 0.5f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.2f, 0.5f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.2f, 0.5f,
        -0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.2f, 0.5f,
        -0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.2f, 0.5f,

        -0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.2f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 0.2f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.2f, 0.8f, 0.2f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.2f, 0.8f, 0.2f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.2f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.2f, 0.0f,

         0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.8f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.8f, 0.2f, 0.8f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.2f, 0.8f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.2f, 0.8f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.2f, 0.8f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.8f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.2f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.2f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.2f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.2f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.2f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.2f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 0.8f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 0.8f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.8f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.8f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.8f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 0.8f, 0.0f,
        };

        m_VertexArray.reset(Deak::VertexArray::Create());

        std::shared_ptr<Deak::VertexBuffer> vertexBuffer;
        vertexBuffer.reset(Deak::VertexBuffer::Create(vertices, sizeof(vertices)));

        vertexBuffer->SetLayout({ { Deak::ShaderDataType::Float3, "a_Position" }, { Deak::ShaderDataType::Float4, "a_Color" } });
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        std::string vertexSource = R"(
            #version 330 core

            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec4 a_Color;

            out vec4 v_Color;

            uniform mat4 u_Model;
            uniform mat4 u_ViewProjection;

            void main()
            {
                gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
                v_Color = a_Color;
            }
        )";

        std::string fragmentSource = R"(
            #version 330 core

            in vec4 v_Color;
            out vec4 fragColor;

            void main()
            {
                fragColor = vec4(v_Color);
            }
        )";

        m_Shader.reset(Deak::Shader::Create(vertexSource, fragmentSource));
    }

    void OnUpdate(Deak::Timestep timestep) override
    {
        m_CameraController.OnUpdate(timestep);

        Deak::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Deak::RenderCommand::Clear();

        Deak::Renderer::BeginScene(m_CameraController.GetCamera());

        Deak::Renderer::Submit(m_Shader, m_VertexArray);

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
    Deak::OrthographicCameraController  m_CameraController;
    std::shared_ptr<Deak::Shader> m_Shader;
    std::shared_ptr<Deak::VertexArray> m_VertexArray;
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

