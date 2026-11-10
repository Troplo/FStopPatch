#include "OnlinePatchManager.h"
#include "hooks/functions/Hooks.h"
#include "core/memory/ModuleMgr.h"
#include "core/util/CurrentModule.h"

namespace FStop
{
	OnlinePatchManager::~OnlinePatchManager()
	{
		if (m_Initialized)
		{
			Shutdown();
		}
	}

	bool OnlinePatchManager::Init()
	{
		if (m_Initialized)
			return true;

		LOG(INFO) << "[OnlinePatchManager] Initializing online patch system...";

		Module* targetModule = ModuleMgr.Get("server.dll");
		if (!targetModule)
		{
			auto targetModuleName = GetCurrentModule();
			targetModule = ModuleMgr.Get(targetModuleName);
		}

		if (targetModule)
		{
			LOGF(INFO, "[OnlinePatchManager] Targeting module: {} (Base: 0x{:X}, Size: 0x{:X})",
				targetModule->Name(), targetModule->Base(), targetModule->Size());
		}
		else
		{
			LOG(INFO) << "[OnlinePatchManager] Target module not directly resolved, using split engine scanner.";
		}

		PatternScanner scanner(targetModule);
		RegisterPatches(scanner);
		scanner.Scan();

		ApplyHooks();

		m_Initialized = true;
		LOG(INFO) << "[OnlinePatchManager] Online patch system initialized successfully.";
		return true;
	}

	bool OnlinePatchManager::RegisterPatches(PatternScanner& scanner)
	{
		LOG(INFO) << "[OnlinePatchManager] Registering signature scans and hooks...";

		auto& pointers = GetPointers();

		constexpr static auto utilPlayerByIndexPtrn =
			Pattern<"55 8B EC 8B 4D ? 85 C9 7E ? A1 ? ? ? ? 3B 48">("UTIL_PlayerByIndex");
		scanner.Add(
			"UTIL_PlayerByIndex",
			utilPlayerByIndexPtrn,
			[&pointers](PointerCalculator ptr)
			{
				pointers.UTIL_PlayerByIndex = ptr.As<OnlinePointers::UTIL_PlayerByIndex_t>();
				LOGF(INFO, "[OnlinePatchManager] Resolved UTIL_PlayerByIndex at 0x{:X}", ptr.As<uintptr_t>());
			},
			false
		);

		constexpr static auto aiGetSinglePlayerPtrn =
			Pattern<"8B 15 ? ? ? ? 8B 42 ? 83 F8">("AI_GetSinglePlayer");
		scanner.Add(
			"AI_GetSinglePlayer",
			aiGetSinglePlayerPtrn,
			[&pointers](PointerCalculator ptr)
			{
				pointers.AI_GetSinglePlayer = ptr.As<OnlinePointers::AI_GetSinglePlayer_t>();
				LOGF(INFO, "[OnlinePatchManager] Resolved AI_GetSinglePlayer (sub_103662C0) at 0x{:X}", ptr.As<uintptr_t>());
			},
			false
		);

		constexpr static auto aiInlinedCheckExactPtrn =
			Pattern<"83 78 14 01 89 5D ? 7E 04 33 C0 EB 05 E8 ? ? ? ? F7 80 20 01 00 00 00 80 00 00">("AI_InlinedCheck_Exact");
		scanner.Add(
			"AI_InlinedCheck",
			aiInlinedCheckExactPtrn,
			[](PointerCalculator ptr)
			{
				LOGF(INFO, "[OnlinePatchManager] Found inlined AI check at 0x{:X}, patching test [eax+120h] -> jmp loc_1017BBFD", ptr.As<uintptr_t>());
				BytePatches::Add(ptr.Add(18).As<void*>(), {
					0xEB, 0x2A,
					0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
				});
			},
			false
		);

		constexpr static auto aiInlinedCheckGenericPtrn =
			Pattern<"83 ? 14 01 ? ? ? 7E 04 33 C0 EB 05 E8 ? ? ? ? F7 ? 20 01 00 00 00 80 00 00">("AI_InlinedCheck_Generic");
		scanner.Add(
			"AI_InlinedCheck",
			aiInlinedCheckGenericPtrn,
			[](PointerCalculator ptr)
			{
				LOGF(INFO, "[OnlinePatchManager] Found generic inlined AI check at 0x{:X}, patching test [eax+120h] -> jmp loc_1017BBFD", ptr.As<uintptr_t>());
				BytePatches::Add(ptr.Add(18).As<void*>(), {
					0xEB, 0x2A,
					0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
				});
			},
			false
		);

		return true;
	}

	bool OnlinePatchManager::ApplyHooks()
	{
		LOG(INFO) << "[OnlinePatchManager] Applying queued hooks...";

		auto& pointers = GetPointers();
		if (pointers.AI_GetSinglePlayer && ProxyRegistry::Unwrap((void*)pointers.AI_GetSinglePlayer))
		{
			BaseHook::Add<Hooks::AI::GetSinglePlayer>(new DetourHook(
				"AI_GetSinglePlayer",
				reinterpret_cast<void*>(pointers.AI_GetSinglePlayer),
				Hooks::AI::GetSinglePlayer
			));
		}

		BaseHook::EnableAll();
		m_HookImpl.ApplyQueued();

		return true;
	}

	void OnlinePatchManager::Shutdown()
	{
		if (!m_Initialized)
			return;

		LOG(INFO) << "[OnlinePatchManager] Shutting down online patches...";

		BaseHook::DisableAll();
		m_HookImpl.ApplyQueued();
		BytePatches::RestoreAll();

		m_Initialized = false;
		LOG(INFO) << "[OnlinePatchManager] Online patches unloaded.";
	}
}
