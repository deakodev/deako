#pragma once

#include <string>

namespace Deak {

	class Time
	{
	public:
		static float GetTime();
	};

	class FileUtils
	{
	public:
		// These returns empty string if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
		static int PromptSaveOnClose();

		static const std::string& GetFilePath() { return s_FilePath; }
		static void EmptyFilePath() { s_FilePath = std::string(); }

		static void SetFileSaved(bool value) { s_SavedChanges = value; }
		static bool IsCurrentFileSaved() { return s_SavedChanges; }

	private:
		static std::string s_FilePath;
		static bool s_SavedChanges;
	};

}
