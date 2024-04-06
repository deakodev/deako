#include "RenderCommand.h"
#include "dkpch.h"

namespace Deak {

    Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

}
