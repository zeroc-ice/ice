//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public Test::TestIntf
{
public:

    virtual void shutdown(const Ice::Current&);

    virtual Ice::Context getEndpointInfoAsContext(const Ice::Current&);
    virtual Ice::Context getConnectionInfoAsContext(const Ice::Current&);
};

#endif
