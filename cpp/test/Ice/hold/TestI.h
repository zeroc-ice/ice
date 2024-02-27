//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class HoldI : public Test::Hold
{
public:

    HoldI(const IceUtil::TimerPtr&, const Ice::ObjectAdapterPtr&);

    virtual void putOnHold(std::int32_t, const Ice::Current&);
    virtual void waitForHold(const Ice::Current&);
    virtual std::int32_t set(std::int32_t, std::int32_t, const Ice::Current&);
    virtual void setOneway(std::int32_t, std::int32_t, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    int _last;
    const IceUtil::TimerPtr _timer;
    const Ice::ObjectAdapterPtr _adapter;
    std::mutex _mutex;
};

#endif
