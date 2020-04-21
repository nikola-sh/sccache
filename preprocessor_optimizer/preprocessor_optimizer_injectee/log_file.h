#pragma once

#include <mutex>
#include <fstream>
#include <string>

class LogFile
{
public:
    LogFile()
    {
        m_log.open(L"M:\\log.txt", std::ios::binary);
        m_log.write("\xff\xfe", 2);
    }

    void Write(wchar_t const* msg)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_log.write((char*)msg, wcslen(msg) * sizeof(wchar_t));
        m_log.write((char*)L"\n", 2);
        m_log.flush();
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
