#pragma once
#include "File.hpp"
#include <cstdint>
#include <memory>

namespace FStop
{
	using cacheData = std::unique_ptr<uint8_t[]>;

	class CacheHeader final
	{
	public:
		std::uint32_t m_CacheVersion = 0;
		std::uint32_t m_FileVersion = 0;
		std::uint64_t m_DataSize = 0;
	};

	class CacheFile final
	{
		File m_CacheFile;
		std::uint32_t m_CacheVersion;
		CacheHeader m_CacheHeader;
		cacheData m_Data;

	public:
		CacheFile() = delete;
		CacheFile(File cacheFile);
		CacheFile(File cacheFile, std::uint32_t cacheVersion);

		void Free();
		bool Load();
		bool Write() const;
		std::uint8_t* Data() const;
		std::uint64_t DataSize() const;

		bool UpToDate(std::uint32_t fileVersion) const;
		void SetData(cacheData&& data, std::uint64_t dataSize);
		void SetHeaderVersion(std::uint32_t fileVersion);
		void SetCacheVersion(std::uint32_t cacheVersion);
	};
}
