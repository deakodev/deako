#include "RenderCommand.h"
#include "dkpch.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Deak {

    Scope<RendererAPI> RenderCommand::s_RendererAPI = CreateScope<OpenGLRendererAPI>();

}
