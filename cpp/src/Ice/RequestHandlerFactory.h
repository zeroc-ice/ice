// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REQUEST_HANDLER_FACTORY_H
#define ICE_REQUEST_HANDLER_FACTORY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>

#include <Ice/RequestHandlerF.h>
#include <Ice/ConnectRequestHandlerF.h>
#include <Ice/ProxyF.h>
#include <Ice/ReferenceF.h>
#include <Ice/InstanceF.h>

namespace IceInternal
{

class RequestHandlerFactory : public IceUtil::Shared, private IceUtil::Mutex
{
public:

    RequestHandlerFactory(const InstancePtr&);

    RequestHandlerPtr getRequestHandler(const RoutableReferencePtr&, const Ice::ObjectPrx&);
    void removeRequestHandler(const ReferencePtr&, const RequestHandlerPtr&);

private:

    const InstancePtr _instance;
    std::map<ReferencePtr, ConnectRequestHandlerPtr> _handlers;
};

}

#endif
