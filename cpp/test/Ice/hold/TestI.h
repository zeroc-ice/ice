//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class HoldI final : public Test::Hold
{
public:

    HoldI(const IceUtil::TimerPtr&, const Ice::ObjectAdapterPtr&);

    void putOnHold(std::int32_t, const Ice::Current&) final;
    void waitForHold(const Ice::Current&) final;
    std::int32_t set(std::int32_t, std::int32_t, const Ice::Current&) final;
    void setOneway(std::int32_t, std::int32_t, const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

private:

    int _last;
    const IceUtil::TimerPtr _timer;
    const Ice::ObjectAdapterPtr _adapter;
    std::mutex _mutex;
};

#endif
