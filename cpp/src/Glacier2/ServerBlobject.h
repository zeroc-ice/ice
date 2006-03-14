// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

    virtual void destroy();

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const Ice::ByteSeq&, const Ice::Current&);

private:

    Ice::ConnectionPtr _connection;
};

}

#endif
