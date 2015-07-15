//******************************************************************************
//
// ポインタホルダ
//
//******************************************************************************

#ifndef TORK_MEMORY_PTR_HOLDER_H_INCLUDED
#define TORK_MEMORY_PTR_HOLDER_H_INCLUDED

#include <memory>
#include <typeinfo>
#include <type_traits>
#include <utility>

namespace tork {
    namespace impl {

    //==========================================================================
    // ポインタホルダ基底クラス
    //==========================================================================
    class ptr_holder_base {
        int ref_counter_ = 0;   // 参照カウンタ
        int weak_counter_ = 0;  // ウィークカウンタ

    protected:
        void* ptr_ = nullptr;   // 保持するポインタ

    public:
        ptr_holder_base(void* ptr) : ptr_(ptr) { }
        virtual ~ptr_holder_base() { }

        // 削除子取得
        virtual void* get_deleter(const std::type_info&) const
        {
            return nullptr;
        }

        // ポインタ取得
        void* get() { return ptr_; }

        // 各カウンタ取得
        int get_ref_counter() const { return ref_counter_; }
        int get_weak_counter() const { return weak_counter_; } 


        // 参照カウンタ増
        void add_ref()
        {
            ++ref_counter_;
            add_weak_ref();
        }

        // 参照カウンタ減
        // 0になったらリソース削除
        void release()
        {
            --ref_counter_;
            assert(ref_counter_ >= 0);
            if (ref_counter_ == 0) {
                destroy();
            }
            release_weak_ref();
        }

        // ウィークカウンタ増
        void add_weak_ref()
        {
            ++weak_counter_;
        }

        // ウィークカウンタ減
        // 0になったらホルダ削除
        void release_weak_ref()
        {
            --weak_counter_;
            assert(weak_counter_ >= 0);
            if (weak_counter_ == 0) {
                destroy_holder();
            }
        }

    private:
        virtual void destroy() = 0;         // リソース削除
        virtual void destroy_holder() = 0;  // ホルダ自身を削除

    };  // class ptr_holder_base

    //==========================================================================
    // ポインタホルダ
    //==========================================================================
    template<class T, class Deleter, class Alloc>
    class ptr_holder : public ptr_holder_base {
        Deleter deleter_;   // 削除子
        Alloc alloc_;       // アロケータ
    public:

        void destroy() override { deleter_(static_cast<T*>(ptr_)); }  // リソース削除

        // 削除子取得
        void* get_deleter(const type_info& tid) const override
        {
            return (tid == typeid(Deleter))
                ? const_cast<void*>(static_cast<const void*>(&deleter_)) :
                  nullptr;
        }

        // ホルダ作成
        static ptr_holder* create_holder(T* ptr, Deleter deleter, Alloc alloc)
        {
            // リソースがなければホルダも作らない
            if (ptr == nullptr) {
                return nullptr;
            }

            // 継承を利用して Alloc::construct() が ptr_holder の
            // private コンストラクタにアクセスできるようにするためのクラス
            struct holder_impl : ptr_holder<T, Deleter, Alloc> {
                holder_impl(T* p, Deleter d, Alloc a)
                    : ptr_holder(p, d, a) { }
            };

            // アロケータの再束縛
            using Holder = holder_impl;
            //using Holder = ptr_holder<T, Deleter, Alloc>;
            using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
            using Traits = std::allocator_traits<Allocator>;
            Allocator a = alloc;

            holder_impl* p = Traits::allocate(a, 1);
            if (p == nullptr) {
                // ホルダの領域を確保できなかったら、リークを防ぐため
                // リソースを解放しておく
                deleter(ptr);
                return nullptr;
            }

            Traits::construct(a, p, ptr, deleter, alloc);
            return p;
        }

        // ホルダ破棄（自殺するので注意して扱うこと）
        void destroy_holder() override
        {
            // アロケータの再束縛
            using Holder = ptr_holder<T, Deleter, Alloc>;
            using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
            using Traits = std::allocator_traits<Allocator>;
            Allocator a = alloc_;

            Traits::destroy(a, this);
            Traits::deallocate(a, this, 1);
        }

        // コピー禁止にする
        ptr_holder(const ptr_holder&) = delete;
        ptr_holder& operator =(const ptr_holder&) = delete;

    private:
        // コンストラクタ
        ptr_holder(T* ptr, Deleter deleter, Alloc alloc)
            :ptr_holder_base(ptr), deleter_(deleter), alloc_(alloc)
        {

        }

    };  // class ptr_holder

    //==========================================================================
    // リソース領域と同時にホルダ領域を確保するホルダ
    //==========================================================================
    template<class T, class Alloc>
    class ptr_holder_alloc : public impl::ptr_holder_base {

        // リソースを保持する領域
        typename std::aligned_storage<
            sizeof(T), std::alignment_of<T>::value>::type storage_;
        Alloc alloc_;   // アロケータ

    public:

        void destroy() override { pointer_cast<T*>(&storage_)->~T(); }

        // ホルダ作成
        template<class... Args>
        static ptr_holder_alloc* create_holder(Alloc alloc, Args&&... args)
        {
            // アロケータの再束縛
            struct holder_impl : ptr_holder_alloc<T, Alloc> {
                holder_impl(Alloc a) : ptr_holder_alloc(a) { }
            };
            using Holder = holder_impl;
            using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
            using Traits = std::allocator_traits<Allocator>;
            Allocator a = alloc;

            // ホルダ領域確保
            holder_impl* p = Traits::allocate(a, 1);
            if (p == nullptr) {
                return nullptr;
            }

            // ホルダ構築
            Traits::construct(a, p, alloc);

            // リソース構築
            ::new(&p->storage_) T(std::forward<Args>(args)...);

            return p;
        }

        // ホルダ破棄（自殺するので注意して扱うこと）
        void destroy_holder() override
        {
            // アロケータの再束縛
            using Holder = ptr_holder_alloc<T, Alloc>;
            using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
            using Traits = std::allocator_traits<Allocator>;
            Allocator a = alloc_;

            Traits::destroy(a, this);
            Traits::deallocate(a, this, 1);
        }

        // コピー禁止にする
        ptr_holder_alloc(const ptr_holder_alloc&) = delete;
        ptr_holder_alloc& operator =(const ptr_holder_alloc&) = delete;

    private:
        // コンストラクタ
        ptr_holder_alloc(Alloc alloc)
            :ptr_holder_base(&storage_), alloc_(alloc)
        {

        }

    };  // class ptr_holder_alloc


    }   // namespace tork::impl
}       // namespace tork

#endif  // TORK_MEMORY_PTR_HOLDER_H_INCLUDED

