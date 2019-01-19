//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef BACKEND_I_H
#define BACKEND_I_H

#include <Test.h>

class BackendI : public Test::Backend
{
public:
    virtual void check(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
