// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_WS_ACCEPTOR_I_H
#define ICE_WS_ACCEPTOR_I_H

#include <Ice/LoggerF.h>
#include <Ice/TransceiverF.h>
#include <Ice/Acceptor.h>
#include <Ice/Network.h>
#include <Ice/ProtocolInstance.h>

namespace IceInternal
{

class WSEndpoint;

class WSAcceptor : public Acceptor, public NativeInfo
{
public:

    virtual NativeInfoPtr getNativeInfo();
#if defined(ICE_USE_IOCP)
    virtual AsyncInfo* getAsyncInfo(SocketOperation);
#elif defined(ICE_OS_WINRT)
    virtual void setCompletedHandler(SocketOperationCompletedHandler^);
#endif

    virtual void close();
    virtual EndpointIPtr listen();
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual void startAccept();
    virtual void finishAccept();
#endif
    virtual TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;

    virtual AcceptorPtr delegate() const { return _delegate; }

private:

    WSAcceptor(const WSEndpointPtr&, const ProtocolInstancePtr&, const AcceptorPtr&);
    virtual ~WSAcceptor();
    friend class WSEndpoint;

    WSEndpointPtr _endpoint;
    const ProtocolInstancePtr _instance;
    const AcceptorPtr _delegate;
};

}

#endif
