//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include "IceUtil/IceUtil.h"
#include "Test.h"

class TestIntfControllerI;
using TestIntfControllerIPtr = std::shared_ptr<TestIntfControllerI>;

class TestIntfI : public virtual Test::TestIntf
{
public:
    virtual void op(const Ice::Current&);
    virtual void sleep(std::int32_t, const Ice::Current&);
    virtual void opWithPayload(Ice::ByteSeq, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

class TestIntfControllerI : public Test::TestIntfController
{
public:
    virtual void holdAdapter(const Ice::Current&);
    virtual void resumeAdapter(const Ice::Current&);

    TestIntfControllerI(const Ice::ObjectAdapterPtr&);

private:
    Ice::ObjectAdapterPtr _adapter;
};

#endif
