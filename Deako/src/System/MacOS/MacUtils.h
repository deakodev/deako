#pragma once 

#include <map>
#include <string>
#include <cstring>
#include <dirent.h>
#include <sys/types.h>
#include <iostream>

namespace Deako {
    namespace MacUtils {

        inline void ReadDirectory(const std::string& directory, const std::string& extension, std::map<std::string, std::string>& filelist, bool recursive)
        {
            struct dirent* entry;
            DIR* dir = opendir(directory.c_str());
            if (dir == NULL)
            {
                DK_CORE_ERROR("Failed to open directory: {0}", directory.c_str());
                return;
            }
            while ((entry = readdir(dir)) != NULL)
            {
                if (entry->d_type == DT_REG)
                {
                    std::string filename(entry->d_name);
                    if (filename.find(extension) != std::string::npos)
                    {
                        filename.erase(filename.find_last_of("."), std::string::npos);
                        filelist[filename] = directory + "/" + entry->d_name;
                    }
                }
                if (recursive && (entry->d_type == DT_DIR))
                {
                    std::string subdir = directory + "/" + entry->d_name;
                    if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
                    {
                        ReadDirectory(subdir, extension, filelist, recursive);
                    }
                }
            }
            closedir(dir);
        }

    }
}
