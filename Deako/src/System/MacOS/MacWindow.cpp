#include "MacWindow.h"
#include "dkpch.h"

namespace Deako {

    static uint8_t s_GLFWWindowCount = 0;

    MacWindow::MacWindow(const WindowProps& props)
    {
        Init(props);
    }

    MacWindow::~MacWindow()
    {
        Shutdown();
    }

    void MacWindow::Init(const WindowProps& props)
    {
        m_WindowData.title = props.title;
        m_WindowData.width = props.width;
        m_WindowData.height = props.height;
        m_WindowData.vsync = true;

        DK_CORE_INFO("Creating window - {0} ({1}, {2})", props.title, props.width, props.height);
    }

    void MacWindow::Shutdown()
    {
    }

    bool MacWindow::IsVSync() const
    {
        return true;
    }

    void MacWindow::SetVSync(bool enabled)
    {

    }

}
