// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SERVER_BLOBJECT_H
#define SERVER_BLOBJECT_H

#include <Ice/RoutingTableF.h>
#include <Ice/Ice.h>

namespace Glacier
{

class ServerBlobject : public Ice::Blobject
{
public:

    ServerBlobject(const Ice::ObjectAdapterPtr&);
    virtual ~ServerBlobject();

    void destroy();
    virtual bool ice_invoke(const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _clientAdapter;
    Ice::LoggerPtr _logger;
    int _traceLevel;
};

}

#endif
