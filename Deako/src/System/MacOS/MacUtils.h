#pragma once

#include <filesystem>

#ifdef __cplusplus
extern "C" {
    #endif

    // Impl of these in objective-c MacPlatformUtils.mm
    const char* OpenFileUsingCocoa(const char* filter, const char* title);
    const char* SaveFileUsingCocoa(const char* filter, const char* title);
    int PromptSaveUsingCocoa(const char* message);

    #ifdef __cplusplus
}
#endif

namespace Deako {
    namespace MacUtils {
        namespace File {

            std::filesystem::path Open(const char* filter, const char* title);

            std::filesystem::path SaveAs(const char* filter, const char* title);

            int PromptSave(const char* message);

        }

    }
}
