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

        Deak::Ref<Deak::VertexBuffer> vertexBuffer;
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

            uniform vec3 u_Color;

            void main()
            {
                fragColor = vec4(u_Color, 1.0) *v_Color;
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

        static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

        std::dynamic_pointer_cast<Deak::OpenGLShader>(m_Shader)->Bind();
        std::dynamic_pointer_cast<Deak::OpenGLShader>(m_Shader)->setUniformVec3("u_Color", m_SquareColor);

        glm::mat4 model = glm::mat4(1.0f);
        Deak::Renderer::Submit(m_Shader, m_VertexArray, model);


        Deak::Renderer::EndScene();
    }

    virtual void OnImGuiRender() override
    {
        ImGui::Begin("Editor");
        auto mousePosition = Deak::Input::GetMousePosition();
        ImGui::Text("Mouse Position: (%.3f, %.3f)", mousePosition.first, mousePosition.second);
        ImGui::ColorEdit3("Square Color:", glm::value_ptr(m_SquareColor));
        ImGui::End();
    }


    void OnEvent(Deak::Event& event) override
    {
        m_CameraController.OnEvent(event);
    }

private:
    Deak::PerspectiveCameraController  m_CameraController;
    Deak::Ref<Deak::Shader> m_Shader;
    Deak::Ref<Deak::VertexArray> m_VertexArray;

    glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
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

