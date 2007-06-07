// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_TRANSCEIVER_H
#define ICE_UDP_TRANSCEIVER_H

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/Transceiver.h>
#include <IceUtil/Mutex.h>

#ifndef _WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class UdpEndpoint;

class SUdpTransceiver;

class UdpTransceiver : public Transceiver
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void shutdownWrite();
    virtual void shutdownReadWrite();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string type() const;
    virtual std::string toString() const;
    virtual void initialize(int);
    virtual void checkSendSize(const Buffer&, size_t);

    bool equivalent(const std::string&, int) const;
    int effectivePort() const;

private:

    UdpTransceiver(const InstancePtr&, const struct sockaddr_in&, const std::string&, int);
    UdpTransceiver(const InstancePtr&, const std::string&, int, const std::string&, bool);
    virtual ~UdpTransceiver();

    void setBufSize(const InstancePtr&);

    friend class UdpEndpointI;
    friend class UdpConnector;

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    const bool _incoming;

    SOCKET _fd;
    struct sockaddr_in _addr;
    bool _mcastServer;
#ifdef _WIN32
    fd_set _rFdSet;
    fd_set _wFdSet;
#endif
    bool _connect;
    int _rcvSize;
    int _sndSize;
    const bool _warn;
    static const int _udpOverhead;
    static const int _maxPacketSize;
    bool _shutdownReadWrite;
    IceUtil::Mutex _shutdownReadWriteMutex;
};

}

#endif
