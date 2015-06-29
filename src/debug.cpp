#include <tork/debug.h>
#include <cstdio>
#include <cstdarg>

namespace {

    char* GetDbgMsgBuffer(const char* msg, va_list args)
    {
        int len = _vscprintf(msg, args) + 1;
        char* pBuffer = T_NEW char[len];
        vsprintf_s(pBuffer, len, msg, args);
        return pBuffer;
    }

}   // anonymous namespace

namespace tork {

void DbgTrace(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    char* pBuffer = GetDbgMsgBuffer(msg, args);
    va_end(args);

    ::OutputDebugStringA(pBuffer);

    delete[] pBuffer;
}

void DbgBox(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    char* pBuffer = GetDbgMsgBuffer(msg, args);
    va_end(args);

    ::MessageBoxA(nullptr, pBuffer, "tork::DbgBox", MB_OK | MB_ICONINFORMATION);

    delete[] pBuffer;
}


// MemoryLeakDetection

MemoryLeakDetection::MemoryLeakDetection(const char* pFile, int line, bool isBreak)
{
    is_break_ = isBreak;
    checkpoint(pFile, line);
}

MemoryLeakDetection::~MemoryLeakDetection()
{
    dump();
}

// チェックポイント設定
void MemoryLeakDetection::checkpoint(const char* pFile, int line)
{
    p_file_ = pFile;
    line_ = line;
    _CrtMemCheckpoint(&mem_state_);
}

// チェックポイントから現在の差分をとって、リークしていればダンプする
void MemoryLeakDetection::dump() const
{
    _CrtMemState stateNow;
    _CrtMemState stateDiff;
    _CrtMemCheckpoint(&stateNow);
    _CrtMemDifference(&stateDiff, &mem_state_, &stateNow);
    if (stateDiff.lCounts[_NORMAL_BLOCK] > 0 || stateDiff.lCounts[_CLIENT_BLOCK] > 0) {
        if (is_break_) {
            DebugBox("Memory Leak Detected!!");
        }
        DebugTrace("Checkpoint:\n%s(%d)\n", p_file_, line_);
        _CrtMemDumpAllObjectsSince(&mem_state_);
        DebugBreakIf(is_break_);
    }
    else {
        DebugTrace("No Memory Leaks. Checkpoint:\n%s(%d)\n", p_file_, line_);
    }
}

}   // namespace tork
