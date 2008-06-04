// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyDerivedClassI : public Test::MyDerivedClass
{
public:

    MyDerivedClassI();

    virtual void shutdown(const Ice::Current&);
    virtual Ice::Context getContext(const Ice::Current&);
    virtual bool ice_isA(const std::string&, const Ice::Current&) const;

private:

    mutable Ice::Context _ctx;
};

#endif
