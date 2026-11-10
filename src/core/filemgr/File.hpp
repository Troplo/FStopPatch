#pragma once
#include "BaseObj.hpp"

namespace FStop
{
	class File final : public BaseObj
	{
	public:
		File(const std::filesystem::path& path);

		File Move(std::filesystem::path newPath);
	};
}
