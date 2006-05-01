// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    virtual void fromUTF8(const Ice::Byte*, const Ice::Byte*, const char*&, const char*&) const;
    virtual void freeTarget(const char*) const;
};

class WstringConverterI : public Ice::WstringConverter
{
public:

    virtual Ice::Byte* toUTF8(const wchar_t*, const wchar_t*, Ice::UTF8Buffer&) const;
    virtual void fromUTF8(const Ice::Byte*, const Ice::Byte*, const wchar_t*&, const wchar_t*&) const;
    virtual void freeTarget(const wchar_t*) const;
};

}

#endif
