//******************************************************************************
//
// 共有配列クラス
//
//******************************************************************************

#ifndef TORK_CONTAINER_SHARED_ARRAY_H_INCLUDED
#define TORK_CONTAINER_SHARED_ARRAY_H_INCLUDED

#include <memory>
#include <cassert>

namespace tork {

    namespace impl {

template<class T, class A>
struct SharedArrayObject {
    typedef size_t size_type;
    typedef A allocator_type;
    typedef std::allocator_traits<allocator_type> AllocTraits;

    T* p_data = nullptr;
    size_type capacity = 0;
    size_type size = 0;
    int ref_counter = 0;
    allocator_type alloc;

    // コンストラクタ
    SharedArrayObject(const A& a, size_type n)
        :p_data(nullptr), capacity(n), size(0), ref_counter(1), alloc(a)
    {
        p_data = AllocTraits::allocate(alloc, capacity);
    }

    // デストラクタ
    ~SharedArrayObject()
    {
        AllocTraits::deallocate(alloc, p_data, capacity);
    }

    // オブジェクト作成
    static SharedArrayObject* create(const A& a, size_type n)
    {
        assert(0 < n);

        using traits = AllocTraits::rebind_traits<SharedArrayObject>;
        AllocTraits::rebind_alloc<SharedArrayObject> allocObj = a;

        SharedArrayObject* p =
            traits::allocate(allocObj, sizeof(SharedArrayObject));
        try {
            traits::construct(allocObj, p, a, n);
        }
        catch (...) {
            traits::deallocate(allocObj, p, sizeof(SharedArrayObject));
            throw;
        }
        return p;
    }

    // オブジェクト破棄
    static void destroy(SharedArrayObject* p)
    {
        if (p == nullptr) return;

        p->clear();

        using traits = AllocTraits::rebind_traits<SharedArrayObject>;
        AllocTraits::rebind_alloc<SharedArrayObject> allocObj = p->alloc;

        traits::destroy(allocObj, p);
        traits::deallocate(allocObj, p, sizeof(SharedArrayObject));
    }

    // 容量を指定されたサイズに拡張する
    void expand(size_type n)
    {
        if (n <= capacity) return;

        // 削除用オブジェクト
        auto del = [this, n](T* ptr){
            AllocTraits::deallocate(alloc, ptr, n);
        };
        // 例外安全のためunique_ptrを使う
        std::unique_ptr<T, decltype(del)>
            p(AllocTraits::allocate(alloc, n), del);

        // すでに構築されている要素を新しい領域に移動
        for (size_type i = 0; i < size; ++i) {
            AllocTraits::construct(
                    alloc, &p.get()[i], std::move(p_data[i]));
        }

        // 古い領域を解放
        AllocTraits::deallocate(alloc, p_data, capacity);

        // メンバを更新
        p_data = p.release();
        capacity = n;
    }

    // 要素を末尾に追加
    template<class... Args>
    void add(Args&&... args)
    {
        if (size == capacity) {
            expand(capacity * 2);
        }
        AllocTraits::construct(
                alloc, &p_data[size], std::forward<Args>(args)...);
        ++size;
    }

    // 末尾から削除
    void pop_back()
    {
        assert(size != 0);
        AllocTraits::destroy(alloc, &p_data[size - 1]);
        --size;
    }

    // サイズ変更
    template<class Arg>
    void resize(size_type n, Arg&& value)
    {
        if (n < size) {
            while (n < size) {
                pop_back();
            }
        }
        else if (size < n) {
            if (capacity < n) expand(n);
            for (size_type i = 0; i < n - size; ++i) {
                AllocTraits::construct(
                        alloc, &p_data[size + i], std::forward<Arg>(value));
            }
            size = n;
        }
    }

    // 要素のクリア
    void clear()
    {
        if (p_data) {
            for (size_type i = 0; i < size; ++i) {
                AllocTraits::destroy(alloc, &p_data[i]);
            }
            size = 0;
        }
    }

    void inc_ref()
    {
        ++ref_counter;
    }

