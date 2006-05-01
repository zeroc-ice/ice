// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <StringConverterI.h>
#include <IceUtil/Unicode.h>

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
				 const char*& targetStart, const char*& targetEnd) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    char* buf = new char[size];

    Ice::Byte* p = const_cast<Ice::Byte*>(sourceEnd);
    for(unsigned int i = 0; i < size; ++i)
    {
        buf[i] = *(--p);
    }

    targetStart = buf;
    targetEnd = targetStart + size;
}

void 
Test::StringConverterI::freeTarget(const char* target) const
{
    delete[] target;
}

Ice::Byte*
Test::WstringConverterI::toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd, Ice::UTF8Buffer& buffer) const
{
    std::wstring ws(sourceStart, sourceEnd);
    std::string s = IceUtil::wstringToString(ws);

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
				  const wchar_t*& targetStart, const wchar_t*& targetEnd) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    std::string s(sourceStart, sourceEnd);

    Ice::Byte* p = const_cast<Ice::Byte*>(sourceEnd);
    for(unsigned int i = 0; i < size; ++i)
    {
        s[i] = *(--p);
    }

    std::wstring ws = IceUtil::stringToWstring(s);
    size = ws.size();
    wchar_t* buf = new wchar_t[size];
    for(unsigned int i = 0; i < size; ++i)
    {
        buf[i] = ws[i];
    }

    targetStart = buf;
    targetEnd = targetStart + size;
}

void 
Test::WstringConverterI::freeTarget(const wchar_t* target) const
{
#if defined(_MSC_VER) && _MSC_VER < 1300
    delete[] const_cast<wchar_t*>(target);
#else
    delete[] target;
#endif
}
