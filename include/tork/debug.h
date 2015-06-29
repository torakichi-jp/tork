#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <cstdio>
#include <windows.h>
#include <crtdbg.h>

namespace tork {
    void DbgTrace(const char* msg, ...);
    void DbgBox(const char* msg, ...);

    // メモリリーク検出クラス
    class MemoryLeakDetection {
        _CrtMemState mem_state_;
        const char* p_file_ = nullptr;
        int line_ = 0;
        bool is_break_ = true;
    public:
        MemoryLeakDetection(const char* pFile, int line, bool isBreak = true);
        ~MemoryLeakDetection();

        void checkpoint(const char* pFile, int line);
        void dump() const;

    };  // class MemoryLeakDetection;
}


#ifdef _DEBUG

#define T_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

#define DebugOnly(arg) arg
#define DebugTrace(msg, ...) tork::DbgTrace(msg, __VA_ARGS__)
#define DebugPrint(msg, ...) fprintf(stderr, msg, __VA_ARGS__)
#define DebugBox(msg, ...) tork::DbgBox(msg, __VA_ARGS__)
#define DebugBreakIf(cond) (cond ? DebugBreak() : static_cast<void>(0))
#define DebugDetectMemoryLeak() tork::MemoryLeakDetection memoryLeakDetectionObject(__FILE__, __LINE__)

#else   // _DEBUG

#define T_NEW new

#define DebugOnly(arg)
#define DebugTrace(msg, ...)
#define DebugPrint(msg, ...)
#define DebugBox(msg, ...)
#define DebugBreakIf(cond)
#define DebugDetectMemoryLeak()

#endif  // _DEBUG

#endif  // DEBUG_H_INCLUDED

