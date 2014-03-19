// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/winrt/StreamConnector.h>
#include <Ice/winrt/StreamTransceiver.h>
#include <Ice/winrt/StreamEndpointI.h>

#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::StreamConnector::connect()
{
    if(_traceLevels->network >= 2)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "trying to establish " << typeToString(_type) << " connection to " << toString();
    }

    try
    {
        TransceiverPtr transceiver = new StreamTransceiver(_instance, _type, createSocket(false, _addr), false);
        dynamic_cast<StreamTransceiver*>(transceiver.get())->connect(_addr);
        return transceiver;
    }
    catch(const Ice::LocalException& ex)
    {
        if(_traceLevels->network >= 2)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "failed to establish " << typeToString(_type) << " connection to " << toString() << "\n" << ex;
        }
        throw;
    }
}

Short
IceInternal::StreamConnector::type() const
{
    return _type;
}

string
IceInternal::StreamConnector::toString() const
{
    return addrToString(_addr);
}

bool
IceInternal::StreamConnector::operator==(const Connector& r) const
{
    const StreamConnector* p = dynamic_cast<const StreamConnector*>(&r);
    if(!p)
    {
        return false;
    }

    if(_type != p->_type)
    {
        return false;
    }

    if(compareAddress(_addr, p->_addr) != 0)
    {
        return false;
    }
    
    if(_timeout != p->_timeout)
    {
        return false;
    }
    
    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    return true;
}

bool
IceInternal::StreamConnector::operator!=(const Connector& r) const
{
    return !operator==(r);
}

bool
IceInternal::StreamConnector::operator<(const Connector& r) const
{
    const StreamConnector* p = dynamic_cast<const StreamConnector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(_type < p->_type)
    {
        return true;
    }
    else if(p->_type < _type)
    {
        return false;
    }

    if(_timeout < p->_timeout)
    {
        return true;
    }
    else if(p->_timeout < _timeout)
    {
        return false;
    }
    
    if(_connectionId < p->_connectionId)
    {
        return true;
    }
    else if(p->_connectionId < _connectionId)
    {
        return false;
    }
    return compareAddress(_addr, p->_addr) < 0;
}

IceInternal::StreamConnector::StreamConnector(const InstancePtr& instance, Ice::Short type, const Address& addr, 
                                              Ice::Int timeout, const string& connectionId) :
    _instance(instance),
    _type(type),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _addr(addr),
    _timeout(timeout),
    _connectionId(connectionId)
{
}

IceInternal::StreamConnector::~StreamConnector()
{
}
