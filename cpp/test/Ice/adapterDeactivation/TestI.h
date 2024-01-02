//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public Test::TestIntf
{
public:

    virtual void transient(const Ice::Current&);
    virtual void deactivate(const Ice::Current&);
};

#ifdef ICE_CPP11_MAPPING
class Cookie
#else
class Cookie : public Ice::LocalObject
#endif
{
public:

    std::string message() const;
};

#ifndef ICE_CPP11_MAPPING
typedef IceUtil::Handle<Cookie> CookiePtr;
#endif

#endif
