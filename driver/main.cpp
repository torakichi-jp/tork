#include <iostream>
#include <tork/debug.h>

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
void Test_text();            // text.h テスト

void Test_default_deleter(); // default_deleter テスト
void Test_shared_ptr();      // shared_ptr テスト
void Test_weak_ptr();        // weak_ptr テスト
void Test_unique_ptr();      // unique_ptr テスト
void Test_enable_shared_from_this(); // enabld_shared_from_this テスト
void Test_pointer_traits();  // std::pointer_traits<shared_ptr<T>> など


// エントリポイント
int main()
{
    /*
    Test_OptionStream();
    Test_optional();
    Test_pointer_traits();
    Test_default_deleter();

    Test_shared_ptr();
    Test_weak_ptr();
    Test_unique_ptr();

    Test_enable_shared_from_this();
    */
    Test_text();
    stopper();
    return 0;
}
