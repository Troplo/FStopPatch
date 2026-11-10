#pragma once
#include "common.hpp"
#include "core/hooking/HookImpl.h"
#include "core/hooking/DetourHook.h"
#include "core/memory/BytePatches.h"
#include "core/memory/PatternScanner.h"

namespace FStop
{
	struct OnlinePointers
	{
		using UTIL_PlayerByIndex_t = void*(__cdecl*)(int playerIndex);
		using AI_GetSinglePlayer_t = void*(__cdecl*)();

		UTIL_PlayerByIndex_t UTIL_PlayerByIndex{nullptr};
		AI_GetSinglePlayer_t AI_GetSinglePlayer{nullptr};
	};

	class OnlinePatchManager final
	{
	public:
		using Pointers = OnlinePointers;

		static OnlinePatchManager& GetInstance()
		{
			static OnlinePatchManager instance;
			return instance;
		}

		Pointers& GetPointers() { return m_Pointers; }
		const Pointers& GetPointers() const { return m_Pointers; }

		bool Init();
		void Shutdown();

		[[nodiscard]] bool IsInitialized() const { return m_Initialized; }

	private:
		OnlinePatchManager() = default;
		~OnlinePatchManager();

		OnlinePatchManager(const OnlinePatchManager&) = delete;
		OnlinePatchManager(OnlinePatchManager&&) noexcept = delete;
		OnlinePatchManager& operator=(const OnlinePatchManager&) = delete;
		OnlinePatchManager& operator=(OnlinePatchManager&&) noexcept = delete;

		Pointers m_Pointers{};
		HookImpl m_HookImpl;
		bool m_Initialized{false};

		bool RegisterPatches(PatternScanner& scanner);
		bool ApplyHooks();
	};
}
