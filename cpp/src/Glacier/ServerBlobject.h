// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
				  const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _clientAdapter;
};

}

#endif
