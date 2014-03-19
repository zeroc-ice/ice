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
#include <IceWS/InstanceF.h>

namespace IceWS
{

class EndpointI;

class AcceptorI : public IceInternal::Acceptor, public IceInternal::NativeInfo
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();
#ifdef ICE_USE_IOCP
    virtual IceInternal::AsyncInfo* getAsyncInfo(IceInternal::SocketOperation);
#endif

    virtual void close();
    virtual void listen();
#ifdef ICE_USE_IOCP
    virtual void startAccept();
    virtual void finishAccept();
#endif
    virtual IceInternal::TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;

private:

    AcceptorI(const InstancePtr&, const IceInternal::AcceptorPtr&);
    virtual ~AcceptorI();
    friend class EndpointI;

    const InstancePtr _instance;
    const IceInternal::AcceptorPtr _delegate;
};

}

#endif
