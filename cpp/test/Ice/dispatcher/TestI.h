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

class TestIntfI : virtual public Test::TestIntf
{
public:

    virtual void op(const Ice::Current&);
    virtual void sleep(Ice::Int, const Ice::Current&);
    virtual void opWithPayload(const Ice::ByteSeq&, const Ice::Current&);
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
