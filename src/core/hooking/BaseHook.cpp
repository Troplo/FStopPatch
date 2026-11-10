#include "BaseHook.h"
#include "common.hpp"

namespace FStop
{
	BaseHook::BaseHook(const std::string_view name) :
	    m_Name(name),
	    m_Enabled(false)
	{
		m_Hooks.emplace_back(this);
	}

	std::vector<BaseHook*>& BaseHook::Hooks()
	{
		return m_Hooks;
	}

	void BaseHook::EnableAll()
	{
		for (auto hook : m_Hooks)
		{
			LOG(INFO) << "Enabling hook: " << hook->Name();
			hook->Enable();
		}
	}

	void BaseHook::DisableAll()
	{
		for (auto hook : m_Hooks)
		{
			hook->Disable();
		}
	}
}
