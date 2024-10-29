#include "Context.h"
#include "dkpch.h"

namespace Deako {

    static DeakoContext* s_DeakoContext = nullptr;

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

    DeakoContext::DeakoContext()
    {
        DK_CORE_ASSERT(!s_DeakoContext, "DeakoContext already exists!");
        s_DeakoContext = this;
    }

}
