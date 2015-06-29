//******************************************************************************
//
// デバッグ用関数など
//
//******************************************************************************

#include <tork/debug.h>
#include <cstdio>
#include <cstdarg>

namespace {

    // メッセージ用のバッファを取得して、メッセージを書き込んで返す
    // 使い終わったら delete[] すること
    char* GetDbgMsgString(const char* msg, va_list args)
    {
        int len = _vscprintf(msg, args) + 1;    // +1はヌル文字分
        char* pBuffer = T_NEW char[len];
        vsprintf_s(pBuffer, len, msg, args);
        return pBuffer;
    }

}   // anonymous namespace


namespace tork {

// デバッグトレース
// printf() と同じ書式が使える
// OutputDebugString() を利用して出力
void DbgTrace(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    char* pMsg = GetDbgMsgString(msg, args);
    va_end(args);

    ::OutputDebugStringA(pMsg);

    delete[] pMsg;
}

// デバッグボックス
// printf() と同じ書式が使える
// MessageBox() を利用して出力
void DbgBox(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    char* pMsg = GetDbgMsgString(msg, args);
    va_end(args);

    ::MessageBoxA(nullptr, pMsg, "tork::DbgBox", MB_OK | MB_ICONINFORMATION);

    delete[] pMsg;
}


// MemoryLeakDetection

// コンストラクタ
// pFile:   リーク検出時に表示するファイル名
// line:    リーク検出時に表示する行番号
// isBreak: リーク検出時にブレークするかどうか
MemoryLeakDetection::MemoryLeakDetection(const char* pFile, int line, bool isBreak)
{
    is_break_ = isBreak;
    checkpoint(pFile, line);
}

// デストラクタ
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

    // 現在のチェックポイントとの差分を取得
    _CrtMemCheckpoint(&stateNow);
    _CrtMemDifference(&stateDiff, &mem_state_, &stateNow);

    // ノーマルブロックかクライアントブロックの数が0より大きければ、
    // リークが起きている
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

