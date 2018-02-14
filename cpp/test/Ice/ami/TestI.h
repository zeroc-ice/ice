// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestIntfControllerI;
typedef IceUtil::Handle<TestIntfControllerI> TestIntfControllerIPtr;

class TestIntfI : virtual public Test::TestIntf, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    TestIntfI();

    virtual void op(const Ice::Current&);
    virtual int opWithResult(const Ice::Current&);
    virtual void opWithUE(const Ice::Current&);
    virtual void opWithPayload(const Ice::ByteSeq&, const Ice::Current&);
    virtual void opBatch(const Ice::Current&);
    virtual Ice::Int opBatchCount(const Ice::Current&);
    virtual void opWithArgs(Ice::Int&, Ice::Int&, Ice::Int&, Ice::Int&, Ice::Int&, Ice::Int&, Ice::Int&,
                            Ice::Int&, Ice::Int&, Ice::Int&, Ice::Int&, const Ice::Current&);
    virtual bool waitForBatch(Ice::Int, const Ice::Current&);
    virtual void close(bool, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    virtual bool supportsFunctionalTests(const Ice::Current&);

private:

    int _batchCount;
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
