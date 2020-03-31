//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <IceUtil/IceUtil.h>
#include <Test.h>

class TestIntfControllerI;
ICE_DEFINE_PTR(TestIntfControllerIPtr, TestIntfControllerI);

class TestIntfI : public virtual Test::TestIntf
{
public:

    virtual void op(const Ice::Current&);
    virtual void sleep(Ice::Int, const Ice::Current&);
    virtual void opWithPayload(Ice::ByteSeq, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

class TestIntfControllerI : public Test::TestIntfController, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    virtual void holdAdapter(const Ice::Current&);
    virtual void resumeAdapter(const Ice::Current&);

    TestIntfControllerI(const Ice::ObjectAdapterPtr&);

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
