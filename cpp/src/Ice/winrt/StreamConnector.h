// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
#include <Ice/Network.h>

namespace IceInternal
{

class StreamConnector : public Connector
{
public:
    
    virtual TransceiverPtr connect();

    virtual Ice::Short type() const;
    virtual std::string toString() const;

    virtual bool operator==(const Connector&) const;
    virtual bool operator!=(const Connector&) const;
    virtual bool operator<(const Connector&) const;

private:

    StreamConnector(const InstancePtr&, Ice::Short, const Address&, Ice::Int, const std::string&); 
    virtual ~StreamConnector();
    friend class StreamEndpointI;

    const InstancePtr _instance;
    const Ice::Short _type;
    const TraceLevelsPtr _traceLevels;
    const ::Ice::LoggerPtr _logger;
    const Address _addr;
    const Ice::Int _timeout;
    const std::string _connectionId;
};

}

#endif
