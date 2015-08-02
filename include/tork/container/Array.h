//******************************************************************************
//
// 配列
//
//******************************************************************************

#ifndef TORK_ARRAY_H_INCLUDED
#define TORK_ARRAY_H_INCLUDED

#include <memory>
#include <iterator>
#include <utility>
#include <type_traits>
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include "../memory/allocator.h"
#include "../memory/unique_ptr.h"

namespace tork {

    namespace impl {

template<class T, class A>
struct ArrayBase {
    typedef size_t size_type;
    typedef A allocator_type;

    typedef std::allocator_traits<allocator_type> alloc_traits;

    allocator_type alloc_;
    T* data_ = nullptr;
    size_type capacity_ = 0;
    size_type size_ = 0;


    ArrayBase(const allocator_type& a, size_type n)
        :alloc_(a), data_(alloc_traits::allocate(alloc_, n)),
        capacity_(n), size_(0)
    {

    }

    ~ArrayBase()
    {
        alloc_traits::deallocate(alloc_, data_, capacity_);
    }

};  // class ArrayBase


    }   // namespace tork::impl

//==============================================================================
// 配列クラス
//==============================================================================
template<class T, class Allocator = tork::allocator<T>>
class Array {
public:
    typedef impl::ArrayBase<T, Allocator> Base;
    typedef Array<T, Allocator> ThisType;

    typedef typename Base::size_type size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;

    typedef typename Base::allocator_type allocator_type;

    typedef std::allocator_traits<allocator_type> AllocTraits;
    typedef typename AllocTraits::pointer pointer;
    typedef typename AllocTraits::const_pointer const_pointer;

    typedef T* iterator;
    typedef const T* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
    Base* p_base_ = nullptr;

public:

    // デフォルトコンストラクタ
    Array() { }

    explicit Array(const Allocator& a)
        :p_base_(create_base(8, a))
    {

    }

    // サイズ（＋アロケータ）
    explicit Array(size_type n, const Allocator& a = Allocator())
        :p_base_(create_base(n, a))
    {
        resize(n);
    }

    // サイズと値（＋アロケータ）
    Array(size_type n, const T& value,
            const Allocator& a = Allocator())
        :p_base_(create_base(n, a))
    {
        resize(n, value);
    }

    // イテレータ（＋アロケータ）
    template<class InputIter,
        class = typename std::enable_if<
            !std::is_integral<InputIter>::value, void>::type>
    Array(InputIter first, InputIter last,
            const Allocator& a = Allocator())
        :p_base_(nullptr)
    {
        ConstructByIter(first, last, a,
                typename std::iterator_traits<InputIter>::iterator_category());
    }

    // コピーコンストラクタ
    Array(const Array& other)
        :Array(other,
                AllocTraits::select_on_container_copy_construction(other.get_allocator()))
    {

    }

    // 他のArrayとアロケータ
    Array(const Array& other, const Allocator& a)
        :Array(other.begin(), other.end(), a)
    {

    }

    // ムーブコンストラクタ
    Array(Array&& other)
        :p_base_(other.p_base_)
    {
        other.p_base_ = nullptr;
    }

    // 他のArray(rvalue)とアロケータ
    Array(Array&& other, const Allocator& a)
        :p_base_(nullptr)
    {
        if (other.get_allocator() == a) {
            p_base_ = other.p_base_;
            p_base_->alloc_ = a;
        }
        else if (!other.empty()) {
            unique_ptr<Base, BaseDeleter>
                p(create_base(other.size(), a), BaseDeleter());
            if (p == nullptr || p->data_ == nullptr) return;
            for (size_type i = 0; i < other.size(); ++i) {
                AllocTraits::construct(a, &p->data_[i], std::move(other[i]));
            }
            p->size_ = other.size();
            p_base_ = p.release();
            destroy_base(other.p_base_);
        }
        else {
            p_base_ = create_base(8, a);
            destroy_base(other.p_base_);
        }
        other.p_base_ = nullptr;
    }

    // 初期化子リスト
    Array(std::initializer_list<T> il,
            const Allocator& a = Allocator())
        :Array(il.begin(), il.end(), a)
    {

    }

    // デストラクタ
    ~Array()
    {
        destroy_base(p_base_);
    }

    // コピー演算子
    Array& operator =(const Array& other)
    {
        if (*this == other) return *this;
        assign(other.begin(), other.end());
        return *this;
    }

    // ムーブ演算子
    Array& operator =(Array&& other)
    {
        if (*this == other) return *this;

        destroy_base(p_base_);
        p_base_ = other.p_base_;
        other.p_base_ = nullptr;

        return *this;
    }

    // 初期化子リスト代入
    Array& operator =(std::initializer_list<T> il)
    {
        assign(il.begin(), il.end());
        return *this;
    }

