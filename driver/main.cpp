#include <iostream>
#include <tork/debug.h>
#include <tork/define.h>

// メモリリーク検出
DebugDetectMemoryLeak(global_memory_leak_detection);

// ストッパー
void stopper()
{
    std::cout << "enter to continue." << std::flush;
    std::cin.clear();
    std::cin.get();
}

void Test_OptionStream();    // OptionStream テスト
void Test_optional();        // optional テスト
void Test_default_deleter(); // default_deleter テスト
void Test_shared_ptr();      // shared_ptr テスト
void Test_weak_ptr();        // weak_ptr テスト
void Test_unique_ptr();      // unique_ptr テスト
void Test_enable_shared_from_this();


// エントリポイント
int main()
{
    /*
    Test_OptionStream();
    Test_optional();

    Test_default_deleter();

    Test_shared_ptr();
    Test_weak_ptr();
    Test_unique_ptr();

    Test_enable_shared_from_this();
    */

    stopper();
    return 0;
}
