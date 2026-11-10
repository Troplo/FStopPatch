#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <cstdint>

namespace FStop
{
#define ADD_COLOR_TO_STREAM(color) "\x1b[" << int(color) << "m"
#define RESET_STREAM_COLOR "\x1b[0m"
#define HEX(value) "0x" << std::hex << std::uppercase << uint64_t(value) << std::dec << std::nouppercase

	class LogHelper final
	{
	public:
		LogHelper(const LogHelper&) = delete;
		LogHelper(LogHelper&&) = delete;
		LogHelper& operator=(const LogHelper&) = delete;
		LogHelper& operator=(LogHelper&&) = delete;

		static void Destroy();
		static bool Init(const std::string_view consoleName, const std::filesystem::path& file, const bool attachConsole = true);
		static void ToggleConsole(bool toggle);

	private:
		LogHelper() = default;
		~LogHelper();

		static LogHelper& GetInstance()
		{
			static LogHelper i{};
			return i;
		}

		void DestroyImpl();
		bool InitImpl(const std::string_view consoleName, const std::filesystem::path& file, const bool attachConsole);
		void ToggleConsoleImpl(bool toggle);

		void CloseOutputStreams();
		void OpenOutputStreams();

	private:
		bool m_Initialized = false;
		bool m_AttachConsole = false;
		bool m_DidConsoleExist = false;

		std::string_view m_ConsoleTitle;
		uint32_t m_OriginalConsoleMode = 0;
		void* m_ConsoleHandle = nullptr;

		std::ofstream m_ConsoleOut;
		std::filesystem::path m_File;
		std::ofstream m_FileOut;
	};
}
