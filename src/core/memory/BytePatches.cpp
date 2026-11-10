#include "BytePatches.h"
#include "common.hpp"
#include <windows.h>

namespace FStop
{
	BytePatches::Patch::~Patch()
	{
	}

	void BytePatches::Patch::Apply()
	{
		if (m_Applied || !g_Running)
			return;

		DWORD old_protect;
		DWORD temp;
		VirtualProtect(m_Address, m_Size, PAGE_EXECUTE_READWRITE, &old_protect);
		std::memcpy(m_Address, m_Value.get(), m_Size);
		VirtualProtect(m_Address, m_Size, old_protect, &temp);
		m_Applied = true;
	}

	void BytePatches::Patch::Restore()
	{
		if (!m_Applied)
			return;

		DWORD old_protect;
		DWORD temp;
		VirtualProtect(m_Address, m_Size, PAGE_EXECUTE_READWRITE, &old_protect);
		std::memcpy(m_Address, m_OriginalBytes.get(), m_Size);
		VirtualProtect(m_Address, m_Size, old_protect, &temp);
		m_Applied = false;
	}

	bool operator==(const std::shared_ptr<BytePatches::Patch>& a, const BytePatches::Patch* b)
	{
		return a->m_Address == b->m_Address;
	}

	void BytePatches::Remove(std::shared_ptr<Patch>& patch)
	{
		if (!patch)
			return;

		std::lock_guard lock(m_Mutex);
		patch->Restore();

		if (const auto it = std::find(m_Patches.begin(), m_Patches.end(), patch); it != m_Patches.end())
		{
			m_Patches.erase(it);
		}
		patch.reset();
	}

	void BytePatches::RestoreAll()
	{
		std::lock_guard lock(m_Mutex);
		for (auto& patch : m_Patches)
		{
			patch->Restore();
		}
		m_Patches.clear();
	}
}
