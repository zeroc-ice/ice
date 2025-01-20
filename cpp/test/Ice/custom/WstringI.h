// Copyright (c) ZeroC, Inc.

#ifndef WSTRING_I_H
#define WSTRING_I_H

#include "Wstring.h"

namespace Test1
{
    class WstringClassI : public virtual WstringClass
    {
    public:
        std::wstring opString(std::wstring, std::wstring&, const Ice::Current&) override;

        ::Test1::WstringStruct opStruct(::Test1::WstringStruct, ::Test1::WstringStruct&, const Ice::Current&) override;

        void throwExcept(std::wstring, const Ice::Current&) override;
    };
}

namespace Test2
{
    class WstringClassI : public virtual WstringClass
    {
    public:
        std::wstring opString(std::wstring, std::wstring&, const Ice::Current&) override;

        ::Test2::WstringStruct opStruct(::Test2::WstringStruct, ::Test2::WstringStruct&, const Ice::Current&) override;

        void throwExcept(std::wstring, const Ice::Current&) override;
    };
}

#endif
