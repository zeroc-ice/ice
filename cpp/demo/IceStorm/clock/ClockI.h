// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef CLOCKI_H
#define CLOCKI_H

#include <Clock.h>

class ClockI : public Clock
{
public:

    ClockI() { }
    ~ClockI() { }

    virtual void
    tick();
};

#endif
