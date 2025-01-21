// Copyright (c) ZeroC, Inc.

#ifndef WSTRINGAMD_I_H
#define WSTRINGAMD_I_H

#include "WstringAMD.h"

namespace Test1
{
    class WstringClassI : public virtual WstringClass
    {
    public:
        void opStringAsync(
            std::wstring,
            std::function<void(std::wstring_view, std::wstring_view)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;

        void opStructAsync(
            ::Test1::WstringStruct,
            std::function<void(const ::Test1::WstringStruct&, const ::Test1::WstringStruct&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;

        void throwExceptAsync(
            std::wstring,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;
    };
}

namespace Test2
{
    class WstringClassI : public virtual WstringClass
    {
    public:
        void opStringAsync(
            std::wstring,
            std::function<void(std::wstring_view, std::wstring_view)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;

        void opStructAsync(
            ::Test2::WstringStruct,
            std::function<void(const ::Test2::WstringStruct&, const ::Test2::WstringStruct&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;

        void throwExceptAsync(
            std::wstring,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;
    };
}

#endif
