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

#ifndef ICE_TCP_CONNECTOR_H
#define ICE_TCP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Connector.h>

#ifndef _WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class TcpEndpoint;

class TcpConnector : public Connector
{
public:
    
    virtual TransceiverPtr connect(int);
    virtual std::string toString() const;
    
private:
    
    TcpConnector(const InstancePtr&, const std::string&, int);
    virtual ~TcpConnector();
    friend class TcpEndpoint;

    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    struct sockaddr_in _addr;
};

}

#endif
