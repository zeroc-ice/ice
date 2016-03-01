// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/EndpointIF.h>
#include <Ice/ConnectorF.h>
#include <Ice/IPEndpointIF.h>
#include <Ice/NetworkF.h>
#include <Ice/Instance.h>

namespace IceInternal
{

class ICE_API ProtocolInstance : public IceUtil::Shared
{
public:

    ProtocolInstance(const Ice::CommunicatorPtr&, Ice::Short, const std::string&, bool);

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
        return _instance->initializationData().logger;
    }

    const std::string& protocol() const
    {
        return _protocol;
    }

    Ice::Short type() const
    {
        return _type;
    }

    const Ice::PropertiesPtr& properties() const
    {
        return _properties;
    }

    bool secure() const
    {
        return _secure;
    }

    BufSizeWarnInfo getBufSizeWarn(Ice::Short type)
    {
        return _instance->getBufSizeWarn(type);
    }

    void setSndBufSizeWarn(Ice::Short type, int size)
    {
        _instance->setSndBufSizeWarn(type, size);
    }

    void setRcvBufSizeWarn(Ice::Short type, int size)
    {
        _instance->setRcvBufSizeWarn(type, size);
    }

    bool preferIPv6() const;
    ProtocolSupport protocolSupport() const;
    const std::string& defaultHost() const;
    const Address& defaultSourceAddress() const;
    const Ice::EncodingVersion& defaultEncoding() const;
    NetworkProxyPtr networkProxy() const;
    size_t messageSizeMax() const;
    int defaultTimeout() const;

    void resolve(const std::string&, int, Ice::EndpointSelectionType, const IPEndpointIPtr&,
                 const EndpointI_connectorsPtr&) const;

protected:

    ProtocolInstance(const InstancePtr&, Ice::Short, const std::string&, bool);
    friend class Instance;
    const InstancePtr _instance;
    const int _traceLevel;
    const std::string _traceCategory;
    const Ice::PropertiesPtr _properties;
    const std::string _protocol;
    const Ice::Short _type;
    const bool _secure;
};

}

#endif
