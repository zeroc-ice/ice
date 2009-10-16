// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UdpConnector.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/UdpEndpointI.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::UdpConnector::connect()
{
    return new UdpTransceiver(_instance, _endpointInfo, _addr);
}

Short
IceInternal::UdpConnector::type() const
{
    return UdpEndpointType;
}

string
IceInternal::UdpConnector::toString() const
{
    return addrToString(_addr);
}

bool
IceInternal::UdpConnector::operator==(const Connector& r) const
{
    const UdpConnector* p = dynamic_cast<const UdpConnector*>(&r);
    if(!p)
    {
        return false;
    }

    if(compareAddress(_addr, p->_addr) != 0)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    if(_endpointInfo->protocolMajor != p->_endpointInfo->protocolMajor)
    {
        return false;
    }

    if(_endpointInfo->protocolMinor != p->_endpointInfo->protocolMinor)
    {
        return false;
    }

    if(_endpointInfo->encodingMajor != p->_endpointInfo->encodingMajor)
    {
        return false;
    }

    if(_endpointInfo->encodingMinor != p->_endpointInfo->encodingMinor)
    {
        return false;
    }

    if(_endpointInfo->mcastTtl != p->_endpointInfo->mcastTtl)
    {
        return false;
    }

    if(_endpointInfo->mcastInterface != p->_endpointInfo->mcastInterface)
    {
        return false;
    }

    return true;
}

bool
IceInternal::UdpConnector::operator!=(const Connector& r) const
{
    return !operator==(r);
}

bool
IceInternal::UdpConnector::operator<(const Connector& r) const
{
    const UdpConnector* p = dynamic_cast<const UdpConnector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(_connectionId < p->_connectionId)
    {
        return true;
    }
    else if(p->_connectionId < _connectionId)
    {
        return false;
    }

    if(_endpointInfo->protocolMajor < p->_endpointInfo->protocolMajor)
    {
        return true;
    }
    else if(p->_endpointInfo->protocolMajor < _endpointInfo->protocolMajor)
    {
        return false;
    }

    if(_endpointInfo->protocolMinor < p->_endpointInfo->protocolMinor)
    {
        return true;
    }
    else if(p->_endpointInfo->protocolMinor < _endpointInfo->protocolMinor)
    {
        return false;
    }

    if(_endpointInfo->encodingMajor < p->_endpointInfo->encodingMajor)
    {
        return true;
    }
    else if(p->_endpointInfo->encodingMajor < _endpointInfo->encodingMajor)
    {
        return false;
    }

    if(_endpointInfo->encodingMinor < p->_endpointInfo->encodingMinor)
    {
        return true;
    }
    else if(p->_endpointInfo->encodingMinor < _endpointInfo->encodingMinor)
    {
        return false;
    }

    if(_endpointInfo->mcastTtl < p->_endpointInfo->mcastTtl)
    {
        return true;
    }
    else if(p->_endpointInfo->mcastTtl < _endpointInfo->mcastTtl)
    {
        return false;
    }

    if(_endpointInfo->mcastInterface < p->_endpointInfo->mcastInterface)
    {
        return true;
    }
    else if(p->_endpointInfo->mcastInterface < _endpointInfo->mcastInterface)
    {
        return false;
    }

    return compareAddress(_addr, p->_addr) == -1;
}


IceInternal::UdpConnector::UdpConnector(const InstancePtr& instance,
                                        const UdpEndpointInfoPtr& endpointInfo,
                                        const struct sockaddr_storage& addr, 
                                        const std::string& connectionId) :
    _instance(instance),
    _endpointInfo(endpointInfo),
    _addr(addr),
    _connectionId(connectionId)
{
}

IceInternal::UdpConnector::~UdpConnector()
{
}
