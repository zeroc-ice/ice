//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyDerivedClassI : public Test::MyDerivedClass
{
public:

    MyDerivedClassI();
    virtual std::shared_ptr<Ice::ObjectPrx> echo(std::shared_ptr<Ice::ObjectPrx>, const Ice::Current&);
    virtual bool ice_isA(std::string, const Ice::Current&) const;
    virtual void shutdown(const Ice::Current&);
    virtual Ice::Context getContext(const Ice::Current&);

private:

    mutable Ice::Context _ctx;
};

#endif
