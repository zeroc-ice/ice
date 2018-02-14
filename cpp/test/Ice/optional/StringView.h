// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <Ice/Ice.h>

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

#ifdef STRING_VIEW_IGNORE_STRING_CONVERTER
        stream->write(v.data(), v.size(), false);
#else
        stream->write(v.data(), v.size(), true);
#endif
    }

    template<class S> static inline void 
    read(S* stream, Util::string_view& v)
    {
        const char* vdata = 0;
        size_t vsize = 0;

#ifdef STRING_VIEW_IGNORE_STRING_CONVERTER
        stream->read(vdata, vsize);
#else
        std::string holder;
        stream->read(vdata, vsize, holder);
        
        // If holder is not empty, a string conversion occured, and we can't return a 
        // string_view since it does not hold the memory
        if(!holder.empty())
        {
            throw Ice::MarshalException(__FILE__, __LINE__, "string conversion not supported");
        }
#endif

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

#endif
