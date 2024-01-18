//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SHARED_PTR_H
#define ICE_SHARED_PTR_H

#include <memory>

namespace Ice
{

/**
 * A smart pointer that wraps a std::shared_ptr and provides an API similar to the IceUtil::Handle API.
 * It's the primary smart pointer for the Original Mapping.
 * \headerfile Ice/Ice.h
 */
template<typename T>
class SharedPtr
{
public:

    typedef T element_type;

    SharedPtr(T* p = nullptr) noexcept : _ptr(p)
    {
    }

    SharedPtr(const SharedPtr&) noexcept = default;

    template<typename Y>
    SharedPtr(const SharedPtr<Y>& r) noexcept : _ptr(r.underlying())
    {
    }

    SharedPtr(std::shared_ptr<T>&& r) noexcept : _ptr(std::move(r))
    {
    }

    SharedPtr(const std::shared_ptr<T>& r) noexcept : _ptr(r)
    {
    }

    ~SharedPtr() = default;

    SharedPtr& operator=(T* p) noexcept
    {
        if(_ptr.get() != p)
        {
            _ptr = std::shared_ptr<T>(p);
        }
        return *this;
    }

    template<typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& r) noexcept
    {
        if(_ptr != r.underlying())
        {
            _ptr = r.underlying();
        }
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& r) noexcept
    {
        if(_ptr != r._ptr)
        {
            _ptr = r._ptr;
        }
        return *this;
    }

    T* get() const noexcept { return _ptr.get(); }

    T* operator->() const noexcept { return _ptr.get(); }

    T& operator*() const noexcept { return *_ptr; }

    operator bool() const noexcept { return _ptr.operator bool(); }

    void swap(SharedPtr& other) noexcept { _ptr.swap(other._ptr); }

    std::shared_ptr<T>& underlying() noexcept { return _ptr; }
    const std::shared_ptr<T>& underlying() const noexcept { return _ptr; }

    template<class Y>
    static SharedPtr dynamicCast(const SharedPtr<Y>& r) noexcept
    {
        return dynamicCast(r.underlying());
    }

    template<class Y>
    static SharedPtr dynamicCast(const std::shared_ptr<Y>& r) noexcept
    {
        return SharedPtr(std::dynamic_pointer_cast<T>(r));
    }

private:

    std::shared_ptr<T> _ptr;
};

// The semantics of these comparison operators is _not_ the same as with IceUtil::Handle. IceUtil::Handle compares the
// pointed-to objects, after dereferencing the pointers. SharedPtr compares the pointers like shared_ptr.
// This does not make a significant difference for Ice::Value and Ice::Object since their default comparison operators
// in Ice 3.7 and before compares "this".

template<typename T, typename U>
inline bool operator==(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept { return lhs.underlying() == rhs.underlying(); }

#if __cplusplus >= 202002L

template<typename T, typename U>
inline std::strong_ordering operator<=>(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept { return lhs.underlying() <=> rhs.underlying(); }

#else

template<typename T, typename U>
inline bool operator!=(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept { return lhs.underlying() != rhs.underlying(); }

template<typename T, typename U>
inline bool operator<(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept { return lhs.underlying() < rhs.underlying();}

template<typename T, typename U>
inline bool operator<=(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept { return lhs.underlying() <= rhs.underlying(); }

template<typename T, typename U>
inline bool operator>(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept { return lhs.underlying() > rhs.underlying(); }

template<typename T, typename U>
inline bool operator>=(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept { return lhs.underlying() >= rhs.underlying(); }

#endif

}

#endif
