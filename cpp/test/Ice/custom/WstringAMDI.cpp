// Copyright (c) ZeroC, Inc.

#include "WstringAMDI.h"

void
Test1::WstringClassI::opStringAsync(
    std::wstring in,
    std::function<void(std::wstring_view, std::wstring_view)> response,
    std::function<void(std::exception_ptr)>,
    const Ice::Current&)
{
    response(in, in);
}

void
Test1::WstringClassI::opStructAsync(
    Test1::WstringStruct in,
    std::function<void(const Test1::WstringStruct&, const Test1::WstringStruct&)> response,
    std::function<void(std::exception_ptr)>,
    const Ice::Current&)
{
    response(in, in);
}

void
Test1::WstringClassI::throwExceptAsync(
    std::wstring in,
    std::function<void()>,
    std::function<void(std::exception_ptr)> error,
    const Ice::Current&)
{
    Test1::WstringException ex(in);
    error(std::make_exception_ptr(ex));
}

void
Test2::WstringClassI::opStringAsync(
    std::wstring in,
    std::function<void(std::wstring_view, std::wstring_view)> response,
    std::function<void(std::exception_ptr)>,
    const Ice::Current&)
{
    response(in, in);
}

void
Test2::WstringClassI::opStructAsync(
    Test2::WstringStruct in,
    std::function<void(const Test2::WstringStruct&, const Test2::WstringStruct&)> response,
    std::function<void(std::exception_ptr)>,
    const Ice::Current&)
{
    response(in, in);
}

void
Test2::WstringClassI::throwExceptAsync(
    std::wstring in,
    std::function<void()>,
    std::function<void(std::exception_ptr)> error,
    const Ice::Current&)
{
    Test2::WstringException ex(in);
    error(std::make_exception_ptr(ex));
}
