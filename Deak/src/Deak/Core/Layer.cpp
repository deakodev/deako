#include "Layer.h"
#include "dkpch.h"

namespace Deak {

    Layer::Layer(std::string_view debugName)
        : m_DebugName(debugName)
    {
    }

    Layer::~Layer()
    {
    }

}
