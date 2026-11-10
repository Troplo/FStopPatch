#pragma once
#include "BaseHook.h"
#include "core/memory/PointerCalculator.h"

#include <MinHook.h>
#include <string_view>

namespace FStop
{
	template<typename T = int*>
	class DetourHook : public BaseHook
	{
	private:
		void* m_TargetFunc;
		void* m_DetourFunc;
		void* m_OriginalFunc;

	public:
		DetourHook(const std::string_view name, void* target, T detour);
		virtual ~DetourHook();

		bool Enable() override;
		bool Disable() override;

		bool EnableNow();
		bool DisableNow();

		T Original() const;

		[[nodiscard]] uintptr_t GetTargetFunc() const
		{
			return reinterpret_cast<uintptr_t>(m_TargetFunc);
		}

	private:
		void OptimizeHook();
	};

	template<typename T>
	inline DetourHook<T>::DetourHook(const std::string_view name, void* target, T detour) :
	    BaseHook(name),
	    m_TargetFunc(target),
	    m_DetourFunc(reinterpret_cast<void*>(detour)),
	    m_OriginalFunc(nullptr)
	{
		LOG(INFO) << "Adding hook: " << name;

		if (!target || !FStop::ProxyRegistry::Unwrap(target))
		{
			LOGF(FATAL, "Failed to hook {} if it's marked as optional this is not a problem.", name);
			return;
		}
		void* actualTarget = FStop::ProxyRegistry::Unwrap(target);
		m_TargetFunc = actualTarget;
		OptimizeHook();

		const auto result = MH_CreateHook(m_TargetFunc, m_DetourFunc, &m_OriginalFunc);
		if (result != MH_OK)
		{
			LOGF(FATAL, "Failed to create hook {}", name);
		}
	}

	template<typename T>
	inline DetourHook<T>::~DetourHook()
	{
		DisableNow();
	}

	template<typename T>
	inline bool DetourHook<T>::Enable()
	{
		if (m_Enabled)
			return false;

		if (const auto result = MH_QueueEnableHook(m_TargetFunc); result != MH_OK)
		{
			LOG(INFO) << "Failed to queue hook to be enabled " << m_Name;
			return false;
		}
		m_Enabled = true;
		return true;
	}

	template<typename T>
	inline bool DetourHook<T>::Disable()
	{
		if (!m_Enabled)
			return false;

		if (const auto result = MH_QueueDisableHook(m_TargetFunc); result != MH_OK)
		{
			LOG(INFO) << "Failed to queue hook to be disabled " << m_Name;
			return false;
		}
		m_Enabled = false;
		return true;
	}

	template<typename T>
	inline bool DetourHook<T>::EnableNow()
	{
		if (m_Enabled)
			return false;

		if (const auto result = MH_EnableHook(m_TargetFunc); result != MH_OK)
		{
			LOG(INFO) << "Failed to enable hook " << m_Name;
			return false;
		}
		m_Enabled = true;
		return true;
	}

	template<typename T>
	inline bool DetourHook<T>::DisableNow()
	{
		if (!m_Enabled)
			return false;

		if (const auto result = MH_DisableHook(m_TargetFunc); result != MH_OK)
		{
			LOG(INFO) << "Failed to disable hook " << m_Name;
			return false;
		}
		m_Enabled = false;
		return true;
	}

	template<typename T>
	inline T DetourHook<T>::Original() const
	{
		return reinterpret_cast<T>(m_OriginalFunc);
	}

	template<typename T>
	inline void DetourHook<T>::OptimizeHook()
	{
		auto ptr = PointerCalculator(m_TargetFunc);
		while (ptr.As<std::uint8_t&>() == 0xE9)
		{
			ptr = ptr.Add(1).Rip();
		}
		m_TargetFunc = ptr.As<void*>();
	}
}
