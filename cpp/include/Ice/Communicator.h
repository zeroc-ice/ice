// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_H
#define ICE_COMMUNICATOR_H

#include <Ice/Shared.h>
#include <Ice/Handle.h>

namespace Ice
{

class CommunicatorI;

void ICE_API _incRef(CommunicatorI*);
void ICE_API _decRef(CommunicatorI*);

typedef Handle<CommunicatorI> Communicator;

class ICE_API CommunicatorI : public SimpleShared
{
public:

private:

    CommunicatorI(const CommunicatorI&);
    void operator=(const CommunicatorI&);

    CommunicatorI();
    virtual ~CommunicatorI();
    friend Communicator initialize(int&, char*[]); // May create Communicators
};

Communicator initialize(int&, char*[]);

}

#endif
