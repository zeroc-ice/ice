// Copyright (c) ZeroC, Inc.

#ifndef CUSTOM_BUFFER_H
#define CUSTOM_BUFFER_H

#include "Ice/Ice.h"
#include <iostream>

#if defined(_MSC_VER)
#    pragma warning(disable : 4800)
#endif

namespace Test
{
    template<typename T> class CustomBuffer
    {
    public:
        CustomBuffer() = default;

        CustomBuffer(const CustomBuffer& o) : _count(o._count)
        {
            if (_count > 0)
            {
                _buf = new T[_count];
                for (size_t i = 0; i < _count; ++i)
                {
                    _buf[i] = o._buf[i];
                }
            }
        }

        CustomBuffer(CustomBuffer&& o) noexcept : _buf(o._buf), _count(o._count)
        {
            o._buf = nullptr;
            o._count = 0;
        }

        ~CustomBuffer() { delete[] _buf; }

        CustomBuffer& operator=(const CustomBuffer& o)
        {
            if (this != &o)
            {
                _count = o._count;
                if (_count > 0)
                {
                    _buf = new T[_count];
                    for (size_t i = 0; i < _count; ++i)
                    {
                        _buf[i] = o._buf[i];
                    }
                }
            }
            return *this;
        }

        CustomBuffer& operator=(CustomBuffer&& o) noexcept
        {
            if (this != &o)
            {
                delete[] _buf;
                _buf = o._buf;
                _count = o._count;
                o._buf = nullptr;
                o._count = 0;
            }
            return *this;
        }

        [[nodiscard]] size_t count() const { return _count; }

        [[nodiscard]] T* get() const { return _buf; }

        void set(T* buf, size_t count)
        {
            _buf = buf;
            _count = count;
        }

        void setAndInit(T* buf, size_t count)
        {
            _buf = buf;
            _count = count;
            for (size_t i = 0; i < count; ++i)
            {
                _buf[i] = static_cast<T>(rand()); // NOLINT
            }
        }

    private:
        T* _buf{nullptr};
        size_t _count{0};
    };

    template<typename T> bool operator!=(const CustomBuffer<T>& lhs, const CustomBuffer<T>& rhs)
    {
        if (lhs.count() != rhs.count())
        {
            return true;
        }

        for (size_t i = 0; i < lhs.count(); ++i)
        {
            if (lhs.get()[i] != rhs.get()[i])
            {
                return true;
            }
        }
        return false;
    }

    template<typename T> bool operator==(const CustomBuffer<T>& lhs, const CustomBuffer<T>& rhs)
    {
        return !operator!=(lhs, rhs);
    }

    template<typename T> bool operator<(const CustomBuffer<T>& lhs, const CustomBuffer<T>& rhs)
    {
        if (lhs.count() < rhs.count())
        {
            return true;
        }
        else if (lhs.count() > rhs.count())
        {
            return false;
        }

        for (size_t i = 0; i < lhs.count(); ++i)
        {
            if (lhs.get()[i] >= rhs.get()[i])
            {
                return false;
            }
        }
        return true;
    }
}

namespace Ice
{
    template<typename T> struct StreamableTraits<::Test::CustomBuffer<T>>
    {
        static const StreamHelperCategory helper = StreamHelperCategorySequence;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    template<typename T> struct StreamHelper<::Test::CustomBuffer<T>, StreamHelperCategorySequence>
    {
        static void write(Ice::OutputStream* stream, const ::Test::CustomBuffer<T>& v)
        {
            stream->write(v.get(), v.get() + v.count());
        }

        static void read(Ice::InputStream* stream, ::Test::CustomBuffer<T>& v)
        {
            std::pair<const T*, const T*> a;
            stream->read(a);
            auto count = static_cast<size_t>(a.second - a.first);
            if (count > 0)
            {
                auto b = new T[count];
                for (size_t i = 0; i < count; ++i)
                {
                    b[i] = a.first[i];
                }
                v.set(b, count);
            }
            else
            {
                v.set(nullptr, 0);
            }
        }

        static void print(std::ostream& os, const Test::CustomBuffer<T>& v)
        {
            os << "CustomBuffer<T> { count = " << v.count() << " }";
        }
    };
}

#endif
