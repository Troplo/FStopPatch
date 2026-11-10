#pragma once
#include "PointerCalculator.h"
#include "common.hpp"

#include <type_traits>
#include <winternl.h>

namespace FStop
{
	template<typename T>
	concept Symbol = std::is_convertible_v<T, int> || std::is_convertible_v<T, std::string_view>;

	class Module
	{
	public:
		Module(LDR_DATA_TABLE_ENTRY* dllEntry);
		~Module() = default;

		std::string Name() const;
		inline std::uintptr_t Base() const;
		inline std::uintptr_t Size() const;
		inline std::uintptr_t End() const;

		template<typename R = void*>
		R GetExport(const std::string& symbolName) const;

		template<typename R = void*>
		R GetExport(int ordinal) const;

		[[nodiscard]] bool IsExported(const std::string& symbolName) const;
		[[nodiscard]] bool IsExported(int ordinal) const;

		void** GetImport(const std::string& moduleName, const std::string& symbolName) const;

		bool Valid() const;

		IMAGE_NT_HEADERS* GetNtHeader() const;
		[[nodiscard]] std::uint64_t Hash() const;

	private:
		template<typename R = void*, Symbol T>
		R GetExportImpl(const T& symbol) const;

	private:
		std::filesystem::path m_Path;
		std::string m_Name;
		PointerCalculator m_Base;
		std::uintptr_t m_Size = 0;
	};

	inline std::uintptr_t Module::Base() const
	{
		return m_Base.As<std::uintptr_t>();
	}

	inline std::uintptr_t Module::Size() const
	{
		return m_Size;
	}

	inline std::uintptr_t Module::End() const
	{
		return Base() + m_Size;
	}

	template<typename R>
	inline R Module::GetExport(const std::string& name) const
	{
		auto result = GetExportImpl<R>(std::string_view(name));
		if (result)
			return result;

		LOG(FATAL) << "Cannot find export: " << name;
		return nullptr;
	}

	template<typename R>
	inline R Module::GetExport(const int ordinal) const
	{
		auto result = GetExportImpl<R>(ordinal);
		if (result)
			return result;

		LOG(FATAL) << "Cannot find export: " << ordinal;
		return nullptr;
	}

	inline bool Module::IsExported(const std::string& symbolName) const
	{
		return GetExportImpl<void*>(std::string_view(symbolName)) != nullptr;
	}

	inline bool Module::IsExported(const int ordinal) const
	{
		return GetExportImpl<void*>(ordinal) != nullptr;
	}

	template<typename R, Symbol T>
	inline R Module::GetExportImpl(const T& symbol) const
	{
		const auto ntHeader = GetNtHeader();
		if (!ntHeader)
			return nullptr;

		const auto imageDataDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		const auto exportDirectory = m_Base.Add(imageDataDirectory.VirtualAddress).As<IMAGE_EXPORT_DIRECTORY*>();
		const auto namesOffsets = m_Base.Add(exportDirectory->AddressOfNames).As<DWORD*>();
		const auto ordinalOffsets = m_Base.Add(exportDirectory->AddressOfNameOrdinals).As<uint16_t*>();
		const auto funcOffsets = m_Base.Add(exportDirectory->AddressOfFunctions).As<DWORD*>();

		for (std::size_t i = 0; i < exportDirectory->NumberOfNames; i++)
		{
			if constexpr (std::is_convertible_v<T, int>)
			{
				if (exportDirectory->Base + ordinalOffsets[i] != symbol)
					continue;

				return m_Base.Add(funcOffsets[ordinalOffsets[i]]).As<R>();
			}
			else if constexpr (std::is_convertible_v<T, std::string_view>)
			{
				const auto functionName = m_Base.Add(namesOffsets[i]).As<const char*>();
				std::string_view sv(symbol);
				if (strcmp(functionName, sv.data()) != 0)
					continue;

				return m_Base.Add(funcOffsets[ordinalOffsets[i]]).As<R>();
			}
		}

		return nullptr;
	}
}
