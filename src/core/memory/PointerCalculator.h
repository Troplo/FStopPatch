#pragma once
#include <cstdint>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <mutex>
#include <initializer_list>
#include "core/logger/LogHelper.h"

namespace FStop
{
    class ProxyRegistry
    {
    private:
        inline static std::unordered_map<void*, void*> s_Map;
        inline static std::mutex s_Mutex;

    public:
        static void Register(void* proxy, void* original)
        {
            std::lock_guard<std::mutex> lock(s_Mutex);
            s_Map[proxy] = original;
        }

        static void* Unwrap(void* proxyOrOriginal)
        {
            std::lock_guard<std::mutex> lock(s_Mutex);
            auto it = s_Map.find(proxyOrOriginal);
            if (it != s_Map.end())
                return it->second;
            return proxyOrOriginal;
        }
    };

    class PointerCalculator final
    {
    private:
        std::uintptr_t m_InternalPtr;
        const char* m_Name;

    public:
        inline PointerCalculator(void* ptr = nullptr, const char* name = "Unknown");
        inline PointerCalculator(std::uintptr_t ptr, const char* name = "Unknown");
        template<typename T>
        inline PointerCalculator(T* ptr, const char* name = "Unknown");
        virtual ~PointerCalculator() = default;

        template<typename T>
        std::enable_if_t<std::is_pointer_v<T>, T> As() const;

        template<typename T>
        std::enable_if_t<std::is_lvalue_reference_v<T>, T> As() const;

        template<typename T>
        std::enable_if_t<std::is_same_v<T, std::uintptr_t>, T> As() const;

        template<typename T>
        PointerCalculator Add(T offset) const;

        template<typename T>
        PointerCalculator Sub(T offset) const;

        PointerCalculator Rip() const;
        PointerCalculator Rel32(std::size_t dispOffset = 1) const;
        PointerCalculator Dword(std::size_t offset = 0) const;
        PointerCalculator Qword(std::size_t offset = 0) const;

        explicit operator bool() const;
        friend bool operator==(const PointerCalculator& a, const PointerCalculator& b);
        friend bool operator!=(const PointerCalculator& a, const PointerCalculator& b);

        void WriteBytes(std::span<const std::byte> bytes) const;
        void WriteBytes(std::initializer_list<std::uint8_t> bytes) const;
        template <class T>
        void WriteCall(T target) const;
    };

    inline PointerCalculator::PointerCalculator(void* ptr, const char* name) :
        m_InternalPtr(reinterpret_cast<std::uintptr_t>(ptr)),
        m_Name(name)
    {
    }

    inline PointerCalculator::PointerCalculator(std::uintptr_t ptr, const char* name) :
        m_InternalPtr(ptr),
        m_Name(name)
    {
    }

    template<typename T>
    inline PointerCalculator::PointerCalculator(T* ptr, const char* name) :
        m_InternalPtr(reinterpret_cast<std::uintptr_t>(ptr)),
        m_Name(name)
    {
    }

    template<typename T>
    inline std::enable_if_t<std::is_pointer_v<T>, T> PointerCalculator::As() const
    {
        return reinterpret_cast<T>(m_InternalPtr);
    }

    template<typename T>
    inline std::enable_if_t<std::is_lvalue_reference_v<T>, T> PointerCalculator::As() const
    {
        return *reinterpret_cast<std::add_pointer_t<std::remove_reference_t<T>>>(m_InternalPtr);
    }

    template<typename T>
    inline std::enable_if_t<std::is_same_v<T, std::uintptr_t>, T> PointerCalculator::As() const
    {
        return m_InternalPtr;
    }

    template<typename T>
    inline PointerCalculator PointerCalculator::Add(T offset) const
    {
        return PointerCalculator(m_InternalPtr + offset, m_Name);
    }

    template<typename T>
    inline PointerCalculator PointerCalculator::Sub(T offset) const
    {
        return PointerCalculator(m_InternalPtr - offset, m_Name);
    }

    inline PointerCalculator PointerCalculator::Rip() const
    {
        return Add(As<std::int32_t&>()).Add(4);
    }

