//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTION_REQUEST_HANDLER_H
#define ICE_CONNECTION_REQUEST_HANDLER_H

#include <Ice/RequestHandler.h>
#include <Ice/ReferenceF.h>
#include <Ice/ProxyF.h>

namespace IceInternal
{

class FixedRequestHandler final :
    public RequestHandler,
    public std::enable_shared_from_this<FixedRequestHandler>
{
public:

    FixedRequestHandler(const ReferencePtr&, const Ice::ConnectionIPtr&, bool);

    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr);

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

private:

    Ice::ConnectionIPtr _connection;
    bool _compress;
};

}

#endif
