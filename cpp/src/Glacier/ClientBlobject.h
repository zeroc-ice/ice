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
#include <Glacier/Blobject.h>

namespace Glacier
{

class ClientBlobject : public Glacier::Blobject
{
public:

    ClientBlobject(const Ice::CommunicatorPtr&, const IceInternal::RoutingTablePtr&, const std::string&);

    void destroy();
    virtual bool ice_invoke(const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&);

private:

    int _traceLevel;
    IceInternal::RoutingTablePtr _routingTable;
    std::vector<std::string> _allowCategories;
};

}

#endif
