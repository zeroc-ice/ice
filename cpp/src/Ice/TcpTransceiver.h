// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class TcpTransceiver : public Transceiver
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

private:

    TcpTransceiver(const InstancePtr&, SOCKET);
    virtual ~TcpTransceiver();
    friend class TcpConnector;
    friend class TcpAcceptor;

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    
    SOCKET _fd;
    fd_set _rFdSet;
    fd_set _wFdSet;

    const std::string _desc;
#ifdef _WIN32
    int _maxPacketSize;
#endif
};

}

#endif
