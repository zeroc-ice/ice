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
#include <Ice/PropertiesF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class TraceLevels : public Shared
{
public:

    TraceLevels(const ::Ice::Properties_ptr&);
    virtual ~TraceLevels();

    const int network;
    const char* networkCat;
    const int protocol;
    const char* protocolCat;
};

}

#endif
