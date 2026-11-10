#pragma once

#include <memory>
#include <span>
#include <vector>
#include <mutex>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <initializer_list>

namespace FStop
{
	class BytePatches
	{
	public:
		class Patch
		{
			void* m_Address;
			std::unique_ptr<uint8_t[]> m_Value;
			std::unique_ptr<uint8_t[]> m_OriginalBytes;
			std::size_t m_Size;
			bool m_Applied;

		public:
			template<typename TAddr>
			Patch(TAddr address, std::remove_pointer_t<std::remove_reference_t<TAddr>> value) :
			    m_Address(reinterpret_cast<void*>(address)),
			    m_Applied(false)
			{
				using ValType = std::remove_pointer_t<std::remove_reference_t<TAddr>>;
				m_Size = sizeof(ValType);
				m_OriginalBytes = std::make_unique<uint8_t[]>(m_Size);
				m_Value = std::make_unique<uint8_t[]>(m_Size);

				std::memcpy(m_OriginalBytes.get(), m_Address, m_Size);
				std::memcpy(m_Value.get(), &value, m_Size);
			}

			Patch(void* address, std::span<const std::uint8_t> values) :
			    m_Address(address),
			    m_Applied(false)
			{
				m_Size = values.size();
				m_OriginalBytes = std::make_unique<uint8_t[]>(m_Size);
				std::memcpy(m_OriginalBytes.get(), m_Address, m_Size);

				m_Value = std::make_unique<uint8_t[]>(m_Size);
				for (size_t i = 0; i < m_Size; i++)
					m_Value[i] = values[i];
			}

			~Patch();

			void Apply();
			void Restore();

			friend bool operator==(const std::shared_ptr<Patch>& a, const Patch* b);
		};

	private:
		static inline std::vector<std::shared_ptr<Patch>> m_Patches{};
		static inline std::mutex m_Mutex;

	public:
		template<typename TAddr>
		static std::shared_ptr<Patch> Add(TAddr address, std::remove_pointer_t<std::remove_reference_t<TAddr>> value)
		{
			std::lock_guard lock(m_Mutex);
			auto patch = std::make_shared<Patch>(address, value);
			patch->Apply();
			return m_Patches.emplace_back(std::move(patch));
		}

		static std::shared_ptr<Patch> Add(void* address, std::span<const std::uint8_t> values)
		{
			std::lock_guard lock(m_Mutex);
			auto patch = std::make_shared<Patch>(address, values);
			patch->Apply();
			return m_Patches.emplace_back(std::move(patch));
		}

		static std::shared_ptr<Patch> Add(void* address, std::initializer_list<std::uint8_t> values)
		{
			return Add(address, std::span<const std::uint8_t>(values.begin(), values.size()));
		}

		static void Remove(std::shared_ptr<Patch>& patch);
		static void RestoreAll();
	};

	using BytePatch = std::shared_ptr<BytePatches::Patch>;
}
