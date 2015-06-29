#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <cstdio>
#include <windows.h>
#include <crtdbg.h>

namespace tork {

    void DbgTrace(const char* msg, ...);    // デバッグトレース
    void DbgBox(const char* msg, ...);      // デバッグボックス

    // メモリリーク検出クラス
    class MemoryLeakDetection {
        _CrtMemState mem_state_;        // checkpoint() を読んだ時のステータス
        const char* p_file_ = nullptr;  // ファイル名
        int line_ = 0;                  // 行番号
        bool is_break_ = true;          // ブレークするかどうか

    public:

        // コンストラクタ
        // pFile:   リーク検出時に表示するファイル名
        // line:    リーク検出時に表示する行番号
        // isBreak: リーク検出時にブレークするかどうか
        MemoryLeakDetection(const char* pFile, int line, bool isBreak = true);

        // デストラクタ
        ~MemoryLeakDetection();

        void checkpoint(const char* pFile, int line);
        void dump() const;

    };  // class MemoryLeakDetection;
}


#ifdef _DEBUG

// デバッグ用 new 演算子
// ダンプ時にメモリ確保した時のファイル名と行番号を出力
#define T_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

//------------------------------------------------------------------------------
// デバッグ用各種マクロ定義
// _DEBUG 未定義時には削除される
//
// DebugOnly        デバッグ時のみ引数評価
// DebugTrace       printf的デバッグトレース 出力ウィンドウに出力
// DebugPrint       printf的デバッグプリント 標準エラー出力
// DebugBox         printf的デバッグボックス メッセージボックス出力
// DebugBreakIf     条件付きブレークポイント
//
// DebugDetectMemoryLeak    指定した名前のメモリリーク検出オブジェクトを定義
//                          スコープの先頭で呼んでおくと、スコープ終了時にリーク検出
//------------------------------------------------------------------------------

#define DebugOnly(arg) arg
#define DebugTrace(msg, ...) tork::DbgTrace(msg, __VA_ARGS__)
#define DebugPrint(msg, ...) fprintf(stderr, msg, __VA_ARGS__)
#define DebugBox(msg, ...) tork::DbgBox(msg, __VA_ARGS__)
#define DebugBreakIf(cond) (cond ? DebugBreak() : static_cast<void>(0))
#define DebugDetectMemoryLeak(obj) tork::MemoryLeakDetection obj(__FILE__, __LINE__)

#else   // _DEBUG

#define T_NEW new

#define DebugOnly(arg)
#define DebugTrace(msg, ...)
#define DebugPrint(msg, ...)
#define DebugBox(msg, ...)
#define DebugBreakIf(cond)
#define DebugDetectMemoryLeak(obj)

#endif  // _DEBUG

#endif  // DEBUG_H_INCLUDED

