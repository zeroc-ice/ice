// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef WSTRING_I_H
#define WSTRING_I_H

#include <Wstring.h>

namespace Test1
{

class WstringClassI : public virtual WstringClass
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

class WstringClassI : public virtual WstringClass
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
