// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
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
    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&);
#endif
    virtual std::string type() const;
    virtual std::string toString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const Buffer&, size_t);

private:

    TcpTransceiver(const InstancePtr&, SOCKET, bool);
    virtual ~TcpTransceiver();

    void connect(const Address&);

    friend class TcpConnector;
    friend class TcpAcceptor;

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    
    State _state;
    std::string _desc;
    Address _connectAddr;

#ifdef ICE_USE_IOCP
    AsyncInfo _read;
    AsyncInfo _write;
    int _maxSendPacketSize;
    int _maxReceivePacketSize;
#endif
};

}
