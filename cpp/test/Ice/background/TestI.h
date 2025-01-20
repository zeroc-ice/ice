// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Configuration.h"
#include "Test.h"

#include <set>

class BackgroundControllerI;
using BackgroundControllerIPtr = std::shared_ptr<BackgroundControllerI>;

class BackgroundI final : public Test::Background
{
public:
    BackgroundI(BackgroundControllerIPtr);

    void op(const Ice::Current&) final;
    void opWithPayload(Ice::ByteSeq, const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

private:
    BackgroundControllerIPtr _controller;
};

class BackgroundControllerI final : public Test::BackgroundController
{
public:
    BackgroundControllerI(Ice::ObjectAdapterPtr, ConfigurationPtr);

    void pauseCall(std::string, const Ice::Current&) final;
    void resumeCall(std::string, const Ice::Current&) final;
    void checkCallPause(const Ice::Current&);

    void holdAdapter(const Ice::Current&) final;
    void resumeAdapter(const Ice::Current&) final;

    void initializeSocketOperation(int, const Ice::Current&) final;
    void initializeException(bool, const Ice::Current&) final;

    void readReady(bool, const Ice::Current&) final;
    void readException(bool, const Ice::Current&) final;

    void writeReady(bool, const Ice::Current&) final;
    void writeException(bool, const Ice::Current&) final;

    void buffered(bool, const Ice::Current&) final;

private:
    Ice::ObjectAdapterPtr _adapter;
    std::set<std::string> _pausedCalls;
    ConfigurationPtr _configuration;
    std::mutex _mutex;
    std::condition_variable _condition;
};

#endif
