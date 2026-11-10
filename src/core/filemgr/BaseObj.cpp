#include "BaseObj.hpp"

namespace FStop
{
	BaseObj::BaseObj(const std::filesystem::path& path) :
	    m_Path(path)
	{
	}

	bool BaseObj::Exists() const
	{
		return std::filesystem::exists(m_Path);
	}

	const std::filesystem::path& BaseObj::Path() const
	{
		return m_Path;
	}
}
