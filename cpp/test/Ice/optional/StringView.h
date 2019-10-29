//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <Ice/Ice.h>

//
// COMPILERFIX: G++ false positive "maybe-uninitialized" warnings when using
// string_view with Ice::optional in C++17 mode.
//
#if defined(__GNUC__) && !defined(__clang__) && ICE_CPLUSPLUS >= 201703L
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

namespace Util
{

//
// A simplified version of boost::string_ref and the new
// std::experimental::fundamentals_v1::string_view
//

class string_view
{
public:

    string_view() :
        _data(0),
        _size(0)
    {}

    string_view(const string_view& sv) :
        _data(sv._data),
        _size(sv._size)
    {}

    string_view(const std::string& s) :
        _data(s.data()),
        _size(s.length())
    {}

    string_view(const char* str) :
        _data(str),
        _size(strlen(str))
    {
    }

    string_view(const char* str, size_t len) :
        _data(str),
        _size(len)
    {}

    string_view& operator=(const string_view& sv)
    {
        _data = sv._data;
        _size = sv._size;
        return *this;
    }

    size_t size() const
    {
        return _size;
    }

    size_t length() const
    {
        return _size;
    }

    bool empty() const
    {
        return _size != 0;
    }

    const char* data() const
    {
        return _data;
    }

    void clear()
    {
        _data = 0;
        _size = 0;
    }

    std::string to_string() const
    {
        return std::string(_data, _size);
    }

    int compare(string_view str) const
    {
        if(_size == str._size)
        {
            if(_data == str._data)
            {
                return 0;
            }
            else
            {
                return strncmp(_data, str._data, _size);
            }
        }
        else if(_size < str._size)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }

private:
    const char* _data;
    size_t _size;
};

inline bool
operator==(string_view lhs, string_view rhs)
{
   return lhs.compare(rhs) == 0;
}

inline bool
operator!=(string_view lhs, string_view rhs)
{
    return lhs.compare(rhs) != 0;
}

}

namespace Ice
{

//
// Describes how to marshal/unmarshal a Util::string_view
// It would be the same for a string_ref or std...::string_view
//

template<>
struct StreamableTraits<Util::string_view>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};

template<>
struct StreamHelper<Util::string_view, StreamHelperCategoryBuiltin>
{
    template<class S> static inline void
    write(S* stream, const Util::string_view& v)
    {
#ifdef ICE_CPP11_MAPPING
        stream->write(v.data(), v.size());
#else
        //
        // In C++98, for consistency with the read, we don't string-convert
        //
        stream->write(v.data(), v.size(), false);
#endif
    }

    template<class S> static inline void
    read(S* stream, Util::string_view& v)
    {
        const char* vdata = 0;
        size_t vsize = 0;

        //
        // In C++98, we ignore the string converter
        //
        stream->read(vdata, vsize);

        if(vsize > 0)
        {
            v = Util::string_view(vdata, vsize);
        }
        else
        {
            v.clear();
        }
    }
};

}

#if defined(__GNUC__) && !defined(__clang__) && ICE_CPLUSPLUS >= 201703L
#   pragma GCC diagnostic pop
#endif

#endif
