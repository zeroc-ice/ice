// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TimeoutI : public virtual Test::Timeout
{
public:

    virtual void op(const Ice::Current&);
    virtual void sendData(ICE_IN(Test::ByteSeq), const Ice::Current&);
    virtual void sleep(Ice::Int, const Ice::Current&);
};

class ControllerI : public virtual Test::Controller
{
public:

    ControllerI(const Ice::ObjectAdapterPtr&);

    virtual void holdAdapter(Ice::Int, const Ice::Current&);
    virtual void resumeAdapter(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    const Ice::ObjectAdapterPtr _adapter;
};

#endif
