// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <StringConverterI.h>

using namespace std;
using namespace IceUtil;

Byte*
Test::StringConverterI::toUTF8(const char* sourceStart, const char* sourceEnd, IceUtil::UTF8Buffer& buffer) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    Byte* targetStart = buffer.getMoreBytes(size, 0);
    Byte* targetEnd = targetStart + size;

    size_t j = size;
    for(size_t i = 0; i < size; ++i)
    {
        targetStart[i] = sourceStart[--j];
    }

    return targetEnd;
}

void
Test::StringConverterI::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd, 
                                 string& target) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    target.resize(size);

    size_t j = size;
    for(size_t i = 0; i < size; ++i)
    {
        target[i] = sourceStart[--j];
    }
}


Byte*
Test::WstringConverterI::toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd, IceUtil::UTF8Buffer& buffer) const
{
    wstring ws(sourceStart, sourceEnd);
    string s = IceUtil::wstringToString(ws);

    size_t size = s.size();
    Byte* targetStart = buffer.getMoreBytes(size, 0);
    Byte* targetEnd = targetStart + size;

    size_t j = size;
    for(size_t i = 0; i < size; ++i)
    {
        targetStart[i] = static_cast<Byte>(s[--j]);
    }
    return targetEnd;
}

void
Test::WstringConverterI::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd, 
                                  wstring& target) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    string s;
    s.resize(size);

    size_t j = size;
    for(size_t i = 0; i < size; ++i)
    {
        s[i] = sourceStart[--j];
    }

    target = IceUtil::stringToWstring(s);
}

