// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SERVER_BLOBJECT_H
#define SERVER_BLOBJECT_H

#include <Glacier2/Blobject.h>

namespace Glacier2
{

class ServerBlobject;
typedef IceUtil::Handle<ServerBlobject> ServerBlobjectPtr;

class ServerBlobject : public Glacier2::Blobject
{
public:

    ServerBlobject(const Ice::CommunicatorPtr&, const Ice::ConnectionPtr&);
    virtual ~ServerBlobject();

    virtual void ice_invoke_async(const Ice::AMD_Array_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);

private:

    const Ice::ConnectionPtr _connection;
};

}

#endif
