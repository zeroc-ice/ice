// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TCP_CONNECTOR_H
#define ICE_TCP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Connector.h>

#ifdef _WIN32
#   include <winsock2.h>
#else
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
