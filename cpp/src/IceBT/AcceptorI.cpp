// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/AcceptorI.h>
#include <IceBT/Engine.h>
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

namespace
{

class ProfileCallbackI : public ProfileCallback
{
public:

    ProfileCallbackI(const AcceptorIPtr& acceptor) :
        _acceptor(acceptor)
    {
    }

    virtual void newConnection(int fd)
    {
        _acceptor->newConnection(fd);
    }

private:

    AcceptorIPtr _acceptor;
};

}

IceInternal::NativeInfoPtr
IceBT::AcceptorI::getNativeInfo()
{
    return this;
}

void
IceBT::AcceptorI::close()
{
    if(!_path.empty())
    {
        try
        {
            _instance->engine()->unregisterProfile(_path);
        }
        catch(...)
        {
        }
    }
}

IceInternal::EndpointIPtr
IceBT::AcceptorI::listen()
{
    assert(!_uuid.empty());

    //
    // The Bluetooth daemon will select an available channel if _channel == 0.
    //

    try
    {
        ProfileCallbackPtr cb = ICE_MAKE_SHARED(ProfileCallbackI, this);
        _path = _instance->engine()->registerProfile(_uuid, _name, _channel, cb);
    }
    catch(const BluetoothException& ex)
    {
        InitializationException e(__FILE__, __LINE__);
        e.reason = "unable to register Bluetooth profile";
        if(!ex.reason.empty())
        {
            e.reason += "\n" + ex.reason;
        }
        throw e;
    }

    _endpoint = _endpoint->endpoint(this);
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

    IceInternal::TransceiverPtr t;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        //
        // The thread pool should only call accept() when we've notified it that we have a
        // new transceiver ready to be accepted.
        //
        assert(!_transceivers.empty());
        t = _transceivers.top();
        _transceivers.pop();

        //
        // Update our status with the thread pool.
        //
        ready(IceInternal::SocketOperationRead, !_transceivers.empty());
    }

    return t;
}

string
IceBT::AcceptorI::protocol() const
{
    return _instance->protocol();
}

string
IceBT::AcceptorI::toString() const
{
    return addrToString(_addr, _channel);
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
    //
    // If no channel was specified in the endpoint (_channel == 0), the Bluetooth daemon will select
    // an available channel for us. Unfortunately, there's no way to discover what that channel is
    // (aside from waiting for the first incoming connection and inspecting the socket endpoint).
    //

    return _channel;
}

void
IceBT::AcceptorI::newConnection(int fd)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

    _transceivers.push(new TransceiverI(_instance, new StreamSocket(_instance, fd), 0, _uuid));

    //
    // Notify the thread pool that we are ready to "read". The thread pool will invoke accept()
    // and we can return the new transceiver.
    //
    ready(IceInternal::SocketOperationRead, true);
}

IceBT::AcceptorI::AcceptorI(const EndpointIPtr& endpoint, const InstancePtr& instance, const string& adapterName,
                            const string& addr, const string& uuid, const string& name, int channel) :
    _endpoint(endpoint),
    _instance(instance),
    _adapterName(adapterName),
    _addr(addr),
    _uuid(uuid),
    _name(name),
    _channel(channel)
{
    string s = IceUtilInternal::trim(_addr);
    if(s.empty())
    {
        //
        // If no address was specified, we use the first available BT adapter.
        //
        s = _instance->engine()->getDefaultAdapterAddress();
    }

    s = IceUtilInternal::toUpper(s);

    DeviceAddress da;
    if(!parseDeviceAddress(s, da))
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "invalid address value `" + s + "' in endpoint " + endpoint->toString();
        throw ex;
    }
    if(!_instance->engine()->adapterExists(s))
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "no device found for `" + s + "' in endpoint " + endpoint->toString();
        throw ex;
    }

    const_cast<string&>(_addr) = s;
}

IceBT::AcceptorI::~AcceptorI()
{
}
