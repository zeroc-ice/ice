// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

class WSEndpointI;

class WSAcceptorI : public Acceptor, public NativeInfo
{
public:

    virtual NativeInfoPtr getNativeInfo();
#if defined(ICE_USE_IOCP)
    virtual AsyncInfo* getAsyncInfo(SocketOperation);
#elif defined(ICE_OS_WINRT)
    virtual void setCompletedHandler(SocketOperationCompletedHandler^);
#endif

    virtual void close();
    virtual void listen();
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual void startAccept();
    virtual void finishAccept();
#endif
    virtual TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;

private:

    WSAcceptorI(const ProtocolInstancePtr&, const AcceptorPtr&);
    virtual ~WSAcceptorI();
    friend class WSEndpointI;

    const ProtocolInstancePtr _instance;
    const AcceptorPtr _delegate;
};

}

#endif
