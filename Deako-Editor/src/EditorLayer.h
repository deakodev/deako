#pragma once

#include "Deako.h"

namespace Deako {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate() override;
        virtual void OnEvent(Event& event) override;

        virtual void OnImGuiRender() override;

    private:


    };

}