    // 要素の割り当て
    template<class InputIter,
        class = typename std::enable_if<
            !std::is_integral<InputIter>::value, void>::type>
    void assign(InputIter first, InputIter last)
    {
        Base* p = CreateByIter(first, last, get_allocator(),
                typename std::iterator_traits<InputIter>::iterator_category());
        if (p == nullptr) return;
        destroy_base(p_base_);
        p_base_ = p;
    }

    void assign(size_type n, const T& u)
    {
        clear();
        resize(n, u);
    }

    void assign(std::initializer_list<T> il)
    {
        assign(il.begin(), il.end());
    }

    // 末尾に追加
    void push_back(const T& value)
    {
        expand_capacity();
        if (p_base_ == nullptr) return;

        AllocTraits::construct(
                p_base_->alloc_, &data()[size()], value);
        ++p_base_->size_;
    }

    // 末尾に追加（ムーブ構築）
    void push_back(T&& value)
    {
        expand_capacity();
        if (p_base_ == nullptr) return;

        AllocTraits::construct(
                p_base_->alloc_, &data()[size()], std::move(value));
        ++p_base_->size_;
    }

    // 末尾に構築
    template<class... Args>
    void emplace_back(Args&&... args)
    {
        expand_capacity();
        if (p_base_ == nullptr) return;

        AllocTraits::construct(
                p_base_->alloc_, &data()[size()], std::forward<Args>(args)...);
        ++p_base_->size_;
    }

    // 末尾から削除
    void pop_back()
    {
        assert(!empty());
        AllocTraits::destroy(p_base_->alloc_, &data()[size() - 1]);
        --p_base_->size_;
    }

    // サイズ変更
private:
    template<class Arg>
    void ResizeImpl(size_type sz, Arg&& value)
    {
        if (sz < size()) {
            for (size_type i = 0; i < size() - sz; ++i) {
                pop_back();
            }
        }
        else if (sz > size()) {
            reserve(sz);
            if (p_base_ == nullptr) return;
            for (size_type i = 0; i < sz - size(); ++i) {
                AllocTraits::construct(
                        p_base_->alloc_, &data()[size() + i], std::forward<Arg>(value));
            }
            p_base_->size_ = sz;
        }
    }
public:
    void resize(size_type sz)
    {
        ResizeImpl(sz, T());
    }

    void resize(size_type sz, const T& value)
    {
        ResizeImpl(sz, value);
    }

    // 要素のクリア
    void clear()
    {
        if (p_base_ == nullptr) return;

        for (size_type i = 0; i < size(); ++i) {
            AllocTraits::destroy(p_base_->alloc_, &data()[i]);
        }
        p_base_->size_ = 0;
    }

    // 容量の予約
    void reserve(size_type s)
    {
        if (p_base_ == nullptr) {
            // 空だったらベースを作る
            p_base_ = create_base(s, allocator_type());
            return;
        }
        else if (s <= capacity()) {
            // 指定された容量が現在の容量よりも小さければ
            // 何もしない
            return;
        }

        unique_ptr<Base, BaseDeleter>
            p(create_base(s, p_base_->alloc_), BaseDeleter());
        if (p == nullptr || p->data_ == nullptr) return;
        // 要素のムーブ
        for (size_type i = 0; i < size(); ++i) {
            AllocTraits::construct(
                    p->alloc_, &p->data_[i], std::move(p_base_->data_[i]));
        }
        p->size_ = size();

        // 古い要素の削除
        destroy_base(p_base_);

        p_base_ = p.release();
    }

    // 容量をサイズにフィットさせる
    void shrink_to_fit()
    {
        Array<T, Allocator>(*this).swap(*this);
    }

    // スワップ
    void swap(Array& other)
    {
        std::swap(p_base_, other.p_base_);
    }

    // 要素への添え字アクセス
    reference at(size_type i)
    {
        if (p_base_ == nullptr || i >= size())
            throw std::out_of_range("tork::Array out of range access");
        return p_base_->data_[i];
    }
    const_reference at(size_type i) const
    {
        if (p_base_ == nullptr || i >= size())
            throw std::out_of_range("tork::Array out of range access");
        return p_base_->data_[i];
    }

    // operator []
    reference operator [](size_type i)
    {
        return p_base_->data_[i];
    }
    const_reference operator [](size_type i) const
    {
        return p_base_->data_[i];
    }

    // 容量
    size_type capacity() const { return p_base_ ? p_base_->capacity_ : 0; }

    // 要素数
    size_type size() const { return p_base_ ? p_base_->size_ : 0; }

    // 格納できる最大数
    size_type max_size() const { return AllocTraits::max_size(get_allocator()); }

    // アロケータ
    allocator_type get_allocator() const {
        return p_base_ ? p_base_->alloc_ : allocator_type();
    }

    // 空かどうか
    bool empty() const { return size() == 0; }

    // データの先頭を指すポインタ
    T* data() const { return p_base_ ? p_base_->data_ : nullptr; }

    // 先頭要素の参照
    reference front() { return *data(); }
    const_reference front() const { return *data(); }

    // 末尾要素の参照
    reference back() { return data()[size() - 1]; }
    const_reference back() const { return data()[size() - 1]; }

