//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    void sleep(std::int32_t, const Ice::Current&) final;

    void sendPayload(Ice::ByteSeq, const Ice::Current&) final;

    void shutdown(const Ice::Current&) final;
};

class TestIntfControllerI final : public Test::TestIntfController
{
public:
    void holdAdapter(const Ice::Current&) final;

    void resumeAdapter(const Ice::Current&) final;

    TestIntfControllerI(const Ice::ObjectAdapterPtr&);

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
