// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef CLOCK_I_H
#define CLOCK_I_H

#include <Clock.h>

class ClockI : public Clock
{
public:

    virtual void tick(const Ice::Current&);
};

#endif
