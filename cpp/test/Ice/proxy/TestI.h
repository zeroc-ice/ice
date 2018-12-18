// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyDerivedClassI : public Test::MyDerivedClass
{
public:

    MyDerivedClassI();
#ifdef ICE_CPP11_MAPPING
    virtual std::shared_ptr<Ice::ObjectPrx> echo(std::shared_ptr<Ice::ObjectPrx>, const Ice::Current&);
    virtual bool ice_isA(std::string, const Ice::Current&) const;
#else
    virtual Ice::ObjectPrx echo(const Ice::ObjectPrx&, const Ice::Current&);
    virtual bool ice_isA(const std::string&, const Ice::Current&) const;
#endif
    virtual void shutdown(const Ice::Current&);
    virtual Ice::Context getContext(const Ice::Current&);

private:

    mutable Ice::Context _ctx;
};

#endif
