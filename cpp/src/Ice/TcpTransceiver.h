// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;

private:

    TcpTransceiver(const InstancePtr&, SOCKET);
    virtual ~TcpTransceiver();
    friend class TcpConnector;
    friend class TcpAcceptor;

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    const std::string _name;
    const std::string _desc;
    
    SOCKET _fd;
    fd_set _rFdSet;
    fd_set _wFdSet;
};

}

#endif
