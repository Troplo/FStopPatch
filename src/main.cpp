#include "common.hpp"
#include "core/logger/LogHelper.h"
#include "core/filemgr/FileMgr.hpp"
#include "core/memory/ModuleMgr.h"
#include "core/util/CurrentModule.h"
#include "core/util/Wine.h"
#include "managers/OnlinePatchManager.h"

#include <shlobj.h>

namespace FStop
{
	static std::filesystem::path GetAppDataPath()
	{
		PWSTR widePath = nullptr;
		if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &widePath)))
		{
			return std::filesystem::current_path();
		}

		std::filesystem::path path(widePath);
		CoTaskMemFree(widePath);
		return path;
	}

	DWORD WINAPI MainThread(LPVOID)
	{
		try
		{
			const auto appData = GetAppDataPath() / "Troplo" / FileMgr::GetProjectName();
			FileMgr::Init(appData);

			if (!ModuleMgr.LoadModules())
			{
				return FALSE;
			}

			bool attachConsole = true;
#if !__DEBUG
			attachConsole = false;
#endif
			LogHelper::Init("FStopPatch", FileMgr::GetProjectFile("cout.log"), attachConsole);

			LOGF(INFO, "Welcome to FStopPatch v{} (commit {})! Built {} at {}",
				PROJECT_VERSION, PROJECT_GIT_COMMIT_HASH, __DATE__, __TIME__);
			LOG(INFO) << "Target Module: " << GetCurrentModule();
			LOGF(INFO, "Execution Base: 0x{:X}", reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)));

			if (IsWine())
			{
				LOG(INFO) << "Running under Wine / Proton environment";
			}

			OnlinePatchManager::GetInstance().Init();

			while (g_Running)
			{
				std::this_thread::sleep_for(100ms);
			}

			OnlinePatchManager::GetInstance().Shutdown();
			LogHelper::Destroy();
		}
		catch (...)
		{
			LOG(FATAL) << "An unhandled exception occurred in main thread.";
		}

		return TRUE;
	}
}

extern "C" __declspec(dllexport) int __stdcall RIB_Main(
	void* provider_handle,
	unsigned int up_down,
	void* alloc_provider_handle,
	void* register_interface,
	void* unregister_interface)
{
	return 1;
}

BOOL WINAPI DllMain(HINSTANCE dllInstance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(dllInstance);
		FStop::g_DllInstance = dllInstance;

		HMODULE hSelf = nullptr;
		GetModuleHandleExA(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
			reinterpret_cast<LPCSTR>(dllInstance),
			&hSelf
		);

		FStop::g_MainThread = CreateThread(nullptr, 0, FStop::MainThread, nullptr, 0, &FStop::g_MainThreadId);
		break;
	}

	case DLL_PROCESS_DETACH:
		FStop::g_Running = false;
		break;
	}

	return TRUE;
}
