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

    virtual bool reverse();

    void destroy();
    virtual bool ice_invoke(const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&);

private:

    IceInternal::RoutingTablePtr _routingTable;
    std::vector<std::string> _allowCategories;
};

}

#endif
