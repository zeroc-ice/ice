// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/TraceLevels.h>
#include <IceE/Properties.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(TraceLevels* p) { p->__incRef(); }
void IceEInternal::decRef(TraceLevels* p) { p->__decRef(); }

IceEInternal::TraceLevels::TraceLevels(const PropertiesPtr& properties) :
    network(0),
    networkCat("Network"),
    protocol(0),
    protocolCat("Protocol"),
    retry(0),
    retryCat("Retry"),
    location(0),
    locationCat("Location"),
    slicing(0),
    slicingCat("Slicing")
{
    const string keyBase = "IceE.Trace.";
    const_cast<int&>(network) = properties->getPropertyAsInt(keyBase + networkCat);
    const_cast<int&>(protocol) = properties->getPropertyAsInt(keyBase + protocolCat);
    const_cast<int&>(retry) = properties->getPropertyAsInt(keyBase + retryCat);
    const_cast<int&>(location) = properties->getPropertyAsInt(keyBase + locationCat);
    const_cast<int&>(slicing) = properties->getPropertyAsInt(keyBase + slicingCat);
}
