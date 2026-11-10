#pragma once
#include <MinHook.h>

namespace FStop
{
	class HookImpl
	{
	public:
		HookImpl()
		{
			MH_Initialize();
		}

		~HookImpl()
		{
			MH_Uninitialize();
		}

		MH_STATUS ApplyQueued()
		{
			return MH_ApplyQueued();
		}
	};
}
