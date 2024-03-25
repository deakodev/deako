#include "RenderCommand.h"
#include "dkpch.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Deak {

    RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

}
