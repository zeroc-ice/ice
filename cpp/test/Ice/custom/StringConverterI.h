// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef STRING_CONVERTER_I_H
#define STRING_CONVERTER_I_H

#include <Ice/StringConverter.h>

//
// Simple contrived string converters which simply reverse the order of the
// characters being sent.
//

namespace Test
{

class StringConverterI : public Ice::StringConverter
{
public:

    virtual Ice::Byte* toUTF8(const char*, const char*, Ice::UTF8Buffer&) const;
    virtual void fromUTF8(const Ice::Byte* sourceStart, const Ice::Byte* sourceEnd,
                          std::string& target) const;
};

class WstringConverterI : public Ice::WstringConverter
{
public:

    virtual Ice::Byte* toUTF8(const wchar_t*, const wchar_t*, Ice::UTF8Buffer&) const;
    virtual void fromUTF8(const Ice::Byte* sourceStart, const Ice::Byte* sourceEnd,
                          std::wstring& target) const;
    
};

}

#endif
