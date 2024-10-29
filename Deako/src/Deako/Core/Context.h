#pragma once

namespace Deako {

    struct DeakoContext
    {

    };

    DeakoContext& GetContext()
    {
        DK_CORE_ASSERT(s_DeakoContext, "DeakoContext not initialized!");
        return *s_DeakoContext;
    }

    void DestroyContext()
    {
        delete s_DeakoContext;
        s_DeakoContext = nullptr;
    }

    DeakoContext::DeakoContext();


}
