#include "PatternScanner.h"
#include "Module.h"
#include "ModuleMgr.h"
#include "core/logger/LogHelper.h"
#include "core/util/Hash.hpp"
#include "core/util/CurrentModule.h"

#include <commctrl.h>
#include <chrono>
#include <format>
#include <algorithm>

namespace FStop
{
	HWND PatternScanner::m_hLabel = nullptr;
	HWND PatternScanner::m_hDialog = nullptr;
	HWND PatternScanner::m_hProgress = nullptr;
	size_t PatternScanner::m_current = 0;
	size_t PatternScanner::m_total = 0;

	PatternScanner::PatternScanner(const Module* module) :
	    m_Module(module)
	{
		CheckSplitEngine();
	}

	void PatternScanner::CheckSplitEngine()
	{
		m_Modules.clear();

		if (m_Module && m_Module->Valid())
			m_Modules.push_back(m_Module);

		if (!IsSplitEngine())
			return;

		for (const auto& dll : SplitEngines())
		{
			auto* module = ModuleMgr.Get(dll);
			if (!module || !module->Valid())
				continue;

			if (std::find(m_Modules.begin(), m_Modules.end(), module) == m_Modules.end())
			{
				m_Modules.push_back(module);
			}
		}
	}

	bool PatternScanner::Scan()
	{
		if (m_Modules.empty())
			CheckSplitEngine();

		if (m_Modules.empty())
		{
			LOG(WARNING) << "PatternScanner: No valid modules found to scan!";
			return false;
		}

		size_t totalEntries = 0;
		for (const auto& group : m_Groups)
			totalEntries += group.variants.size();

		bool dialogShown = false;
		auto startTime = std::chrono::steady_clock::now();

		bool success = true;
		size_t current = 0;

		for (const auto& group : m_Groups)
		{
			if (!ScanGroup(group))
				success = false;

			current++;

			// Only show dialog if scanning takes longer than 1000ms
			if (!dialogShown)
			{
				auto now = std::chrono::steady_clock::now();
				if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() > 1000)
				{
					ShowProgressDialog(totalEntries);
					dialogShown = true;
				}
			}

			if (dialogShown)
			{
				UpdateProgress(current, totalEntries);

				MSG msg;
				while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}

		if (dialogShown)
			CloseProgressDialog();

		return success;
	}

	bool PatternScanner::ScanGroup(const PatternGroup& group) const
	{
		bool optional = false;

		for (std::size_t i = 0; i < group.variants.size(); ++i)
		{
			const auto& entry = group.variants[i];
			optional |= entry.markNotRequired;

			if (ScanInternal(entry.pattern, entry.func, std::string(group.name)))
			{
				return true;
			}
		}

		if (optional)
		{
			LOG(INFO) << "Optional pattern group [" << group.name << "] not found";
			return true;
		}

		LOG(WARNING) << "Failed to resolve required pattern group [" << group.name << "]";
		return false;
	}

	std::optional<std::uintptr_t> PatternScanner::FindAddress(const IPattern* pattern, const std::string& groupName) const
	{
		const auto signature = pattern->Signature();

		for (const auto* module : m_Modules)
		{
			if (!module || !module->Valid())
				continue;

			for (auto i = module->Base(); i + signature.size() <= module->End(); ++i)
			{
				const auto instruction = reinterpret_cast<const std::uint8_t*>(i);
				bool found = true;
				for (std::size_t instructionIdx = 0; instructionIdx < signature.size(); ++instructionIdx)
				{
					if (signature[instructionIdx] && signature[instructionIdx].value() != instruction[instructionIdx])
					{
						found = false;
						break;
					}
				}

				if (found)
				{
					LOG(INFO) << "Found pattern [" << pattern->Name() << "] at 0x"
					          << std::hex << i << " in " << module->Name();
					return i;
				}
			}
		}

		return std::nullopt;
	}

	bool PatternScanner::ScanInternal(const IPattern* pattern, PatternFunc func, const std::string& groupName) const
	{
		auto address = FindAddress(pattern, groupName);
		if (!address.has_value())
			return false;

		std::invoke(func, PointerCalculator(address.value(), pattern->Name().data()));
		return true;
	}

	PatternHash PatternScanner::ComputeGroupHash(const PatternGroup& group) const
	{
		PatternHash hash;
		auto fnv64 = Utils::Fnv64(group.name);
		hash = hash.Update(fnv64);

		for (const auto* module : m_Modules)
		{
			if (module && module->Valid())
				hash = hash.Update(module->Size());
		}

		for (const auto& entry : group.variants)
			hash = hash.Update(entry.pattern->Hash().m_Hash);

		return hash;
	}

	LRESULT CALLBACK PatternScanner::ProgressProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CLOSE:
			DestroyWindow(hwnd);
			m_hDialog = nullptr;
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	void PatternScanner::ShowProgressDialog(size_t total)
	{
		m_total = total;

		INITCOMMONCONTROLSEX icex{ sizeof(icex), ICC_PROGRESS_CLASS };
		InitCommonControlsEx(&icex);

		HINSTANCE inst = GetModuleHandle(nullptr);

		WNDCLASSA wc{};
		wc.lpfnWndProc = ProgressProc;
		wc.hInstance = inst;
		wc.lpszClassName = "FStopPatternScanProgress";
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

		RegisterClassA(&wc);

		int width = 500;
		int height = 250;
		int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

		m_hDialog = CreateWindowExA(
			WS_EX_APPWINDOW,
			"FStopPatternScanProgress",
			"FStopPatch",
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
			x, y, width, height,
			nullptr, nullptr, inst, nullptr
		);

		std::string text = "Welcome to FStopPatch!\r\nPlease wait while signatures are scanned...";

		m_hLabel = CreateWindowA(
			"EDIT",
			text.c_str(),
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_LEFT,
			10, 10, width - 20, height - 70,
			m_hDialog, nullptr, inst, nullptr
		);

		m_hProgress = CreateWindowExA(
			0, PROGRESS_CLASS, nullptr,
			WS_CHILD | WS_VISIBLE,
			10, height - 50, width - 20, 25,
			m_hDialog, nullptr, inst, nullptr
		);

		SendMessageA(m_hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, static_cast<int>(m_total)));
		SendMessageA(m_hProgress, PBM_SETPOS, 0, 0);

		ShowWindow(m_hDialog, SW_SHOW);
		UpdateWindow(m_hDialog);
	}

	void PatternScanner::UpdateProgress(size_t current, size_t total)
	{
		m_current = current;
		m_total = total;

		if (m_hProgress)
			SendMessageA(m_hProgress, PBM_SETPOS, static_cast<WPARAM>(m_current), 0);

		if (m_hLabel)
		{
			std::string text =
				"Scanning for functions in " + GetCurrentModule() + "...\r\n"
				"Progress: " + std::to_string(m_current) + "/" + std::to_string(m_total) + "\r\n";

			SetWindowTextA(m_hLabel, text.c_str());
		}

		UpdateWindow(m_hDialog);
	}

	void PatternScanner::CloseProgressDialog()
	{
		if (m_hDialog)
			DestroyWindow(m_hDialog);

		m_hDialog = nullptr;
		m_hProgress = nullptr;
		m_hLabel = nullptr;
	}
}
