#include "Hooks.h"
#include "managers/OnlinePatchManager.h"
#include "core/hooking/BaseHook.h"
#include "core/hooking/DetourHook.h"
#include "core/logger/LogHelper.h"

namespace FStop::Hooks::AI
{
	void* __cdecl GetSinglePlayer()
	{
		auto& pointers = OnlinePatchManager::GetInstance().GetPointers();
		if (pointers.UTIL_PlayerByIndex)
		{
			void* player = pointers.UTIL_PlayerByIndex(1);
			if (player)
			{
				return player;
			}

			player = pointers.UTIL_PlayerByIndex(2);
			if (player)
			{
				return player;
			}
		}

		auto hook = BaseHook::Get<GetSinglePlayer, DetourHook<decltype(&GetSinglePlayer)>>();
		assert(hook);
		if (hook)
		{
			void* orig = hook->Original()();
			if (orig)
				return orig;
		}

		return nullptr;
	}
}
