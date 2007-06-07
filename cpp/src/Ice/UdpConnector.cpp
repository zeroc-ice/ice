// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UdpConnector.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/UdpEndpointI.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::UdpConnector::connect(int timeout)
{
    return new UdpTransceiver(_instance, _addr, _mcastInterface, _mcastTtl);
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

    if(_protocolMajor != p->_protocolMajor)
    {
        return false;
    }

    if(_protocolMinor != p->_protocolMinor)
    {
        return false;
    }

    if(_encodingMajor != p->_encodingMajor)
    {
        return false;
    }

    if(_encodingMinor != p->_encodingMinor)
    {
        return false;
    }

    if(_mcastTtl != p->_mcastTtl)
    {
        return false;
    }

    if(_mcastInterface != p->_mcastInterface)
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

    if(_protocolMajor < p->_protocolMajor)
    {
        return true;
    }
    else if(p->_protocolMajor < _protocolMajor)
    {
        return false;
    }

    if(_protocolMinor < p->_protocolMinor)
    {
        return true;
    }
    else if(p->_protocolMinor < _protocolMinor)
    {
        return false;
    }

    if(_encodingMajor < p->_encodingMajor)
    {
        return true;
    }
    else if(p->_encodingMajor < _encodingMajor)
    {
        return false;
    }

    if(_encodingMinor < p->_encodingMinor)
    {
        return true;
    }
    else if(p->_encodingMinor < _encodingMinor)
    {
        return false;
    }

    if(_mcastTtl < p->_mcastTtl)
    {
        return true;
    }
    else if(p->_mcastTtl < _mcastTtl)
    {
        return false;
    }

    if(_mcastInterface < p->_mcastInterface)
    {
        return true;
    }
    else if(p->_mcastInterface < _mcastInterface)
    {
        return false;
    }

    return compareAddress(_addr, p->_addr) == -1;
}

IceInternal::UdpConnector::UdpConnector(const InstancePtr& instance, const struct sockaddr_in& addr, 
                                        const string& mcastInterface, int mcastTtl, Ice::Byte protocolMajor,
                                        Ice::Byte protocolMinor, Ice::Byte encodingMajor, Ice::Byte encodingMinor,
                                        const std::string& connectionId) :
    _instance(instance),
    _addr(addr),
    _mcastInterface(mcastInterface),
    _mcastTtl(mcastTtl),
    _protocolMajor(protocolMajor),
    _protocolMinor(protocolMinor),
    _encodingMajor(encodingMajor),
    _encodingMinor(encodingMinor),
    _connectionId(connectionId)
{
}

IceInternal::UdpConnector::~UdpConnector()
{
}
