//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_OPTIONAL_H
#define ICE_UTIL_OPTIONAL_H

#ifndef ICE_CPP11_MAPPING

#include <IceUtil/Config.h>

namespace IceUtilInternal
{

struct NoneType
{
};

}

namespace IceUtil
{

/** A sentinel value used to indicate that no optional value was provided. */
const IceUtilInternal::NoneType None = {};

/** Encapsulates an optional value, which may or may not be present. */
template<typename T>
class Optional
{
public:

    typedef T element_type;

    /**
     * Constructs an empty optional with no value.
     */
    Optional() : _isSet(false)
    {
    }

    /**
     * Constructs an empty optional with no value.
     */
    Optional(IceUtilInternal::NoneType) : _isSet(false)
    {
    }
    
    /**
     * Constructs an optional as a copy of the given optional.
     * @param r The source optional.
     */
    Optional(const Optional& r) : _value(r._value), _isSet(r._isSet)
    {
    }

    /**
     * Constructs an optional with the given value.
     * @param p The initial value.
     */
    template<typename Y>
    Optional(Y p) : _value(p), _isSet(true)
    {
    }

    /**
     * Constructs an optional as a copy of the given optional.
     * @param r The source optional.
     */
    template<typename Y>
    Optional(const Optional<Y>& r) : _value(r._value), _isSet(r._isSet)
    {
    }

    ~Optional()
    {
    }

    /**
     * Resets this optional to have no value.
     */
    Optional& operator=(IceUtilInternal::NoneType)
    {
        _value = T();
        _isSet = false;
        return *this;
    }

    /**
     * Resets this optional to have the given value.
     * @param p The new value.
     */
    template<typename Y>
    Optional& operator=(Y p)
    {
        _value = p;
        _isSet = true;
        return *this;
    }

    /**
     * Resets this optional to be a copy of the given optional.
     * @param r The source optional.
     */
    template<typename Y>
    Optional& operator=(const Optional<Y>& r)
    {
        _value = r._value;
        _isSet = r._isSet;
        return *this;
    }

    /**
     * Resets this optional to be a copy of the given optional.
     * @param r The source optional.
     */
    Optional& operator=(const Optional& r)
    {
        _value = r._value;
        _isSet = r._isSet;
        return *this;
    }

    /**
     * Obtains the current value.
     * @return The current value.
     * @throws OptionalNotSetException if this optional has no value.
     */
    const T& value() const
    {
        checkIsSet();
        return _value;
    }

    /**
     * Obtains the current value.
     * @return The current value.
     * @throws OptionalNotSetException if this optional has no value.
     */
    T& value()
    {
        checkIsSet();
        return _value;
    }

    /**
     * Obtains the current value.
     * @return The current value.
     * @throws OptionalNotSetException if this optional has no value.
     */
    const T& get() const
    {
        return value();
    }

    /**
     * Obtains the current value.
     * @return The current value.
     * @throws OptionalNotSetException if this optional has no value.
     */
    T& get()
    {
        return value();
    }

    /**
     * Obtains a pointer to the current value.
     * @return A pointer to the current value.
     * @throws OptionalNotSetException if this optional has no value.
     */
    const T* operator->() const
    {
        return &value();
    }

    /**
     * Obtains a pointer to the current value.
     * @return A pointer to the current value.
     * @throws OptionalNotSetException if this optional has no value.
     */
    T* operator->()
    {
        return &value();
    }

    /**
     * Obtains the current value.
     * @return The current value.
     * @throws OptionalNotSetException if this optional has no value.
     */
    const T& operator*() const
    {
        return value();
    }

    /**
     * Obtains the current value.
     * @return The current value.
     * @throws OptionalNotSetException if this optional has no value.
     */
    T& operator*()
    {
        return value();
    }

    /**
     * Determines whether this optional has a value.
     * @return True if the optional has a value, false otherwise.
     */
    operator bool() const
    {
        return _isSet;
    }

    /**
     * Determines whether this optional has a value.
     * @return True if the optional does not have a value, false otherwise.
     */
    bool operator!() const
    {
        return !_isSet;
    }

    /**
     * Exchanges the state of this optional with another one.
     * @param other The optional value with which to swap.
     */
    void swap(Optional& other)
    {
        std::swap(_isSet, other._isSet);
        std::swap(_value, other._value);
    }

    /// \cond INTERNAL
    void __setIsSet()
    {
        _isSet = true;
    }
    /// \endcond

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

/**
 * Constructs an optional from the given value.
 * @param v The optional's initial value.
 * @return A new optional object.
 */
template<class T> inline Optional<T>
makeOptional(const T& v)
{
    return Optional<T>(v);
}

/// \cond INTERNAL
template<typename T> inline void
Optional<T>::throwOptionalNotSetException(const char* file, int line) const
{
    throw OptionalNotSetException(file, line);
}
/// \endcond

template<typename T, typename U>
inline bool operator==(const Optional<T>& lhs, const Optional<U>& rhs)
{
    if(lhs && rhs)
    {
        return *lhs == *rhs;
    }
    else
    {
        return !lhs && !rhs;
    }
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
    else
    {
        return !lhs && rhs;
    }
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
    if(!lhs)
    {
        return false;
    }
    else
    {
        return *lhs == rhs;
    }
}

template<typename T, typename Y>
inline bool operator!=(const Optional<T>& lhs, const Y& rhs)
{
    return !operator==(lhs, rhs);
}

template<typename T, typename Y>
inline bool operator<(const Optional<T>& lhs, const Y& rhs)
{
    if(lhs)
    {
        return *lhs < rhs;
    }
    else
    {
        return true;
    }
}

template<typename T, typename Y>
inline bool operator<=(const Optional<T>& lhs, const Y& rhs)
{
    return lhs < rhs || lhs == rhs;
}

template<typename T, typename Y>
inline bool operator>(const Optional<T>& lhs, const Y& rhs)
{
    return !(lhs < rhs || lhs == rhs);
}

template<typename T, typename Y>
inline bool operator>=(const Optional<T>& lhs, const Y& rhs)
{
    return !(lhs < rhs);
}

// Y vs Optional<T>

template<typename T, typename Y>
inline bool operator==(const Y& lhs, const Optional<T>& rhs)
{
    if(!rhs)
    {
        return false;
    }
    else
    {
        return lhs == *rhs;
    }
}

template<typename T, typename Y>
inline bool operator!=(const Y& lhs, const Optional<T>& rhs)
{
    return !operator==(lhs, rhs);
}

template<typename T, typename Y>
inline bool operator<(const Y& lhs, const Optional<T>& rhs)
{
    if(rhs)
    {
        return lhs < *rhs;
    }
    else
    {
        return false;
    }
}

template<typename T, typename Y>
inline bool operator<=(const Y& lhs, const Optional<T>& rhs)
{
    return lhs < rhs || lhs == rhs;
}

template<typename T, typename Y>
inline bool operator>(const Y& lhs, const Optional<T>& rhs)
{
    return !(lhs < rhs || lhs == rhs);
}

template<typename T, typename Y>
inline bool operator>=(const Y& lhs, const Optional<T>& rhs)
{
    return !(lhs < rhs);
}

}

#endif
#endif
