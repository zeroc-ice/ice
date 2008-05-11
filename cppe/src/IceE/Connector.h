// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONNECTOR_H
#define ICEE_CONNECTOR_H

#include <IceE/ConnectorF.h>
#include <IceE/TransceiverF.h>
#include <IceE/InstanceF.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/LoggerF.h>
#include <IceE/Shared.h>

#ifdef _WIN32
#  include <winsock2.h>
#else
#  include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class Endpoint;

class Connector : public ::IceUtil::Shared
{
public:
    
    TransceiverPtr connect(int);
    std::string toString() const;
    
private:
    
    Connector(const InstancePtr&, const std::string&, int);
    virtual ~Connector();
    friend class TcpEndpoint;

    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    struct sockaddr_in _addr;
};

}

#endif
