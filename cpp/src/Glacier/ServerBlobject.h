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

#include <Glacier/Blobject.h>

namespace Glacier
{

class ServerBlobject : public Glacier::Blobject
{
public:

    ServerBlobject(const Ice::ObjectAdapterPtr&);

    void destroy();
    virtual bool ice_invoke(const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _clientAdapter;
    int _traceLevel;
};

}

#endif
