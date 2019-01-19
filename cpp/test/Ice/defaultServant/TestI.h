//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyObjectI : public virtual Test::MyObject
{
public:

    virtual void ice_ping(const Ice::Current&) const;
    virtual std::string getName(const Ice::Current&);
};

#endif
