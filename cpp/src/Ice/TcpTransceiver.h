// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TCP_TRANSCEIVER_H
#define ICE_TCP_TRANSCEIVER_H

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/Transceiver.h>

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
        StateConnected
    };

public:

    virtual NativeInfoPtr getNativeInfo();
#ifdef ICE_USE_IOCP
    virtual AsyncInfo* getAsyncInfo(SocketOperation);
#endif

    virtual SocketOperation initialize();
    virtual void close();
    virtual bool write(Buffer&);
    virtual bool read(Buffer&);
#ifdef ICE_USE_IOCP
    virtual void startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&);
#endif
    virtual std::string type() const;
    virtual std::string toString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const Buffer&, size_t);

private:

    TcpTransceiver(const InstancePtr&, const Ice::TcpEndpointInfoPtr&, SOCKET, bool);
    virtual ~TcpTransceiver();

    void connect(const struct sockaddr_storage&);

    friend class TcpConnector;
    friend class TcpAcceptor;

    const Ice::TcpEndpointInfoPtr _endpointInfo;
    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    
    State _state;
    std::string _desc;
#ifdef ICE_USE_IOCP
    struct sockaddr_storage _connectAddr;
    AsyncInfo _read;
    AsyncInfo _write;
    int _maxSendPacketSize;
    int _maxReceivePacketSize;
#endif
};

}

#endif
