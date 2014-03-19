// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_REQUEST_HANDLER_H
#define ICE_CONNECTION_REQUEST_HANDLER_H

#include <Ice/RequestHandler.h>
#include <Ice/ReferenceF.h>
#include <Ice/ProxyF.h>

namespace IceInternal
{

class ConnectionRequestHandler : public RequestHandler
{
public:

    ConnectionRequestHandler(const ReferencePtr&, const Ice::ObjectPrx&);
    ConnectionRequestHandler(const ReferencePtr&, const Ice::ConnectionIPtr&, bool);

    virtual void prepareBatchRequest(BasicStream*);
    virtual void finishBatchRequest(BasicStream*);
    virtual void abortBatchRequest();

    virtual Ice::ConnectionI* sendRequest(Outgoing*);
    virtual AsyncStatus sendAsyncRequest(const OutgoingAsyncPtr&);

    virtual bool flushBatchRequests(BatchOutgoing*);
    virtual AsyncStatus flushAsyncBatchRequests(const BatchOutgoingAsyncPtr&);

    virtual Ice::ConnectionIPtr getConnection(bool);

private:

    Ice::ConnectionIPtr _connection;
    bool _compress;
};

}

#endif
