// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Glacier/ServerBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::ServerBlobject::ServerBlobject(const ObjectAdapterPtr& clientAdapter) :
    Glacier::Blobject(clientAdapter->getCommunicator()),
    _clientAdapter(clientAdapter)
{
    PropertiesPtr properties = _communicator->getProperties();
    _traceLevel = properties->getPropertyAsInt("Glacier.Router.Trace.Server");
    _forwardContext = properties->getPropertyAsInt("Glacier.Router.Server.ForwardContext") > 0;
    _batchSleepTime = IceUtil::Time::milliSeconds(
	properties->getPropertyAsIntWithDefault("Glacier.Router.Server.BatchSleepTime", 250));
}

bool
Glacier::ServerBlobject::reverse()
{
    return true;
}

void
Glacier::ServerBlobject::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _clientAdapter = 0;
    Blobject::destroy();
}

bool
Glacier::ServerBlobject::ice_invoke(const vector<Byte>& inParams, vector<Byte>& outParams, const Current& current)
{
    assert(_clientAdapter); // Destroyed?

    ObjectPrx proxy = _clientAdapter->createReverseProxy(current.id);
    assert(proxy);

    return invoke(proxy, inParams, outParams, current);
}
