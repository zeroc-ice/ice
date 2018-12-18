// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICESSL_ACCEPTOR_I_H
#define ICESSL_ACCEPTOR_I_H

#include <Ice/TransceiverF.h>
#include <Ice/Acceptor.h>
#include <IceSSL/InstanceF.h>
#include <Ice/Network.h>

#include <vector>

namespace IceSSL
{

class AcceptorI : public IceInternal::Acceptor, public IceInternal::NativeInfo
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();
#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
    virtual IceInternal::AsyncInfo* getAsyncInfo(IceInternal::SocketOperation);
#endif

    virtual void close();
    virtual IceInternal::EndpointIPtr listen();
#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
    virtual void startAccept();
    virtual void finishAccept();
#endif
    virtual IceInternal::TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;

private:

    AcceptorI(const EndpointIPtr&, const InstancePtr&, const IceInternal::AcceptorPtr&, const std::string&);
    virtual ~AcceptorI();
    friend class EndpointI;

    EndpointIPtr _endpoint;
    const InstancePtr _instance;
    const IceInternal::AcceptorPtr _delegate;
    const std::string _adapterName;
};

}

#endif // IceSSL namespace end
