// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
#ifdef ICE_CPP11_MAPPING
    virtual void opWithPayload(Ice::ByteSeq, const Ice::Current&);
#else
    virtual void opWithPayload(const Ice::ByteSeq&, const Ice::Current&);
#endif
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
