#include "DebugPanel.h"

namespace Deako {

    void DebugPanel::OnImGuiRender()
    {
        ImGui::Begin("Debug");

        DkContext& deako = Deako::GetContext();
        const RendererStats& stats = Deako::GetSceneStats();

        float fps = 1000.0f / stats.frameTime;
        ImGui::Text("%s: %.2f ms/frame (%.0f fps)", "Frame Time", stats.frameTime, fps);
        ImGui::Text("Primitive Count: %llu", stats.primitiveCount);
        ImGui::Text("Draw Call Count: %llu", stats.drawCallCount);

        ImGui::Text("Scene Handle: %llu", (DkU64)deako.activeSceneHandle);
        ImGui::Text("Hovered Handle: %llu", (DkU64)deako.hoveredHandle);
        ImGui::Text("Active Handle: %llu", (DkU64)deako.activeHandle);

        const char* blocked = Deako::AreEventsBlocked() ? "true" : "false";
        ImGui::Text("Events Blocked? %s", blocked);

        ImGui::End();
    }

}
