#include "Deak/Utils/PlatformUtils.h"
#include "dkpch.h"

#include "MacPlatformUtils.h"

namespace Deak {

    std::string FileUtils::OpenFile(const char* filter)
    {
        const char* filePath = OpenFileUsingCocoa(filter);

        if (filePath)
        {
            s_FilePath = filePath;
            free((void*)filePath); // Free memory allocated by strup (in MacPlatformUtils.mm)
            return s_FilePath;
        }

        return std::string();
    }

    std::string FileUtils::SaveFile(const char* filter)
    {
        const char* filePath = SaveFileUsingCocoa(filter);

        if (filePath)
        {
            s_FilePath = filePath;
            free((void*)filePath); // Free memory allocated by strup (in MacPlatformUtils.mm)
            return s_FilePath;
        }

        return std::string();
    }

    int FileUtils::PromptSaveOnClose() // Decision to keep this abstraction for platform agnostic reasons
    {
        return PromptSaveOnExitUsingCocoa();
    }

}
