// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef WSTRINGAMD_I_H
#define WSTRINGAMD_I_H

#include <WstringAMD.h>

namespace Test1
{

#ifdef ICE_CPP11_MAPPING
class WstringClassI : public virtual WstringClass
{
public:

    void opStringAsync(std::wstring,
                       std::function<void(const std::wstring&, const std::wstring&)>,
                       std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void opStructAsync(::Test1::WstringStruct,
                       std::function<void(const ::Test1::WstringStruct&, const ::Test1::WstringStruct&)>,
                       std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void throwExceptAsync(std::wstring,
                          std::function<void()>,
                          std::function<void(std::exception_ptr)>, const Ice::Current&) override;
};
#else
class WstringClassI : public virtual WstringClass
{
public:

    virtual void opString_async(const ::Test1::AMD_WstringClass_opStringPtr&,
                                const std::wstring&,
                                const Ice::Current&);

    virtual void opStruct_async(const ::Test1::AMD_WstringClass_opStructPtr&,
                                const ::Test1::WstringStruct&,
                                const Ice::Current&);

    virtual void throwExcept_async(const ::Test1::AMD_WstringClass_throwExceptPtr&,
                                   const std::wstring&,
                                   const Ice::Current&);
};
#endif
}

namespace Test2
{
#ifdef ICE_CPP11_MAPPING
class WstringClassI : public virtual WstringClass
{
public:

    void opStringAsync(std::wstring,
                       std::function<void(const std::wstring&, const std::wstring&)>,
                       std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void opStructAsync(::Test2::WstringStruct,
                       std::function<void(const ::Test2::WstringStruct&, const ::Test2::WstringStruct&)>,
                       std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void throwExceptAsync(std::wstring,
                          std::function<void()>,
                          std::function<void(std::exception_ptr)>, const Ice::Current&) override;
};

#else
class WstringClassI : public virtual WstringClass
{
public:

    virtual void opString_async(const ::Test2::AMD_WstringClass_opStringPtr&,
                                const std::wstring&,
                                const Ice::Current&);

    virtual void opStruct_async(const ::Test2::AMD_WstringClass_opStructPtr&,
                                const ::Test2::WstringStruct&,
                                const Ice::Current&);

    virtual void throwExcept_async(const ::Test2::AMD_WstringClass_throwExceptPtr&,
                                   const std::wstring&,
                                   const Ice::Current&);
};
#endif

}

#endif
