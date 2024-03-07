//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        virtual std::uint8_t* toUTF8(const char*, const char*, Ice::UTF8Buffer&) const;
        virtual void
        fromUTF8(const std::uint8_t* sourceStart, const std::uint8_t* sourceEnd, std::string& target) const;
    };

    class WstringConverterI : public Ice::WstringConverter
    {
    public:
        virtual std::uint8_t* toUTF8(const wchar_t*, const wchar_t*, Ice::UTF8Buffer&) const;
        virtual void
        fromUTF8(const std::uint8_t* sourceStart, const std::uint8_t* sourceEnd, std::wstring& target) const;
    };

}

#endif
