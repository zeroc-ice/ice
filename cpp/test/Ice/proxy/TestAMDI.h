//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_AMD_I_H
#define TEST_AMD_I_H

#include <TestAMD.h>

class MyDerivedClassI : public Test::MyDerivedClass
{
public:

    MyDerivedClassI();

    virtual void echoAsync(
        std::shared_ptr<Ice::ObjectPrx>,
        ::std::function<void(const ::std::shared_ptr<Ice::ObjectPrx>&)>,
        ::std::function<void(::std::exception_ptr)>,
        const Ice::Current&);

    virtual void shutdownAsync(
        ::std::function<void()>,
        ::std::function<void(::std::exception_ptr)>,
        const Ice::Current&);

    virtual void getContextAsync(
        ::std::function<void(const Ice::Context&)>,
        ::std::function<void(::std::exception_ptr)>,
        const Ice::Current&);

    virtual bool ice_isA(std::string, const Ice::Current&) const;
private:

    mutable Ice::Context _ctx;
};

#endif