    void dec_ref()
    {
        --ref_counter;
        if (ref_counter == 0) {
            destroy(this);
        }
    }

};  // struct SharedArrayObject

    }   // namespace tork::impl

template<class T, class Allocator = std::allocator<T>>
class SharedArray {

public:
    typedef SharedArray<T, Allocator> ThisType;
    typedef impl::SharedArrayObject<T, Allocator> ObjType;

    typedef typename ObjType::size_type size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;

    typedef Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> AllocTraits;
    typedef typename AllocTraits::pointer pointer;
    typedef typename AllocTraits::const_pointer const_pointer;

    typedef T* iterator;
    typedef const T* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:

    ObjType* p_obj_ = nullptr;

public:

    SharedArray() {}

    ~SharedArray()
    {
        if (p_obj_) p_obj_->dec_ref();
    }

    // 末尾に追加
    void push_back(const T& value)
    {
        if (p_obj_ == nullptr) reserve(8);
        p_obj_->add(value);
    }

    // 末尾に追加（ムーブ構築）
    void push_back(T&& value)
    {
        if (p_obj_ == nullptr) reserve(8);
        p_obj_->add(std::move(value));
    }

    // 末尾に構築
    template<class... Args>
    void emplace_back(Args&&... args)
    {
        if (p_obj_ == nullptr) reserve(8);
        p_obj_->add(std::forward<Args>(args)...);
    }

    // 末尾から削除
    void pop_back()
    {
        p_obj_->pop_back();
    }

    // サイズ変更
    void resize(size_type n)
    {
        if (p_obj_ == nullptr) reserve(n);
        p_obj_->resize(n, T());
    }

    void resize(size_type n, const T& value)
    {
        if (p_obj_ == nullptr) reserve(n);
        p_obj_->resize(n, value);
    }

    // 要素のクリア
    void clear()
    {
        if (p_obj_) p_obj_->clear();
    }

    // 容量の予約
    void reserve(size_type n)
    {
        assert(n > 0);
        if (p_obj_ == nullptr) {
            // 空だったら配列オブジェクトを作成
            p_obj_ = ObjType::create(allocator_type(), n);
        }
        else if (n > capacity()) {
            p_obj_->expand(n);
        }
    }

    // 要素数
    size_type size() const { return p_obj_ ? p_obj_->size : 0; }

    // 空かどうか
    bool empty() const { return size() == 0; }

    // 容量
    size_type capacity() const { return p_obj_ ? p_obj_->capacity : 0; }

    // 格納できる最大数
    size_type max_size() const { return AllocTraits::max_size(get_allocator()); }

    // アロケータ
    allocator_type get_allocator() const {
        return p_obj_ ? p_obj_->alloc : allocator_type();
    }

    // データの先頭を指すポインタ
    T* data() const { return p_obj_ ? p_obj_->p_data : nullptr; }

    // 先頭要素への参照
    reference front() { return *data(); }
    const_reference front() const { return *data(); }

    // 末尾要素への参照
    reference back() { return data()[size() - 1]; }
    const_reference back() const { return data()[size() - 1]; }

    // begin
    iterator begin() { return p_obj_ ? data() : nullptr; }
    const_iterator begin() const { return p_obj_ ? data() : nullptr; }

    // end
    iterator end() { return p_obj_ ? data() + size() : nullptr; }
    const_iterator end() const { return p_obj_ ? data() + size() : nullptr; }

    // rbegin
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

    // rend
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    // constイテレータ
    const_iterator cbegin() const {
        return static_cast<const ThisType*>(this)->begin();
    }
    const_iterator cend() const {
        return static_cast<const ThisType*>(this)->end();
    }

    // const逆イテレータ
    const_reverse_iterator crbegin() const {
        return static_cast<const ThisType*>(this)->rbegin();
    }
    const_reverse_iterator crend() const {
        return static_cast<const ThisType*>(this)->rend();
    }

};  // class SharedArray

}

#endif  // TORK_CONTAINER_SHARED_ARRAY_H_INCLUDED

