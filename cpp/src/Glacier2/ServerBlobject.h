//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    ServerBlobject(const InstancePtr&, const Ice::ConnectionPtr&);
    virtual ~ServerBlobject();

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);
};

}

#endif
