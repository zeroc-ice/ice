// Copyright (c) ZeroC, Inc.

#include "AcceptorI.h"
#include "../Ice/Network.h"
#include "../Ice/StreamSocket.h"
#include "EndpointI.h"
#include "Engine.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"
#include "Instance.h"
#include "TransceiverI.h"
#include "Util.h"

using namespace std;
using namespace Ice;
using namespace IceBT;

namespace
{
    class ProfileCallbackI final : public ProfileCallback
    {
    public:
        ProfileCallbackI(AcceptorIPtr acceptor) : _acceptor(std::move(acceptor)) {}

        void newConnection(int fd) override { _acceptor->newConnection(fd); }

    private:
        AcceptorIPtr _acceptor;
    };
}

IceInternal::NativeInfoPtr
IceBT::AcceptorI::getNativeInfo()
{
    return shared_from_this();
}

void
IceBT::AcceptorI::close()
{
    if (!_path.empty())
    {
        try
        {
            _instance->engine()->unregisterProfile(_path);
        }
        catch (...)
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
        auto cb = make_shared<ProfileCallbackI>(shared_from_this());
        _path = _instance->engine()->registerProfile(_uuid, _name, _channel, cb);
    }
    catch (const BluetoothException& ex)
    {
        ostringstream os;
        os << "unable to register Bluetooth profile: " << ex.what();
        throw InitializationException(__FILE__, __LINE__, os.str());
    }

    _endpoint = _endpoint->endpoint(shared_from_this());
    return _endpoint;
}

IceInternal::TransceiverPtr
IceBT::AcceptorI::accept()
{
    //
    // The plug-in may not be initialized.
    //
    if (!_instance->initialized())
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceBT: plug-in is not initialized");
    }

    IceInternal::TransceiverPtr t;

    {
        lock_guard lock(_mutex);

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
    if (!_name.empty())
    {
        os << "\nservice name = '" << _name << "'";
    }
    if (!_uuid.empty())
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
    lock_guard lock(_mutex);

    _transceivers.push(make_shared<TransceiverI>(_instance, make_shared<StreamSocket>(_instance, fd), nullptr, _uuid));

    //
    // Notify the thread pool that we are ready to "read". The thread pool will invoke accept()
    // and we can return the new transceiver.
    //
    ready(IceInternal::SocketOperationRead, true);
}

IceBT::AcceptorI::AcceptorI(
    EndpointIPtr endpoint,
    InstancePtr instance,
    string adapterName,
    string addr,
    string uuid,
    string name,
    int channel)
    : _endpoint(std::move(endpoint)),
      _instance(std::move(instance)),
      _adapterName(std::move(adapterName)),
      _addr(std::move(addr)),
      _uuid(std::move(uuid)),
      _name(std::move(name)),
      _channel(channel)
{
    string s = IceInternal::trim(_addr);
    if (s.empty())
    {
        //
        // If no address was specified, we use the first available BT adapter.
        //
        s = _instance->engine()->getDefaultAdapterAddress();
    }

    s = IceInternal::toUpper(s);

    DeviceAddress da;
    if (!parseDeviceAddress(s, da))
    {
        throw ParseException(
            __FILE__,
            __LINE__,
            "invalid address value '" + s + "' in endpoint '" + _endpoint->toString() + "'");
    }
    if (!_instance->engine()->adapterExists(s))
    {
        throw ParseException(
            __FILE__,
            __LINE__,
            "no device found for '" + s + "' in endpoint '" + _endpoint->toString() + "'");
    }

    const_cast<string&>(_addr) = s;
}

IceBT::AcceptorI::~AcceptorI() = default;
