#include "MacUtils.h"
#include "dkpch.h"

namespace Deako {

    std::filesystem::path MacUtils::File::Open(const char* filter, const char* title)
    {
        const char* inPath = OpenFileUsingCocoa(filter, title);

        if (inPath)
        {
            std::filesystem::path outPath(inPath);
            free((void*)inPath); // Free memory allocated by strup (in MacPlatformUtils.mm)
            return outPath;
        }

        return std::filesystem::path();
    }

    std::filesystem::path MacUtils::File::SaveAs(const char* filter, const char* title)
    {
        const char* inPath = SaveFileUsingCocoa(filter, title);

        if (inPath)
        {
            std::filesystem::path outPath(inPath);
            free((void*)inPath); // Free memory allocated by strup (in MacPlatformUtils.mm)
            return outPath;
        }

        return std::filesystem::path();
    }

    int MacUtils::File::PromptSave(const char* message)
    {
        return PromptSaveUsingCocoa(message);
    }

}
