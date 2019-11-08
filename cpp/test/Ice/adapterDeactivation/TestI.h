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

class Cookie
#ifndef  ICE_CPP11_MAPPING
    : public Ice::LocalObject
#endif // ! ICE_CPP11_MAPPING
{
public:
    virtual ~Cookie();
    virtual std::string message() const;
};

#ifndef  ICE_CPP11_MAPPING
typedef IceUtil::Handle<Cookie> CookiePtr;
#endif // ! ICE_CPP11_MAPPING

#endif
