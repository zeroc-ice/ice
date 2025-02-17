// Copyright (c) ZeroC, Inc.

#include "TraceLevels.h"
#include "Ice/Properties.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::TraceLevels::TraceLevels(const PropertiesPtr& properties)
{
    const string keyBase = "Ice.Trace.";
    const_cast<int&>(dispatch) = properties->getIcePropertyAsInt(keyBase + dispatchCat);
    const_cast<int&>(network) = properties->getIcePropertyAsInt(keyBase + networkCat);
    const_cast<int&>(protocol) = properties->getIcePropertyAsInt(keyBase + protocolCat);
    const_cast<int&>(retry) = properties->getIcePropertyAsInt(keyBase + retryCat);
    const_cast<int&>(location) = properties->getIcePropertyAsInt(keyBase + locationCat);
    const_cast<int&>(slicing) = properties->getIcePropertyAsInt(keyBase + slicingCat);
    const_cast<int&>(threadPool) = properties->getIcePropertyAsInt(keyBase + threadPoolCat);
}
