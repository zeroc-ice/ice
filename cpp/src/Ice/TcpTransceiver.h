// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TCP_TRANSCEIVER_H
#define ICE_TCP_TRANSCEIVER_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>

namespace IceInternal
{

class TcpConnector;
class TcpAcceptor;

class TcpTransceiver : public Transceiver, public NativeInfo
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateProxyConnectRequest,
        StateProxyConnectRequestPending,
        StateConnected
    };

public:

    virtual NativeInfoPtr getNativeInfo();
#ifdef ICE_USE_IOCP
    virtual AsyncInfo* getAsyncInfo(SocketOperation);
#endif
    
    virtual SocketOperation initialize(Buffer&, Buffer&, bool&);
    virtual SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual SocketOperation write(Buffer&);
    virtual SocketOperation read(Buffer&, bool&);
#ifdef ICE_USE_IOCP
    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&);
#endif
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const Buffer&, size_t);

private:

    TcpTransceiver(const ProtocolInstancePtr&, SOCKET, const NetworkProxyPtr&, const Address&);
    TcpTransceiver(const ProtocolInstancePtr&, SOCKET);
    virtual ~TcpTransceiver();

    void connect();

    friend class TcpConnector;
    friend class TcpAcceptor;

    const ProtocolInstancePtr _instance;
    const NetworkProxyPtr _proxy;
    const Address _addr;
    
    State _state;
    std::string _desc;

#ifdef ICE_USE_IOCP
    AsyncInfo _read;
    AsyncInfo _write;
    int _maxSendPacketSize;
    int _maxReceivePacketSize;
#endif
};

}

#endif
