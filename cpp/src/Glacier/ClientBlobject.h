// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef CLIENT_BLOBJECT_H
#define CLIENT_BLOBJECT_H

#include <Ice/RoutingTableF.h>
#include <Ice/Ice.h>

namespace Glacier
{

class ClientBlobject : public Ice::Blobject
{
public:

    ClientBlobject(const Ice::CommunicatorPtr&, const IceInternal::RoutingTablePtr&);

    void destroy();
    virtual void ice_invoke(const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
    Ice::LoggerPtr _logger;
    int _traceLevel;
    IceInternal::RoutingTablePtr _routingTable;
};

}

#endif
