// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class PriorityI : public Test::Priority
{
public:

    PriorityI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual int getPriority(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