    inline PointerCalculator::operator bool() const
    {
        return m_InternalPtr != 0;
    }

    inline bool operator==(const PointerCalculator& a, const PointerCalculator& b)
    {
        return a.m_InternalPtr == b.m_InternalPtr;
    }

    inline bool operator!=(const PointerCalculator& a, const PointerCalculator& b)
    {
        return a.m_InternalPtr != b.m_InternalPtr;
    }

    inline PointerCalculator PointerCalculator::Rel32(std::size_t dispOffset) const
    {
        if (!m_InternalPtr) return PointerCalculator(nullptr, m_Name);
        const auto disp = *reinterpret_cast<const std::int32_t*>(m_InternalPtr + dispOffset);
        return PointerCalculator(m_InternalPtr + dispOffset + sizeof(std::int32_t) + disp, m_Name);
    }

    inline PointerCalculator PointerCalculator::Dword(std::size_t offset) const
    {
        if (!m_InternalPtr) return PointerCalculator(nullptr, m_Name);
        auto ptr = *reinterpret_cast<const uint32_t*>(m_InternalPtr + offset);
        return PointerCalculator(ptr, m_Name);
    }

    inline PointerCalculator PointerCalculator::Qword(std::size_t offset) const
    {
        if (!m_InternalPtr) return PointerCalculator(nullptr, m_Name);
        auto ptr = *reinterpret_cast<const uint64_t*>(m_InternalPtr + offset);
        return PointerCalculator(ptr, m_Name);
    }

    inline void PointerCalculator::WriteBytes(
        std::span<const std::byte> bytes
    ) const
    {
        if (!m_InternalPtr || bytes.empty())
            return;

        DWORD oldProtect = 0;

        if (!VirtualProtect(
            reinterpret_cast<void*>(m_InternalPtr),
            bytes.size(),
            PAGE_EXECUTE_READWRITE,
            &oldProtect))
        {
            LOG(FATAL) << "Failed to change memory protection for: " << m_Name;
            return;
        }

        memcpy(
            reinterpret_cast<void*>(m_InternalPtr),
            bytes.data(),
            bytes.size()
        );

        FlushInstructionCache(
            GetCurrentProcess(),
            reinterpret_cast<void*>(m_InternalPtr),
            bytes.size()
        );

        DWORD restoredProtect = 0;

        if (!VirtualProtect(
            reinterpret_cast<void*>(m_InternalPtr),
            bytes.size(),
            oldProtect,
            &restoredProtect))
        {
            LOG(FATAL) << "Failed to restore memory protection for: " << m_Name;
        }
    }

    inline void PointerCalculator::WriteBytes(
        std::initializer_list<std::uint8_t> bytes
    ) const
    {
        if (!m_InternalPtr || bytes.size() == 0)
            return;

        WriteBytes(std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(bytes.begin()),
            bytes.size()
        ));
    }

    template<typename T>
    inline void PointerCalculator::WriteCall(T target) const
    {
        static_assert(
            std::is_pointer_v<T>,
            "WriteCall target must be a function pointer"
        );

        if (!m_InternalPtr || !target)
            return;

        const auto callAddress = m_InternalPtr;
        const auto nextInstruction = callAddress + 5;
        const auto targetAddress = reinterpret_cast<std::uintptr_t>(target);

        const auto displacement =
            static_cast<std::int64_t>(targetAddress) -
            static_cast<std::int64_t>(nextInstruction);

        if (displacement < INT32_MIN || displacement > INT32_MAX)
        {
            LOG(FATAL)
                << "CALL target is outside rel32 range for: "
                << m_Name;
            return;
        }

        const auto relative = static_cast<std::int32_t>(displacement);

        WriteBytes({
            0xE8,
            static_cast<std::uint8_t>(relative & 0xFF),
            static_cast<std::uint8_t>((relative >> 8) & 0xFF),
            static_cast<std::uint8_t>((relative >> 16) & 0xFF),
            static_cast<std::uint8_t>((relative >> 24) & 0xFF)
        });
    }
}
