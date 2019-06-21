//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CUSTOM_BUFFER_H
#define CUSTOM_BUFFER_H

#include <Ice/Ice.h>

#if defined(_MSC_VER)
#  pragma warning( disable : 4800 )
#endif

namespace Test
{

template<typename T>
class CustomBuffer
{
public:

    CustomBuffer() : _buf(0), _count(0)
    {
    }

    CustomBuffer(const CustomBuffer& o) : _buf(0), _count(o._count)
    {
        if(_count > 0)
        {
            _buf = new T[_count];
            for(size_t i = 0; i < _count; ++i)
            {
                _buf[i] = o._buf[i];
            }
        }
    }

    ~CustomBuffer()
    {
        if(_buf != 0)
        {
            delete[] _buf;
        }
    }

    CustomBuffer& operator=(const CustomBuffer& o)
    {
        _count = o._count;
        if(_count > 0)
        {
            _buf = new T[_count];
            for(size_t i = 0; i < _count; ++i)
            {
                _buf[i] = o._buf[i];
            }
        }
        return *this;
    }

    size_t
    count() const
    {
        return _count;
    }

    T* get() const
    {
        return _buf;
    }

    void set(T* buf, size_t count)
    {
        _buf = buf;
        _count = count;
    }

    void setAndInit(T* buf, size_t count)
    {
        _buf = buf;
        _count = count;
        for(size_t i = 0; i < count; ++i)
        {
            _buf[i] = static_cast<T>(rand());
        }
    }

private:

    T* _buf;
    size_t _count;
};

template<typename T> bool
operator!=(const CustomBuffer<T>& lhs, const CustomBuffer<T>& rhs)
{
    if(lhs.count() != rhs.count())
    {
        return true;
    }

    for(size_t i = 0; i < lhs.count(); ++i)
    {
        if(lhs.get()[i] != rhs.get()[i])
        {
            return true;
        }
    }
    return false;
}

template<typename T> bool
operator==(const CustomBuffer<T>& lhs, const CustomBuffer<T>& rhs)
{
    return !operator!=(lhs, rhs);
}

template<typename T> bool
operator<(const CustomBuffer<T>& lhs, const CustomBuffer<T>& rhs)
{
    if(lhs.count() < rhs.count())
    {
        return true;
    }
    else if(lhs.count() > rhs.count())
    {
        return false;
    }

    for(size_t i = 0; i < lhs.count(); ++i)
    {
        if(lhs.get()[i] >= rhs.get()[i])
        {
            return false;
        }
    }
    return true;
}

}

namespace Ice
{

template<typename T>
struct StreamableTraits< ::Test::CustomBuffer<T> >
{
    static const StreamHelperCategory helper = StreamHelperCategorySequence;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};

template<typename T>
struct StreamHelper< ::Test::CustomBuffer<T>, StreamHelperCategorySequence>
{
    template<class S> static inline void
    write(S* stream, const ::Test::CustomBuffer<T>& v)
    {
        stream->write(v.get(), v.get() + v.count());
    }

    template<class S> static inline void
    read(S* stream, ::Test::CustomBuffer<T>& v)
    {
#ifdef ICE_CPP11_MAPPING
        std::pair<const T*, const T*> a;
        stream->read(a);
        size_t count = a.second - a.first;
        if(count > 0)
        {
            auto b = new T[count];
            for(size_t i = 0; i < count; ++i)
            {
                b[i] = a.first[i];
            }
            v.set(b, count);
        }
        else
        {
            v.set(0, 0);
        }
#else
        IceUtil::ScopedArray<T> p;
        std::pair<const T*, const T*> a;
        stream->read(a, p);
        T* b = p.release();
        size_t count = static_cast<size_t>(a.second - a.first);
        if(b == 0 && count > 0)
        {
            b = new T[count];
            for(size_t i = 0; i < count; ++i)
            {
                b[i] = a.first[i];
            }
        }
        v.set(b, count);
#endif
    }
};

}

#endif
