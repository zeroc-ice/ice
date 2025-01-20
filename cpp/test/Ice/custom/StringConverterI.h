// Copyright (c) ZeroC, Inc.

#ifndef STRING_CONVERTER_I_H
#define STRING_CONVERTER_I_H

#include "Ice/StringConverter.h"

//
// Simple contrived string converters which simply reverse the order of the
// characters being sent.
//

namespace Test
{
    class StringConverterI final : public Ice::StringConverter
    {
    public:
        std::byte* toUTF8(const char*, const char*, Ice::UTF8Buffer&) const final;
        void fromUTF8(const std::byte* sourceStart, const std::byte* sourceEnd, std::string& target) const final;
    };

    class WstringConverterI final : public Ice::WstringConverter
    {
    public:
        std::byte* toUTF8(const wchar_t*, const wchar_t*, Ice::UTF8Buffer&) const final;
        void fromUTF8(const std::byte* sourceStart, const std::byte* sourceEnd, std::wstring& target) const final;
    };
}

#endif
