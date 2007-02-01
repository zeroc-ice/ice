// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <StringConverterI.h>
#include <IceUtil/Unicode.h>

using namespace std;

Ice::Byte*
Test::StringConverterI::toUTF8(const char* sourceStart, const char* sourceEnd, Ice::UTF8Buffer& buffer) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    Ice::Byte* targetStart = buffer.getMoreBytes(size, 0);
    Ice::Byte* targetEnd = targetStart + size;

    size_t j = size;
    for(size_t i = 0; i < size; ++i)
    {
        targetStart[i] = sourceStart[--j];
    }

    return targetEnd;
}

void
Test::StringConverterI::fromUTF8(const Ice::Byte* sourceStart, const Ice::Byte* sourceEnd, 
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


Ice::Byte*
Test::WstringConverterI::toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd, Ice::UTF8Buffer& buffer) const
{
    wstring ws(sourceStart, sourceEnd);
    string s = IceUtil::wstringToString(ws);

    size_t size = s.size();
    Ice::Byte* targetStart = buffer.getMoreBytes(size, 0);
    Ice::Byte* targetEnd = targetStart + size;

    size_t j = size;
    for(size_t i = 0; i < size; ++i)
    {
        targetStart[i] = static_cast<Ice::Byte>(s[--j]);
    }
    return targetEnd;
}

void
Test::WstringConverterI::fromUTF8(const Ice::Byte* sourceStart, const Ice::Byte* sourceEnd, 
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

