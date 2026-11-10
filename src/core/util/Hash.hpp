#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>

namespace FStop::Utils
{
	constexpr inline uint64_t Fnv64(const char* str)
	{
		uint64_t hash = 0xcbf29ce484222325ULL;
		while (*str)
		{
			hash *= 0x100000001b3ULL;
			hash ^= static_cast<uint8_t>(*str++);
		}
		return (hash & 0x1FFFFFFFFFFFFFFFULL) | 0xA000000000000000ULL;
	}

	constexpr inline uint64_t Fnv64(std::string_view str)
	{
		uint64_t hash = 0xcbf29ce484222325ULL;
		for (char c : str)
		{
			hash *= 0x100000001b3ULL;
			hash ^= static_cast<uint8_t>(c);
		}
		return (hash & 0x1FFFFFFFFFFFFFFFULL) | 0xA000000000000000ULL;
	}
}