    // 最初の要素を指すイテレータ
    iterator begin() { return p_base_ ? data() : nullptr; }
    const_iterator begin() const { return p_base_ ? data() : nullptr; }

    // 最後の要素の次を指すイテレータ
    iterator end() { return p_base_ ? data() + size() : nullptr; }
    const_iterator end() const { return p_base_ ? data() + size() : nullptr; }

    // 最後の要素を指す逆イテレータ
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

    // 最初の要素の前を指す逆イテレータ
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

private:

    // 容量を拡張する
    void expand_capacity(size_type first_size = 8)
    {
        if (capacity() == 0) {
            reserve(first_size);
        }
        else if (size() == capacity()) {
            reserve(capacity() * 2);
        }
    }

    // 配列ベース作成
    static Base* create_base(size_type s, const allocator_type& alloc)
    {
        using traits = AllocTraits::rebind_traits<Base>;
        AllocTraits::rebind_alloc<Base> a = alloc;
        Base* p = traits::allocate(a, sizeof(Base));
        try {
            traits::construct(a, p, alloc, s);
        }
        catch (...) {
            traits::deallocate(a, p, sizeof(Base));
            throw;
        }
        return p;
    }

    // 配列ベース破棄
    static void destroy_base(Base* p)
    {
        if (p) {
            for (size_type i = 0; i < p->size_; ++i) {
                AllocTraits::destroy(p->alloc_, &p->data_[i]);
            }

            using traits = AllocTraits::rebind_traits<Base>;
            AllocTraits::rebind_alloc<Base> a = p->alloc_;

            traits::destroy(a, p);
            traits::deallocate(a, p, sizeof(Base));
        }
    }

    struct BaseDeleter {
        void operator ()(Base* p) {
            Array<T, Allocator>::destroy_base(p);
        }
    };

    // 入力イテレータによる構築
    template<class InputIter>
    void ConstructByIter(InputIter first, InputIter last,
            const Allocator& a, std::input_iterator_tag)
    {
        p_base_ = create_base(8, a);
        if (p_base_ == nullptr || p_base_->data_ == nullptr) return;

        for (auto it = first; it != last; ++it) {
            emplace_back(*it);
        }
    }

    // 前進イテレータによる構築
    template<class ForwardIter>
    void ConstructByIter(ForwardIter first, ForwardIter last,
            const Allocator& a, std::forward_iterator_tag)
    {
        unique_ptr<Base, BaseDeleter>
            p(create_base(std::distance(first, last), a), BaseDeleter());
        if (p == nullptr || p->data_ == nullptr) return;

        size_type i = 0;
        for (auto it = first; it != last; ++it) {
            AllocTraits::construct(p->alloc_, &p->data_[i], *it);
            ++i;
        }
        p->size_ = i;
        p_base_ = p.release();
    }

    // 入力イテレータによる構築
    template<class InputIter>
    Base* CreateByIter(InputIter first, InputIter last,
            const Allocator& a, std::input_iterator_tag)
    {
        unique_ptr<Base, BaseDeleter>
            p(create_base(8, a), BaseDeleter());
        if (p == nullptr || p->data_ == nullptr) return nullptr;

        for (auto it = first; it != last; ++it) {
            size_type& sz = p->size_;
            AllocTraits::construct(p->alloc_, &p->data_[sz], *it);
            ++sz;
            // 容量がいっぱいになった
            if (p->capacity_ == sz) {
                unique_ptr<Base, BaseDeleter>
                    tmp(create_base(p->capacity_ * 2, p->alloc_), BaseDeleter());
                if (tmp == nullptr || p->data_ == nullptr) return nullptr;
                for (size_type i = 0; i < sz; ++i) {
                    AllocTraits::construct(tmp->alloc_,
                            &tmp->data_[i], std::move(p->data_[i]));
                }
                tmp->size_ = sz;
                tmp.swap(p);
                destroy_base(tmp.release());
            }
        }
        return p.release();
    }

    // 前進イテレータによる構築
    template<class ForwardIter>
    Base* CreateByIter(ForwardIter first, ForwardIter last,
            const Allocator& a, std::forward_iterator_tag)
    {
        unique_ptr<Base, BaseDeleter>
            p(create_base(std::distance(first, last), a), BaseDeleter());
        if (p == nullptr || p->data_ == nullptr) return nullptr;

        size_type i = 0;
        for (auto it = first; it != last; ++it) {
            AllocTraits::construct(p->alloc_, &p->data_[i], *it);
            ++i;
        }
        p->size_ = i;
        return p.release();
    }
};  // class Array

// operator ==()
template<class T, class A>
bool operator ==(const Array<T, A> x, const Array<T, A> y)
{
    if (x.size() != y.size()) return false;

    for (size_t i = 0; i < x.size(); ++i) {
        if (x[i] == y[i]) continue;
        return false;
    }
    return true;
}

}   // namespace tork

#endif  // TORK_ARRAY_H_INCLUDED

