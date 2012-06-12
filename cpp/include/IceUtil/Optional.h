// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_OPTIONAL_H
#define ICE_UTIL_OPTIONAL_H

#include <IceUtil/Config.h>

namespace IceUtilInternal
{

struct NoneType 
{
};

}

namespace IceUtil
{

const IceUtilInternal::NoneType None = {};

template<typename T>
class Optional
{
public:

    typedef T element_type;

    Optional() : _isSet(false)
    {
    }

    Optional(IceUtilInternal::NoneType) : _isSet(false)
    {
    }

    Optional(const T& p) : _value(p), _isSet(true)
    {
    }
    
    template<typename Y>
    Optional(const Optional<Y>& r) : _value(r._value), _isSet(r._isSet)
    {
    }

    Optional(const Optional& r) : _value(r._value), _isSet(r._isSet)
    {
    }
    
    ~Optional()
    {
    }
    
    Optional& operator=(IceUtilInternal::NoneType)
    {
        _value = T();
        _isSet = false;
        return *this;
    }

    Optional& operator=(const T& p)
    {
        _value = p;
        _isSet = true;
        return *this;
    }
        
    template<typename Y>
    Optional& operator=(const Optional<Y>& r)
    {
        _value = r._value;
        _isSet = r._isSet;
        return *this;
    }

    Optional& operator=(const Optional& r)
    {
        _value = r._value;
        _isSet = r._isSet;
        return *this;
    }

    const T& get() const
    {
        checkIsSet();
        return _value;
    }
    T& get()
    {
        checkIsSet();
        return _value;
    }

    const T* operator->() const
    {
        return &get();
    }
    T* operator->()
    {
        return &get();
    }

    const T& operator*() const
    {
        return get();
    }
    T& operator*()
    {
        return get();
    }

    operator bool() const
    {
        return _isSet;
    }

    bool operator!() const 
    { 
        return !_isSet;
    }

    void swap(Optional& other)
    {
        std::swap(_isSet, other._isSet);
        std::swap(_value, other._value);
    }

    void __setIsSet()
    {
        _isSet = true;
    }

private:

    void checkIsSet() const
    {
        if(!_isSet)
        {
            throwOptionalNotSetException(__FILE__, __LINE__);
        }
    }
    
    void throwOptionalNotSetException(const char *, int) const;

    T _value;
    bool _isSet;
};

template<class T> inline Optional<T> 
makeOptional(const T& v)
{
    return Optional<T>(v);
}

template<typename T> inline void 
Optional<T>::throwOptionalNotSetException(const char* file, int line) const
{
    throw OptionalNotSetException(file, line);
}

template<typename T, typename U>
inline bool operator==(const Optional<T>& lhs, const Optional<U>& rhs)
{
    if(lhs && rhs)
    {
        return *lhs == *rhs;
    }

    // Note: don't use if { } else { }. This causes lots warnings when
    // compiling with GCC and optimization enabled. See bug 2330.
    return !lhs && !rhs;
}

template<typename T, typename U>
inline bool operator!=(const Optional<T>& lhs, const Optional<U>& rhs)
{
    return !operator==(lhs, rhs);
}

template<typename T, typename U>
inline bool operator<(const Optional<T>& lhs, const Optional<U>& rhs)
{
    if(lhs && rhs)
    {
        return *lhs < *rhs;
    }

    // Note: don't use if { } else { }. This causes lots warnings when
    // compiling with GCC and optimization enabled. See bug 2330.
    return !lhs && rhs;
}

template<typename T, typename U>
inline bool operator<=(const Optional<T>& lhs, const Optional<U>& rhs)
{
    return lhs < rhs || lhs == rhs;
}

template<typename T, typename U>
inline bool operator>(const Optional<T>& lhs, const Optional<U>& rhs)
{
    return !(lhs < rhs || lhs == rhs);
}

template<typename T, typename U>
inline bool operator>=(const Optional<T>& lhs, const Optional<U>& rhs)
{
    return !(lhs < rhs);
}

// Optional<T> vs Y

template<typename T, typename Y>
inline bool operator==(const Optional<T>& lhs, const Y& rhs)
{
    return operator==(lhs, Optional<Y>(rhs));
}

template<typename T, typename Y>
inline bool operator!=(const Optional<T>& lhs, const Y& rhs)
{
    return !operator==(lhs, Optional<Y>(rhs));
}

template<typename T, typename Y>
inline bool operator<(const Optional<T>& lhs, const Y& rhs)
{
    return operator<(lhs, Optional<Y>(rhs));
}

template<typename T, typename Y>
inline bool operator<=(const Optional<T>& lhs, const Y& rhs)
{
    return operator<=(lhs, Optional<Y>(rhs));
}

template<typename T, typename Y>
inline bool operator>(const Optional<T>& lhs, const Y& rhs)
{
    return operator>(lhs, Optional<Y>(rhs));
}

template<typename T, typename Y>
inline bool operator>=(const Optional<T>& lhs, const Y& rhs)
{
    return operator>=(lhs, Optional<Y>(rhs));
}

// Y vs Optional<T>

template<typename T, typename Y>
inline bool operator==(const Y& lhs, const Optional<Y>& rhs)
{
    return operator==(Optional<Y>(lhs), rhs);
}

template<typename T, typename Y>
inline bool operator!=(const Y& lhs, const Optional<Y>& rhs)
{
    return !operator==(Optional<Y>(lhs), rhs);
}

template<typename T, typename Y>
inline bool operator<(const Y& lhs, const Optional<Y>& rhs)
{
    return operator<(Optional<Y>(lhs), rhs);
}

template<typename T, typename Y>
inline bool operator<=(const Y& lhs, const Optional<Y>& rhs)
{
    return operator<=(Optional<Y>(lhs), rhs);
}

template<typename T, typename Y>
inline bool operator>(const Y& lhs, const Optional<Y>& rhs)
{
    return operator>(Optional<Y>(lhs), rhs);
}

template<typename T, typename Y>
inline bool operator>=(const Y& lhs, const Optional<Y>& rhs)
{
    return operator>=(Optional<Y>(lhs), rhs);
}

}

#endif
