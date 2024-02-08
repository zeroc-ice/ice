//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REQUEST_HANDLER_FACTORY_H
#define ICE_REQUEST_HANDLER_FACTORY_H

#include <IceUtil/Shared.h>

#include <Ice/RequestHandlerF.h>
#include <Ice/ConnectRequestHandlerF.h>
#include <Ice/ProxyF.h>
#include <Ice/Reference.h>
#include <Ice/InstanceF.h>
#include <Ice/Comparable.h>

namespace IceInternal
{

class RequestHandlerFactory : public IceUtil::Shared
{
public:

    RequestHandlerFactory(const InstancePtr&);

    RequestHandlerPtr getRequestHandler(const RoutableReferencePtr&);
    void removeRequestHandler(const ReferencePtr&, const RequestHandlerPtr&);

private:

    const InstancePtr _instance;
    std::map<ReferencePtr, ConnectRequestHandlerPtr, Ice::TargetCompare<ReferencePtr, std::less>> _handlers;
    std::mutex _mutex;
};

}

#endif
