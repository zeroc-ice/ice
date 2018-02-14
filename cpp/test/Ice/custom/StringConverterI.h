// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef STRING_CONVERTER_I_H
#define STRING_CONVERTER_I_H

#include <IceUtil/StringConverter.h>

//
// Simple contrived string converters which simply reverse the order of the
// characters being sent.
//

namespace Test
{

class StringConverterI : public IceUtil::StringConverter
{
public:

    virtual IceUtil::Byte* toUTF8(const char*, const char*, IceUtil::UTF8Buffer&) const;
    virtual void fromUTF8(const IceUtil::Byte* sourceStart, const IceUtil::Byte* sourceEnd,
                          std::string& target) const;
};

class WstringConverterI : public IceUtil::WstringConverter
{
public:

    virtual IceUtil::Byte* toUTF8(const wchar_t*, const wchar_t*, IceUtil::UTF8Buffer&) const;
    virtual void fromUTF8(const IceUtil::Byte* sourceStart, const IceUtil::Byte* sourceEnd,
                          std::wstring& target) const;
    
};

}

#endif
