#include "LayerStack.h"
#include "dkpch.h"

namespace Deako {

    LayerStack::~LayerStack()
    {
        for (Layer* layer : m_Layers)
        {
            layer->OnDetach();

            delete layer;
            layer = nullptr;
        }
    }

    void LayerStack::OnUpdate()
    {
        for (Layer* layer : m_Layers)
            layer->OnUpdate();
    }

    void LayerStack::OnEvent(Event& event)
    {
        for (Layer* layer : m_Layers)
        {
            if (event.Handled)
                break;
            layer->OnEvent(event);
        }
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
        m_LayerInsertIndex++;
        layer->OnAttach();
    }

    void LayerStack::PushOverlay(Layer* overlay)
    {
        m_Layers.emplace_back(overlay);
        overlay->OnAttach();
    }

    void LayerStack::PopLayer(int count)
    {
        for (int i = 0; i < count && m_LayerInsertIndex > 0; ++i)
        {
            auto layer = m_Layers[m_LayerInsertIndex - 1];
            layer->OnDetach();
            m_Layers.erase(m_Layers.begin() + --m_LayerInsertIndex);
        }
    }

    void LayerStack::PopOverlay(int count)
    {
        for (int i = 0; i < count && !m_Layers.empty(); ++i)
        {
            auto overlay = m_Layers.back();
            overlay->OnDetach();
            m_Layers.pop_back();
        }
    }

}
