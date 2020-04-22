#pragma once

#include <windows.h>

#include <mutex>
#include <fstream>
#include <sstream>
#include <string>

class LogFile
{
public:
    LogFile()
    {
        /*std::wostringstream logName;
        logName << L"M:\\pp_optimizer." << ::GetCurrentProcessId() << L".log";

        m_log.open(logName.str().c_str(), std::ios::binary);
        m_log.write("\xff\xfe", 2);*/
    }

    void Write(wchar_t const* msg)
    {
        /*std::lock_guard<std::mutex> lock(m_mutex);

        m_log.write((char*)msg, wcslen(msg) * sizeof(wchar_t));
        m_log.write((char*)L"\n", 2);
        m_log.flush();*/
    }

    void Write(std::wstring const& str)
    {
        return Write(str.c_str());
    }

private:
    std::ofstream m_log;
    std::mutex m_mutex;
};

extern std::unique_ptr<LogFile> g_logFile;
