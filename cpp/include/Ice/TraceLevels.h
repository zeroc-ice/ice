// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TRACE_LEVELS_H
#define ICE_TRACE_LEVELS_H

#include <Ice/TraceLevelsF.h>
#include <Ice/Shared.h>

namespace _Ice
{

class ICE_API TraceLevelsI : virtual public Shared
{
public:

    TraceLevelsI(int);
    virtual ~TraceLevelsI();

    const int network;
 
private:

    TraceLevelsI(const TraceLevelsI&);
    void operator=(const TraceLevelsI&);
};

}

#endif
