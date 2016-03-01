// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/AcceptorI.h>
#include <IceBT/EndpointI.h>
#include <IceBT/Instance.h>
#include <IceBT/TransceiverI.h>
#include <IceBT/Util.h>

#include <Ice/Communicator.h>
#include <Ice/Exception.h>
#include <Ice/LocalException.h>
#include <Ice/Network.h>
#include <Ice/Properties.h>
#include <Ice/StreamSocket.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

IceUtil::Shared* IceBT::upCast(AcceptorI* p) { return p; }

IceInternal::NativeInfoPtr
IceBT::AcceptorI::getNativeInfo()
{
    return this;
}

void
IceBT::AcceptorI::close()
{
    try
    {
        _instance->engine()->removeService(_adapter, _serviceHandle);
    }
    catch(const BluetoothException&)
    {
        // Ignore.
    }

    if(_fd != INVALID_SOCKET)
    {
        IceInternal::closeSocketNoThrow(_fd);
        _fd = INVALID_SOCKET;
    }
}

IceInternal::EndpointIPtr
IceBT::AcceptorI::listen()
{
    try
    {
        if(!_instance->engine()->adapterExists(_adapter))
        {
            throw SocketException(__FILE__, __LINE__, EADDRNOTAVAIL);
        }
        _addr = doBind(_fd, _addr);
        IceInternal::doListen(_fd, _backlog);
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }

    assert(!_uuid.empty());

    _serviceHandle = _instance->engine()->addService(_adapter, _name, _uuid, _addr.rc_channel);

    return _endpoint;
}

IceInternal::TransceiverPtr
IceBT::AcceptorI::accept()
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

    SOCKET fd = doAccept(_fd);

    return new TransceiverI(_instance, new StreamSocket(_instance, fd), _uuid);
}

string
IceBT::AcceptorI::protocol() const
{
    return _instance->protocol();
}

string
IceBT::AcceptorI::toString() const
{
    return addrToString(_addr);
}

string
IceBT::AcceptorI::toDetailedString() const
{
    ostringstream os;
    os << "local address = " << toString();
    if(!_name.empty())
    {
        os << "\nservice name = '" << _name << "'";
    }
    if(!_uuid.empty())
    {
        os << "\nservice uuid = " << _uuid;
    }
    return os.str();
}

int
IceBT::AcceptorI::effectiveChannel() const
{
    return _addr.rc_channel;
}

IceBT::AcceptorI::AcceptorI(const EndpointIPtr& endpoint, const InstancePtr& instance, const string& adapterName,
                            const string& addr, const string& uuid, const string& name, int channel) :
    _endpoint(endpoint),
    _instance(instance),
    _adapterName(adapterName),
    _uuid(uuid),
    _name(name),
    _serviceHandle(0)
{
    // TBD - Necessary?
    //_backlog = instance->properties()->getPropertyAsIntWithDefault("IceBT.Backlog", 1);
    _backlog = 1;

    _addr.rc_family = AF_BLUETOOTH;
    _addr.rc_channel = channel;

    _adapter = IceUtilInternal::trim(addr);
    if(_adapter.empty())
    {
        //
        // If no address was specified, we use the first available BT adapter.
        //
        _adapter = _instance->engine()->getDefaultAdapterAddress();
    }

    _adapter = IceUtilInternal::toUpper(_adapter);

    if(!parseDeviceAddress(_adapter, _addr.rc_bdaddr))
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "invalid address value `" + _adapter + "' in endpoint " + endpoint->toString();
        throw ex;
    }

    _fd = createSocket();
    IceInternal::setBlock(_fd, false);
}

IceBT::AcceptorI::~AcceptorI()
{
    assert(_fd == INVALID_SOCKET);
}
