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
{
public:
    virtual ~Cookie();
    virtual std::string message() const;
};

#endif
