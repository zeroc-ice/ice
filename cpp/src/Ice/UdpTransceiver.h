// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_TRANSCEIVER_H
#define ICE_UDP_TRANSCEIVER_H

#ifdef __hpux
#   define _XOPEN_SOURCE_EXTENDED
#endif

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/Transceiver.h>
#include <Ice/Protocol.h>
#include <IceUtil/Mutex.h>

#ifndef _WIN32
#   include <sys/socket.h> // For struct sockaddr_storage
#endif

namespace IceInternal
{

class UdpEndpoint;

class SUdpTransceiver;

class UdpTransceiver : public Transceiver, public NativeInfo
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateConnected,
        StateNotConnected
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

    int effectivePort() const;

private:

    UdpTransceiver(const InstancePtr&, const struct sockaddr_storage&, const std::string&, int);
    UdpTransceiver(const InstancePtr&, const std::string&, int, const std::string&, bool);
    virtual ~UdpTransceiver();

    void setBufSize(const InstancePtr&);

    friend class UdpEndpointI;
    friend class UdpConnector;

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    const bool _incoming;
    const struct sockaddr_storage _addr;
    struct sockaddr_storage _mcastAddr;
    struct sockaddr_storage _peerAddr;
    
    State _state;
    int _rcvSize;
    int _sndSize;
    static const int _udpOverhead;
    static const int _maxPacketSize;

#ifdef ICE_USE_IOCP
    AsyncInfo _read;
    AsyncInfo _write;
    struct sockaddr_storage _readAddr;
    socklen_t _readAddrLen;
#endif
};

}

#endif
