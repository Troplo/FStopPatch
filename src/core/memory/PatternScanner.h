#pragma once
#include "Pattern.hpp"
#include "PointerCalculator.h"

#include <functional>
#include <vector>
#include <string_view>
#include <string>
#include <optional>
#include <windows.h>

namespace FStop
{
	class Module;
	using PatternFunc = std::function<void(PointerCalculator)>;

	class PatternScanner
	{
	private:
		const Module* m_Module;
		std::vector<const Module*> m_Modules{};

		struct PatternEntry
		{
			const IPattern* pattern;
			PatternFunc func;
			bool markNotRequired;
		};
		struct PatternGroup
		{
			std::string_view name;
			std::vector<PatternEntry> variants;
		};

		std::vector<PatternGroup> m_Groups;

		// progress dialog
		static HWND m_hDialog;
		static HWND m_hLabel;
		static size_t m_total;
		static size_t m_current;
		static HWND m_hProgress;

	public:
		PatternScanner(const Module* module = nullptr);
		void CheckSplitEngine();

		template<Signature S>
		void Add(std::string_view groupName, const Pattern<S>& pattern, const PatternFunc& func, bool markNotRequired = false);

		template<Signature S>
		void Add(std::string_view groupName, Pattern<S>&& pattern, const PatternFunc& func, bool markNotRequired = false) = delete;

		bool Scan();

	private:
		bool ScanGroup(const PatternGroup& group) const;
		bool ScanInternal(const IPattern* pattern, PatternFunc func, const std::string& groupName) const;
		std::optional<std::uintptr_t> FindAddress(const IPattern* pattern, const std::string& groupName) const;
		PatternHash ComputeGroupHash(const PatternGroup& group) const;
		static LRESULT CALLBACK ProgressProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void ShowProgressDialog(size_t total);
		void UpdateProgress(size_t current, size_t total);
		void CloseProgressDialog();
	};

	template<Signature S>
	inline void PatternScanner::Add(std::string_view groupName,
									const Pattern<S>& pattern,
									const PatternFunc& func, bool markNotRequired)
	{
		for (auto& group : m_Groups)
		{
			if (group.name == groupName)
			{
				group.variants.push_back({ &pattern, func, markNotRequired });
				return;
			}
		}

		PatternGroup group;
		group.name = groupName;
		group.variants.push_back({ &pattern, func, markNotRequired });
		m_Groups.push_back(std::move(group));
	}
}
