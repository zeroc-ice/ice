// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UWP_ACCEPTOR_I_H
#define ICE_SSL_UWP_ACCEPTOR_I_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/Acceptor.h>
#include <Ice/Network.h>
#include <IceSSL/uwp/TransceiverF.h>

#include <IceUtil/Mutex.h>

#include <deque>

namespace IceSSL
{

class AcceptorI : public IceInternal::Acceptor,
                  public IceInternal::NativeInfo
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();
    virtual void setCompletedHandler(IceInternal::SocketOperationCompletedHandler^);

    virtual void close();
    virtual IceInternal::EndpointIPtr listen();

    virtual void startAccept();
    virtual void finishAccept();

    virtual IceInternal::TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;

    int effectivePort() const;

private:

    AcceptorI(const EndpointIPtr&, const IceInternal::ProtocolInstancePtr&, const std::string&, int);
    virtual ~AcceptorI();
    friend class EndpointI;

    virtual void queueAcceptedSocket(Windows::Networking::Sockets::StreamSocket^);

    EndpointIPtr _endpoint;
    const IceInternal::ProtocolInstancePtr _instance;
    const IceInternal::Address _addr;

    IceUtil::Mutex _mutex;
    bool _acceptPending;
    IceInternal::SocketOperationCompletedHandler^ _completedHandler;
    std::deque<Windows::Networking::Sockets::StreamSocket^> _accepted;
};

}
#endif
