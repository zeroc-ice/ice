// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyObjectI : virtual public Test::MyObject
{
public:

    virtual void ice_ping(const Ice::Current&) const;
    virtual std::string getName(const Ice::Current&);
};

#endif
