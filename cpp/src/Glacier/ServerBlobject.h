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

#ifndef SERVER_BLOBJECT_H
#define SERVER_BLOBJECT_H

#include <Glacier/Blobject.h>

namespace Glacier
{

class ServerBlobject : public Glacier::Blobject
{
public:

    ServerBlobject(const Ice::ObjectAdapterPtr&);

    virtual bool reverse();

    void destroy();
    virtual bool ice_invoke(const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _clientAdapter;
};

}

#endif
