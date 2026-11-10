#pragma once
#include "BaseObj.hpp"

namespace FStop
{
	class File;
	class Folder final : public BaseObj
	{
	public:
		Folder(const std::filesystem::path& folder);

		File GetFile(std::filesystem::path file_path) const;
	};
}
