#include "CurrentModule.h"
#include <windows.h>
#include <filesystem>

namespace FStop
{
	static std::string s_CachedModule;

	std::string GetCurrentModule()
	{
		if (!s_CachedModule.empty())
			return s_CachedModule;

		const char* modules[] = {
			"server.dll",
			"client.dll",
			"engine.dll",
			"p2008.exe",
			"hl2.exe"
		};

		for (const auto* module : modules)
		{
			if (GetModuleHandleA(module) != NULL)
			{
				s_CachedModule = module;
				return s_CachedModule;
			}
		}

		char buffer[MAX_PATH];
		if (GetModuleFileNameA(NULL, buffer, MAX_PATH))
		{
			std::filesystem::path p(buffer);
			s_CachedModule = p.filename().string();
			return s_CachedModule;
		}

		return "server.dll";
	}

	std::vector<std::string>& SplitEngines()
	{
		static std::vector<std::string> splitEngines = {
			"server.dll",
			"client.dll",
			"engine.dll",
			"tier0.dll",
			"vstdlib.dll",
			"vphysics.dll",
			"materialsystem.dll",
			"studiorender.dll",
			"vguimatsurface.dll",
			"vgui2.dll",
			"filesystem_stdio.dll",
			"inputsystem.dll",
			"matchmaking.dll",
			"datacache.dll",
			"launcher.dll"
		};

		return splitEngines;
	}

	bool IsSplitEngine()
	{
		return true;
	}

	bool IsServerModule()
	{
		return GetCurrentModule() == "server.dll";
	}

	bool IsClientModule()
	{
		return GetCurrentModule() == "client.dll";
	}

	bool IsEngineModule()
	{
		return GetCurrentModule() == "engine.dll";
	}
}
