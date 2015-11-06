// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/ConnectorI.h>
#include <IceBT/EndpointI.h>
#include <IceBT/Instance.h>
#include <IceBT/TransceiverI.h>
#include <IceBT/Util.h>

#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

IceInternal::TransceiverPtr
IceBT::ConnectorI::connect()
{
    //
    // The plug-in may not be initialized.
    //
    if(!_instance->initialized())
    {
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceBT: plug-in is not initialized";
        throw ex;
    }

    assert(_fd != -1);

    //
    // Transceiver takes ownership of the file descriptor and connection.
    //
    IceInternal::TransceiverPtr t = new TransceiverI(_instance, new StreamSocket(_instance, _fd), _connection, _uuid);
    _fd = -1;
    _connection = 0;

    return t;
}

Short
IceBT::ConnectorI::type() const
{
    return _instance->type();
}

string
IceBT::ConnectorI::toString() const
{
    return _addr;
}

bool
IceBT::ConnectorI::operator==(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return false;
    }

    if(_addr != p->_addr)
    {
        return false;
    }

    if(_uuid != p->_uuid)
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
IceBT::ConnectorI::operator!=(const IceInternal::Connector& r) const
{
    return !operator==(r);
}

bool
IceBT::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(_addr < p->_addr)
    {
        return true;
    }

    if(_uuid < p->_uuid)
    {
        return true;
    }
    else if(p->_uuid < _uuid)
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

    return _connectionId < p->_connectionId;
}

IceBT::ConnectorI::ConnectorI(const InstancePtr& instance, SOCKET fd, const ConnectionPtr& conn, const string& addr,
                              const string& uuid, Int timeout, const string& connectionId) :
    _instance(instance),
    _fd(fd),
    _connection(conn),
    _addr(addr),
    _uuid(uuid),
    _timeout(timeout),
    _connectionId(connectionId)
{
}

IceBT::ConnectorI::~ConnectorI()
{
    //
    // We must close the connection and socket if we haven't passed them to a transceiver yet.
    //
    if(_fd != -1)
    {
        _connection->close();
        IceInternal::closeSocketNoThrow(_fd);
    }
}
