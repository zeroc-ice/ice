// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

class WstringClassI : virtual public WstringClass
{
public:

    virtual void opString_async(const ::Test1::AMD_WstringClass_opStringPtr&,
                                const ::std::wstring&,
                                const Ice::Current&);

    virtual void opStruct_async(const ::Test1::AMD_WstringClass_opStructPtr&,
                                const ::Test1::WstringStruct&,
                                const Ice::Current&);

    virtual void throwExcept_async(const ::Test1::AMD_WstringClass_throwExceptPtr&,
                                   const ::std::wstring&,
                                   const Ice::Current&);
};

}

namespace Test2
{

class WstringClassI : virtual public WstringClass
{
public:

    virtual void opString_async(const ::Test2::AMD_WstringClass_opStringPtr&,
                                const ::std::wstring&,
                                const Ice::Current&);

    virtual void opStruct_async(const ::Test2::AMD_WstringClass_opStructPtr&,
                                const ::Test2::WstringStruct&,
                                const Ice::Current&);

    virtual void throwExcept_async(const ::Test2::AMD_WstringClass_throwExceptPtr&,
                                   const ::std::wstring&,
                                   const Ice::Current&);
};

}

#endif
