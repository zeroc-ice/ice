//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TimeoutI : public virtual Test::Timeout
{
public:

    virtual void op(const Ice::Current&);
    virtual void sendData(Test::ByteSeq, const Ice::Current&);
    virtual void sleep(std::int32_t, const Ice::Current&);
};

class ControllerI : public virtual Test::Controller
{
public:

    ControllerI(const Ice::ObjectAdapterPtr&);

    virtual void holdAdapter(std::int32_t, const Ice::Current&);
    virtual void resumeAdapter(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    const Ice::ObjectAdapterPtr _adapter;
};

#endif
