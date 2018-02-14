// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_AMD_I_H
#define TEST_AMD_I_H

#include <TestAMD.h>

class MyDerivedClassI : public Test::MyDerivedClass
{
public:

    MyDerivedClassI();
    
    virtual void echo_async(const Test::AMD_MyDerivedClass_echoPtr&, const Ice::ObjectPrx&, const Ice::Current&);
    virtual void shutdown_async(const Test::AMD_MyClass_shutdownPtr&,
                                const Ice::Current&);
    virtual void getContext_async(const Test::AMD_MyClass_getContextPtr& cb,
                                  const Ice::Current&);
    virtual bool ice_isA(const std::string&, const Ice::Current&) const;

private:

    mutable Ice::Context _ctx;
};

#endif
