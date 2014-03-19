// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROTOCOL_INSTANCE_H
#define ICE_PROTOCOL_INSTANCE_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/EndpointIF.h>
#include <Ice/ConnectorF.h>
#include <Ice/IPEndpointIF.h>
#include <Ice/NetworkF.h>

namespace IceInternal
{

class ICE_API ProtocolInstance : public IceUtil::Shared
{
public:
    
    ProtocolInstance(const Ice::CommunicatorPtr&, Ice::Short, const std::string&);
    
    int traceLevel() const
    {
        return _traceLevel;
    }

    const std::string& traceCategory() const
    {
        return _traceCategory;
    }

    const Ice::LoggerPtr& logger() const
    {
        return _logger;
    }

    const std::string& protocol() const
    {
        return _protocol;
    }

    Ice::Short type() const
    {
        return _type;
    }

    const Ice::StatsPtr& stats() const
    {
        return _stats;
    }

    const Ice::PropertiesPtr& properties() const
    {
        return _properties;
    }

    bool preferIPv6() const;
    ProtocolSupport protocolSupport() const;
    const std::string& defaultHost() const;
    const Ice::EncodingVersion& defaultEncoding() const;
    NetworkProxyPtr networkProxy() const;
    size_t messageSizeMax() const;
    
    std::vector<ConnectorPtr> resolve(const std::string&, int, Ice::EndpointSelectionType, const IPEndpointIPtr&) const;
    void resolve(const std::string&, int, Ice::EndpointSelectionType, const IPEndpointIPtr&, 
                 const EndpointI_connectorsPtr&) const;

private:

    ProtocolInstance(const InstancePtr&, Ice::Short, const std::string&);
    friend class Instance;
    const InstancePtr _instance;

protected:

    const int _traceLevel;
    const std::string _traceCategory;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    const Ice::PropertiesPtr _properties;
    const std::string _protocol;
    const Ice::Short _type;
};

}

#endif
