#include "Folder.hpp"
#include "File.hpp"
#include "FileMgr.hpp"

namespace FStop
{
	Folder::Folder(const std::filesystem::path& path) :
	    BaseObj(path)
	{
	}

	File Folder::GetFile(std::filesystem::path file_path) const
	{
		return File(m_Path / file_path);
	}
}
