// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_UDP_TRANSCEIVER_H
#define ICE_UDP_TRANSCEIVER_H

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Transceiver.h>

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
    virtual void shutdown();
    virtual void write(Buffer&, int);
    virtual void read(Buffer&, int);
    virtual std::string toString() const;

    bool equivalent(const std::string&, int) const;
    int effectivePort();

private:

    UdpTransceiver(const InstancePtr&, const std::string&, int);
    UdpTransceiver(const InstancePtr&, const std::string&, int, bool);
    virtual ~UdpTransceiver();

    friend class UdpEndpoint;

    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    bool _incoming;
    bool _connect;
    SOCKET _fd;
    struct sockaddr_in _addr;
    fd_set _rFdSet;
    fd_set _wFdSet;
};

}

#endif
