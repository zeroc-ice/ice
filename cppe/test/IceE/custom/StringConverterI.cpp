// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <StringConverterI.h>
#include <IceE/Unicode.h>

using namespace std;

Ice::Byte*
Test::StringConverterI::toUTF8(const char* sourceStart, const char* sourceEnd, Ice::UTF8Buffer& buffer) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    Ice::Byte* targetStart = buffer.getMoreBytes(size, 0);
    Ice::Byte* targetEnd = targetStart + size;

    char* p = const_cast<char*>(sourceEnd);
    for(unsigned int i = 0; i < size; ++i)
    {
        targetStart[i] = *(--p);
    }

    return targetEnd;
}

void
Test::StringConverterI::fromUTF8(const Ice::Byte* sourceStart, const Ice::Byte* sourceEnd, 
				 string& target) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    target.resize(size);

    Ice::Byte* p = const_cast<Ice::Byte*>(sourceEnd);
    for(unsigned int i = 0; i < size; ++i)
    {
        target[i] = *(--p);
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

    char* p = const_cast<char*>(s.c_str() + size);
    for(unsigned int i = 0; i < size; ++i)
    {
        targetStart[i] = static_cast<Ice::Byte>(*(--p));
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

    Ice::Byte* p = const_cast<Ice::Byte*>(sourceEnd);
    for(unsigned int i = 0; i < size; ++i)
    {
        s[i] = *(--p);
    }

    target = IceUtil::stringToWstring(s);
}

