// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class HoldI : public Test::Hold, public IceUtil::Mutex, public IceUtil::TimerTask
#ifdef ICE_CPP11_MAPPING
    , public std::enable_shared_from_this<HoldI>
#endif
{
public:
    
    HoldI(const IceUtil::TimerPtr&, const Ice::ObjectAdapterPtr&);

    virtual void putOnHold(Ice::Int, const Ice::Current&);
    virtual void waitForHold(const Ice::Current&);
    virtual Ice::Int set(Ice::Int, Ice::Int, const Ice::Current&);
    virtual void setOneway(Ice::Int, Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    virtual void runTimerTask();
    
private:

    int _last;
    const IceUtil::TimerPtr _timer;
    const Ice::ObjectAdapterPtr _adapter;
};

#endif
