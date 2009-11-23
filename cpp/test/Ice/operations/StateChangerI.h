// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef STATECHANGER_I_H
#define STATECHANGER_I_H

#include <StateChanger.h>

class StateChangerI : public Test::StateChanger, public IceUtil::TimerTask
{
public:

    StateChangerI(const IceUtil::TimerPtr&, const Ice::ObjectAdapterPtr&);

    virtual void hold(Ice::Int, const Ice::Current&);
    virtual void activate(Ice::Int, const Ice::Current&);

    virtual void runTimerTask();

private:

    enum Action { Hold, Activate };

    Action _action;
    const IceUtil::TimerPtr _timer;
    Ice::ObjectAdapterPtr _otherAdapter;
};

#endif
