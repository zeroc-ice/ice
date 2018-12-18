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

#include <Test.h>
#include <Configuration.h>

#include <set>

class BackgroundControllerI;
ICE_DEFINE_PTR(BackgroundControllerIPtr, BackgroundControllerI);

class BackgroundI : public virtual Test::Background
{
public:

    virtual void op(const Ice::Current&);
    virtual void opWithPayload(ICE_IN(Ice::ByteSeq), const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    BackgroundI(const BackgroundControllerIPtr&);

private:

    BackgroundControllerIPtr _controller;
};

class BackgroundControllerI : public Test::BackgroundController, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    virtual void pauseCall(ICE_IN(std::string), const Ice::Current&);
    virtual void resumeCall(ICE_IN(std::string), const Ice::Current&);
    virtual void checkCallPause(const Ice::Current&);

    virtual void holdAdapter(const Ice::Current&);
    virtual void resumeAdapter(const Ice::Current&);

    virtual void initializeSocketOperation(int, const Ice::Current&);
    virtual void initializeException(bool, const Ice::Current&);

    virtual void readReady(bool, const Ice::Current&);
    virtual void readException(bool, const Ice::Current&);

    virtual void writeReady(bool, const Ice::Current&);
    virtual void writeException(bool, const Ice::Current&);

    virtual void buffered(bool, const Ice::Current&);

    BackgroundControllerI(const Ice::ObjectAdapterPtr&, const ConfigurationPtr&);

private:

    Ice::ObjectAdapterPtr _adapter;
    std::set<std::string> _pausedCalls;
    ConfigurationPtr _configuration;
};

#endif
