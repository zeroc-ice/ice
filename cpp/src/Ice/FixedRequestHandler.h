//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_FIXED_REQUEST_HANDLER_H
#define ICE_FIXED_REQUEST_HANDLER_H

#include "RequestHandler.h"
#include "Ice/ReferenceF.h"

namespace IceInternal
{

// A request handler for a fixed reference.
class FixedRequestHandler final : public RequestHandler
{
public:

    FixedRequestHandler(const ReferencePtr&, const Ice::ConnectionIPtr&, bool);

    AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&) final;

    void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr) final;

    Ice::ConnectionIPtr getConnection() final;
    Ice::ConnectionIPtr waitForConnection() final;

private:

    Ice::ConnectionIPtr _connection;
    bool _compress;
};

}

#endif
