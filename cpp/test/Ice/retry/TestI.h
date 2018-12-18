// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class RetryI : public Test::Retry
{
public:

    RetryI();

    virtual void op(bool, const Ice::Current&);
    virtual int opIdempotent(int, const Ice::Current&);
    virtual void opNotIdempotent(const Ice::Current&);
    virtual void opSystemException(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    int _counter;
};

#endif
