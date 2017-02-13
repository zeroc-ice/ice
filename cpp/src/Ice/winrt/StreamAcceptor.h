// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_ACCEPTOR_H
#define ICE_STREAM_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/Acceptor.h>
#include <Ice/Network.h>
#include <Ice/winrt/StreamF.h>

#include <IceUtil/Mutex.h>

#include <deque>

namespace IceInternal
{

class StreamAcceptor : public Acceptor, public NativeInfo
{
public:

    virtual NativeInfoPtr getNativeInfo();
    virtual void setCompletedHandler(SocketOperationCompletedHandler^);

    virtual void close();
    virtual EndpointIPtr listen();

    virtual void startAccept();
    virtual void finishAccept();

    virtual TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;

    int effectivePort() const;

private:

    StreamAcceptor(const StreamEndpointIPtr&, const ProtocolInstancePtr&, const std::string&, int);
    virtual ~StreamAcceptor();
    friend class StreamEndpointI;

    virtual void queueAcceptedSocket(Windows::Networking::Sockets::StreamSocket^);

    StreamEndpointIPtr _endpoint;
    const ProtocolInstancePtr _instance;
    const Address _addr;

    IceUtil::Mutex _mutex;
    bool _acceptPending;
    SocketOperationCompletedHandler^ _completedHandler;
    std::deque<Windows::Networking::Sockets::StreamSocket^> _accepted;
};

}
#endif

