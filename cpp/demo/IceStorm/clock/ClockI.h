// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CLOCK_I_H
#define CLOCK_I_H

#include <Clock.h>

class ClockI : public Demo::Clock
{
public:

    virtual void tick(const Ice::Current&);
};

#endif
