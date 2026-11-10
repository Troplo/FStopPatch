#pragma once
#include <cstdint>
#include <cstddef>

namespace FStop
{
	class PatternHash
	{
	public:
		std::uint64_t m_Hash;

		constexpr PatternHash() :
		    m_Hash(0xcbf29ce484222325ULL)
		{
		}

		constexpr PatternHash(std::uint64_t hash) :
		    m_Hash(hash)
		{
		}

		constexpr PatternHash(const PatternHash& other) :
		    m_Hash(other.m_Hash)
		{
		}

		constexpr PatternHash(PatternHash&& other) :
		    m_Hash(other.m_Hash)
		{
		}

		constexpr PatternHash& operator=(const PatternHash& other)
		{
			m_Hash = other.m_Hash;
			return *this;
		}

		constexpr PatternHash Update(char data) const
		{
			auto hash = m_Hash;
			hash += (static_cast<std::uint64_t>(data));
			hash += hash << 10;
			hash ^= (hash >> 6);
			return PatternHash(hash);
		}

		constexpr PatternHash Update(int data) const
		{
			auto hash = m_Hash;
			hash ^= (data >> 8);
			hash += hash << 10;
			hash ^= (hash >> 7);
			return PatternHash(hash);
		}

		constexpr PatternHash Update(std::uint32_t data) const
		{
			auto hash = m_Hash;
			hash ^= (static_cast<std::uint64_t>(data) * 0x56295FFE2749C000ULL);
			hash += hash << 15;
			hash ^= (hash >> 5);
			return PatternHash(hash);
		}

		constexpr PatternHash Update(std::uint64_t data) const
		{
			auto hash = m_Hash;
			hash ^= (data * 0x56295FFE2749C000ULL);
			hash += hash << 15;
			hash ^= (hash >> 5);
			return PatternHash(hash);
		}

		constexpr std::uint64_t GetHash() const
		{
			return m_Hash;
		}
	};
}
