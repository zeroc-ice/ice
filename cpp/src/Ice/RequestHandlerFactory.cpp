//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/RequestHandlerFactory.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/ConnectRequestHandler.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Instance.h>

using namespace std;
using namespace IceInternal;

RequestHandlerFactory::RequestHandlerFactory(const InstancePtr& instance) : _instance(instance)
{
}

RequestHandlerPtr
IceInternal::RequestHandlerFactory::getRequestHandler(const RoutableReferencePtr& ref)
{
    if(ref->getCollocationOptimized())
    {
        Ice::ObjectAdapterPtr adapter = _instance->objectAdapterFactory()->findObjectAdapter(ref);
        if(adapter)
        {
            return make_shared<CollocatedRequestHandler>(ref, adapter);
        }
    }

    ConnectRequestHandlerPtr handler;
    bool connect = false;
    if(ref->getCacheConnection())
    {
        lock_guard lock(_mutex);
        auto p = _handlers.find(ref);
        if(p == _handlers.end())
        {
            handler = make_shared<ConnectRequestHandler>(ref);
            _handlers.insert(make_pair(ref, handler));
            connect = true;
        }
        else
        {
            handler = p->second;
        }
    }
    else
    {
        handler = make_shared<ConnectRequestHandler>(ref);
        connect = true;
    }
    if(connect)
    {
        ref->getConnection(handler);
    }
    return handler->connect();
}

void
IceInternal::RequestHandlerFactory::removeRequestHandler(const ReferencePtr& ref, const RequestHandlerPtr& handler)
{
    if(ref->getCacheConnection())
    {
        lock_guard lock(_mutex);
        auto p = _handlers.find(ref);
        if(p != _handlers.end() && p->second.get() == handler.get())
        {
            _handlers.erase(p);
        }
    }
}
