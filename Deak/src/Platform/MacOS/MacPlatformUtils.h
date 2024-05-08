#pragma once

#ifdef __cplusplus
extern "C" {
    #endif

    // Impl of these in objective-c MacPlatformUtils.mm
    const char* OpenFileUsingCocoa(const char* filter);
    const char* SaveFileUsingCocoa(const char* filter);
    int PromptSaveOnExitUsingCocoa();

    #ifdef __cplusplus
}
#endif
