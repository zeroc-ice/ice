// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/TraceLevels.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(TraceLevels* p) { p->__incRef(); }
void IceInternal::decRef(TraceLevels* p) { p->__decRef(); }

IceInternal::TraceLevels::TraceLevels(const PropertiesPtr& properties) :
    network(0),
    networkCat("Network"),
    protocol(0),
    protocolCat("Protocol"),
    retry(0),
    retryCat("Retry"),
    location(0),
    locationCat("Location")
{
    const string keyBase = "Ice.Trace.";
    const_cast<int&>(network) = properties->getPropertyAsInt(keyBase + networkCat);
    const_cast<int&>(protocol) = properties->getPropertyAsInt(keyBase + protocolCat);
    const_cast<int&>(retry) = properties->getPropertyAsInt(keyBase + retryCat);
    const_cast<int&>(location) = properties->getPropertyAsInt(keyBase + locationCat);
}
