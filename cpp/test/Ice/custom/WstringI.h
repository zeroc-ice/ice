// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef WSTRING_I_H
#define WSTRING_I_H

#include <Wstring.h>

namespace Test1
{

#ifdef ICE_CPP11_MAPPING
class WstringClassI : public virtual WstringClassDisp
#else
class WstringClassI : public virtual WstringClass
#endif
{
public:

    virtual ::std::wstring opString(ICE_IN(::std::wstring),
                                    ::std::wstring&,
                                    const Ice::Current&);

    virtual ::Test1::WstringStruct opStruct(ICE_IN(::Test1::WstringStruct),
                                            ::Test1::WstringStruct&,
                                            const Ice::Current&);

    virtual void throwExcept(ICE_IN(::std::wstring),
                             const Ice::Current&);
};

}

namespace Test2
{

#ifdef ICE_CPP11_MAPPING
class WstringClassI : public virtual WstringClassDisp
#else
class WstringClassI : public virtual WstringClass
#endif
{
public:

    virtual ::std::wstring opString(ICE_IN(::std::wstring),
                                    ::std::wstring&,
                                    const Ice::Current&);

    virtual ::Test2::WstringStruct opStruct(ICE_IN(::Test2::WstringStruct),
                                            ::Test2::WstringStruct&,
                                            const Ice::Current&);

    virtual void throwExcept(ICE_IN(::std::wstring),
                             const Ice::Current&);
};

}

#endif
