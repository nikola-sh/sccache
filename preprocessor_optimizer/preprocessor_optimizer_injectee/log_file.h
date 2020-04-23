#pragma once

#include <windows.h>

#include <mutex>
#include <fstream>
#include <sstream>
#include <string>

// #define ENABLE_LOGGING

class LogFile
{
public:
    LogFile()
    {
#ifdef ENABLE_LOGGING
        std::wostringstream logName;
        logName << L"M:\\pp_optimizer." << ::GetCurrentProcessId() << L".log";

        m_log.open(logName.str().c_str(), std::ios::binary);
        m_log.write("\xff\xfe", 2);
#endif
    }

    void Write(wchar_t const* token1, wchar_t const* token2 = nullptr, wchar_t const* token3 = nullptr)
    {
#ifdef ENABLE_LOGGING
        std::lock_guard<std::mutex> lock(m_mutex);

        WriteToken(token1);
        WriteToken(token2);
        WriteToken(token3);
        
        m_log.flush();
#endif
    }

private:
    void WriteToken(wchar_t const* token)
    {
        if (token)
            m_log.write((char*)token, wcslen(token) * sizeof(wchar_t));
    }

private:
    std::ofstream m_log;
    std::mutex m_mutex;
};

extern std::unique_ptr<LogFile> g_logFile;
