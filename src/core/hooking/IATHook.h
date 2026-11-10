#pragma once
#include "BaseHook.h"
#include "core/memory/Module.h"
#include "core/memory/PointerCalculator.h"
#include <string_view>
#include <string>

namespace FStop
{
	template<typename T = int*>
	class IATHook : public BaseHook
	{
	private:
		void** m_HookLocation;
		void* m_OriginalFunc;
		void* m_HookFunc;

	public:
		IATHook(const std::string_view name, Module* module, const std::string_view library, const std::string_view import, T detour);
		virtual ~IATHook();

		bool Enable() override;
		bool Disable() override;

		T Original() const;
	};

	template<typename T>
	inline IATHook<T>::IATHook(const std::string_view name, Module* module, const std::string_view library, const std::string_view import, T detour) :
	    BaseHook(name)
	{
		m_HookLocation = module->GetImport(std::string(library), std::string(import));
		m_OriginalFunc = m_HookLocation ? *m_HookLocation : nullptr;
		m_HookFunc = reinterpret_cast<void*>(detour);
	}

	template<typename T>
	inline IATHook<T>::~IATHook()
	{
		Disable();
	}

	template<typename T>
	inline bool IATHook<T>::Enable()
	{
		if (!m_HookLocation)
			return false;
		*m_HookLocation = m_HookFunc;
		m_Enabled = true;
		return true;
	}

	template<typename T>
	inline bool IATHook<T>::Disable()
	{
		if (!m_HookLocation)
			return false;
		*m_HookLocation = m_OriginalFunc;
		m_Enabled = false;
		return true;
	}

	template<typename T>
	inline T IATHook<T>::Original() const
	{
		return reinterpret_cast<T>(m_OriginalFunc);
	}
}
