// **********************************************************************
//
// Copyright (c) 2003
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

    void destroy();
    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
				  const Ice::Current&);

private:

    IceInternal::RoutingTablePtr _routingTable;
    std::vector<std::string> _allowCategories;
};

}

#endif
