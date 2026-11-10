#pragma once
#include <filesystem>
#include <windows.h>

#include "File.hpp"
#include "Folder.hpp"

#ifndef TNT_MAX_PATH
#define TNT_MAX_PATH 32768
#endif

namespace FStop
{
	class FileMgr final
	{
	private:
		FileMgr() = default;

	public:
		virtual ~FileMgr() = default;

		FileMgr(const FileMgr&) = delete;
		FileMgr(FileMgr&&) noexcept = delete;
		FileMgr& operator=(const FileMgr&) = delete;
		FileMgr& operator=(FileMgr&&) noexcept = delete;

		static void Init(const std::filesystem::path& rootFolder);

		[[nodiscard]] static File GetProjectFile(const std::filesystem::path& file)
		{
			return GetInstance().GetProjectFileImpl(file);
		}
		[[nodiscard]] static Folder GetProjectFolder(const std::filesystem::path& folder)
		{
			return GetInstance().GetProjectFolderImpl(folder);
		}

		[[nodiscard]] static File GetBakedProjectFile(const std::filesystem::path& file)
		{
			return GetInstance().GetBakedProjectFileImpl(file);
		}
		[[nodiscard]] static Folder GetBakedProjectFolder(const std::filesystem::path& folder)
		{
			return GetInstance().GetBakedProjectFolderImpl(folder);
		}

		static std::filesystem::path CreateFolderIfNotExists(std::filesystem::path folder);
		static std::filesystem::path EnsureFileCanBeCreated(std::filesystem::path file);
		static const std::filesystem::path& GetGameDirectory()
		{
			if (!GetInstance().m_GameFolder.empty())
			{
				return GetInstance().m_GameFolder;
			}
			wchar_t buffer[TNT_MAX_PATH];
			GetModuleFileNameW(NULL, buffer, TNT_MAX_PATH);

			std::filesystem::path exePath(buffer);
			GetInstance().m_GameFolder = exePath.parent_path();
			return GetInstance().m_GameFolder;
		}
		static const char* GetProjectName() { return "FStopPatch"; }
		static const wchar_t* GetProjectNameW() { return L"FStopPatch"; }

	private:
		void InitImpl(const std::filesystem::path& rootFolder);

		File GetProjectFileImpl(const std::filesystem::path& file) const;
		Folder GetProjectFolderImpl(const std::filesystem::path& folder) const;
		File GetBakedProjectFileImpl(const std::filesystem::path& file) const;
		Folder GetBakedProjectFolderImpl(const std::filesystem::path& folder) const;

		static FileMgr& GetInstance()
		{
			static FileMgr i{};
			return i;
		}

	private:
		std::filesystem::path m_RootFolder;
		std::filesystem::path m_GameFolder;
	};
}
