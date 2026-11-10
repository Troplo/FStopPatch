#pragma once
#include <string>
#include <vector>

namespace FStop
{
	std::string GetCurrentModule();
	std::vector<std::string>& SplitEngines();
	bool IsSplitEngine();

	bool IsServerModule();
	bool IsClientModule();
	bool IsEngineModule();
}
