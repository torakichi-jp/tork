//******************************************************************************
//
// 所有権移動スマートポインタ
//
//******************************************************************************
#ifndef TORK_MEMORY_UNIQUE_PTR_INCLUDED
#define TORK_MEMORY_UNIQUE_PTR_INCLUDED

namespace tork {

    namespace impl {

        struct deleter_has_pointer {
            template<class T, class D>
                static typename D::pointer check(typename D::pointer);

            template<class T, class D>
                static T* check(...);
        };

    }   // namespace tork::impl


template<class T, class D = default_deleter<T>>
class unique_ptr {
public:
    typedef T element_type;
    typedef D deleter_type;

    typedef decltype(impl::deleter_has_pointer::check<T, D>(nullptr)) pointer;

private:
    pointer ptr_ = nullptr;
    deleter_type deleter_;

public:

    // デフォルトコンストラクタ
    unique_ptr() : ptr_(nullptr) { }

    // ポインタを受け取るコンストラクタ
    explicit unique_ptr(pointer ptr)
        :ptr_(ptr), deleter_(default_deleter<T>()) { }

    // ポインタと削除子
    unique_ptr(pointer ptr, const D& deleter)
        :ptr_(ptr), deleter_(deleter) { }
    unique_ptr(pointer ptr, D&& deleter)
        :ptr_(ptr), deleter_(std::move(deleter)) { }

    // nullptr
    unique_ptr(nullptr_t) :p_holder_(nullptr) { }

    // ムーブコンストラクタ
    unique_ptr(unique_ptr&& other)
        :ptr_(other.release()), deleter_(std::forward<D>(other.get_deleter()))
    {

    }

    // 別の型からのムーブコンストラクタ
    template<class U, class E,
        class = typename std::enable_if<!std::is_array<U>::value
            && std::is_convertible<typename unique_ptr<U, E>::pointer,
                pointer>::value
            && ((std::is_reference<D>::value && std::is_same<D, E>::value)
                || (!std::is_reference<D>::value
                    && std::is_convertible<E, D>::value)),
            void>::type>
    unique_ptr(unique_ptr<U, E>&& other)
        :ptr_(other.release()), deleter_(std::forward<E>(other.get_deleter()))
    {

    }

    // コピー構築禁止
    unique_ptr(const unique_ptr&) = delete;

    // デストラクタ
    ~unique_ptr()
    {
        if (ptr_) {
            deleter_(ptr_);
        }
    }

    // ムーブ代入
    unique_ptr& operator =(unique_ptr&& other)
    {
        assert(ptr_ != other.ptr_ || ptr_ == nullptr);
        reset(other.release());
        deleter_ = std::move(other.deleter_);
        return *this;
    }

    // 別の型からのムーブ代入
    template<class U, class E,
        class = typename std::enable_if<
            std::convertible<unique_ptr<U, E>::pointer, pointer>::value
            && !std::is_array<U>::value,
        void>::type>
    unique_ptr& operator =(unique_ptr<U, E>&& other)
    {
        assert(ptr_ != other.get() || ptr_ == nullptr);
        reset(other.release());
        deleter_ = std::forward<E>(other.get_deleter());
        return *this;
    }

    // nullptr代入
    unique_ptr& operator =(nullptr_t)
    {
        reset();
        return *this;
    }

    // コピー代入禁止
    unique_ptr& operator =(const unique_ptr&) = delete;

    // リソース取得
    pointer get() const { return ptr_; }

    // リソースの所有権放棄
    pointer release()
    {
        pointer pRet = ptr_;
        ptr_ = nullptr;
        return pRet;
    }

    // 削除子への参照取得
    deleter_type& get_deleter() { return deleter_; }
    const deleter_type& get_deleter() const { return deleter_; }

    // リセット
    void reset(pointer p = pointer())
    {
        pointer pOld = ptr_;
        ptr_ = p;
        if (pOld) {
            deleter_(pOld);
        }
    }

};  // class unique_ptr

}   // namespace tork

#endif  // TORK_MEMORY_UNIQUE_PTR_INCLUDED

