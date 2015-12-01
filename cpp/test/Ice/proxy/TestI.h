// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyDerivedClassI : 
#ifdef ICE_CPP11_MAPPING
    public Test::MyDerivedClassDisp
#else
    public Test::MyDerivedClass
#endif
{
public:

    MyDerivedClassI();

    virtual Ice::ObjectPrxPtr echo(const Ice::ObjectPrxPtr&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
    virtual Ice::Context getContext(const Ice::Current&);
    virtual bool ice_isA(const std::string&, const Ice::Current&) const;

private:

    mutable Ice::Context _ctx;
};

#endif
