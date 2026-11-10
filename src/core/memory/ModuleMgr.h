#pragma once
#include "Module.h"
#include "common.hpp"
#include <unordered_map>
#include <memory>
#include <string>

namespace FStop
{
	class ModuleMgr
	{
	public:
		ModuleMgr() = default;
		virtual ~ModuleMgr() = default;
		ModuleMgr(const ModuleMgr&) = delete;
		ModuleMgr(ModuleMgr&&) noexcept = delete;
		ModuleMgr& operator=(const ModuleMgr&) = delete;
		ModuleMgr& operator=(ModuleMgr&&) noexcept = delete;

		Module* Get(const std::string& name);

		/**
		 * @brief Loads the modules from PEB and caches them.
		 * 
		 * @return true If the peb is found and modules have been cached.
		 * @return false If the peb or peb->Ldr pointer were invalid. 
		 */
		bool LoadModules();
		std::unordered_multimap<std::string, std::unique_ptr<Module>>& GetModules();
		bool IsManualMapped() const;

	private:
		bool m_ManualMapped = false;
		std::unordered_multimap<std::string, std::unique_ptr<Module>> m_CachedModules;
	};

	inline ModuleMgr ModuleMgr;
}
