#include "FileMgr.hpp"
#include "File.hpp"
#include "Folder.hpp"

namespace FStop
{
	void FileMgr::Init(const std::filesystem::path& rootFolder)
	{
		GetInstance().InitImpl(rootFolder);
	}

	void FileMgr::InitImpl(const std::filesystem::path& rootFolder)
	{
		m_RootFolder = rootFolder;
		CreateFolderIfNotExists(m_RootFolder);
	}

	std::filesystem::path FileMgr::CreateFolderIfNotExists(std::filesystem::path folder)
	{
		bool create_path = !exists(folder);

		if (!create_path && !is_directory(folder))
		{
			std::filesystem::remove(folder);
			create_path = true;
		}

		if (create_path)
			std::filesystem::create_directories(folder);

		return folder;
	}

	std::filesystem::path FileMgr::EnsureFileCanBeCreated(std::filesystem::path file)
	{
		return FileMgr::CreateFolderIfNotExists(file.parent_path());
	}

	File FileMgr::GetProjectFileImpl(const std::filesystem::path& file) const
	{
		auto projFile = File(m_RootFolder / file);
		EnsureFileCanBeCreated(m_RootFolder / file);
		return projFile;
	}

	Folder FileMgr::GetProjectFolderImpl(const std::filesystem::path& folder) const
	{
		return CreateFolderIfNotExists(m_RootFolder / folder);
	}

	File FileMgr::GetBakedProjectFileImpl(const std::filesystem::path& file) const
	{
		auto fullPath = GetGameDirectory() / "FStopPatch" / file;
		EnsureFileCanBeCreated(fullPath);
		return File(fullPath);
	}

	Folder FileMgr::GetBakedProjectFolderImpl(const std::filesystem::path& folder) const
	{
		auto fullPath = GetGameDirectory() / "FStopPatch" / folder;
		return CreateFolderIfNotExists(fullPath);
	}
}
