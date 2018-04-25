// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/EndpointI.h>
#include <IceBT/AcceptorI.h>
#include <IceBT/ConnectorI.h>
#include <IceBT/Engine.h>
#include <IceBT/Instance.h>
#include <IceBT/Util.h>

#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/HashUtil.h>
#include <Ice/Object.h>
#include <Ice/Properties.h>
#include <Ice/UUID.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceBT::upCast(EndpointI* p) { return p; }
#endif

IceBT::EndpointI::EndpointI(const InstancePtr& instance, const string& addr, const string& uuid, const string& name,
                            Int channel, Int timeout, const string& connectionId, bool compress) :
    _instance(instance),
    _addr(addr),
    _uuid(uuid),
    _name(name),
    _channel(channel),
    _timeout(timeout),
    _connectionId(connectionId),
    _compress(compress),
    _hashValue(0)
{
    hashInit();
}

IceBT::EndpointI::EndpointI(const InstancePtr& instance) :
    _instance(instance),
    _channel(0),
    _timeout(instance->defaultTimeout()),
    _compress(false),
    _hashValue(0)
{
}

IceBT::EndpointI::EndpointI(const InstancePtr& instance, InputStream* s) :
    _instance(instance),
    _channel(0),
    _timeout(-1),
    _compress(false),
    _hashValue(0)
{
    //
    // _name and _channel are not marshaled.
    //
    s->read(const_cast<string&>(_addr), false);
    s->read(const_cast<string&>(_uuid), false);
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
    hashInit();
}

void
IceBT::EndpointI::streamWriteImpl(OutputStream* s) const
{
    //
    // _name and _channel are not marshaled.
    //
    s->write(_addr, false);
    s->write(_uuid, false);
    s->write(_timeout);
    s->write(_compress);
}

Ice::Short
IceBT::EndpointI::type() const
{
    return _instance->type();
}

const string&
IceBT::EndpointI::protocol() const
{
    return _instance->protocol();
}

Int
IceBT::EndpointI::timeout() const
{
    return _timeout;
}

IceInternal::EndpointIPtr
IceBT::EndpointI::timeout(Int timeout) const
{
    if(timeout == _timeout)
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
        return ICE_MAKE_SHARED(EndpointI, _instance, _addr, _uuid, _name, _channel, timeout, _connectionId, _compress);
    }
}

const string&
IceBT::EndpointI::connectionId() const
{
    return _connectionId;
}

IceInternal::EndpointIPtr
IceBT::EndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
        return ICE_MAKE_SHARED(EndpointI, _instance, _addr, _uuid, _name, _channel, _timeout, connectionId, _compress);
    }
}

bool
IceBT::EndpointI::compress() const
{
    return _compress;
}

IceInternal::EndpointIPtr
IceBT::EndpointI::compress(bool compress) const
{
    if(compress == _compress)
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
        return ICE_MAKE_SHARED(EndpointI, _instance, _addr, _uuid, _name, _channel, _timeout, _connectionId, compress);
    }
}

bool
IceBT::EndpointI::datagram() const
{
    return false;
}

bool
IceBT::EndpointI::secure() const
{
    return _instance->secure();
}

IceInternal::TransceiverPtr
IceBT::EndpointI::transceiver() const
{
    return 0;
}

void
IceBT::EndpointI::connectors_async(EndpointSelectionType selType, const IceInternal::EndpointI_connectorsPtr& cb) const
{
    vector<IceInternal::ConnectorPtr> connectors;
    connectors.push_back(new ConnectorI(_instance, _addr, _uuid, _timeout, _connectionId));
    cb->connectors(connectors);
}

IceInternal::AcceptorPtr
IceBT::EndpointI::acceptor(const string& adapterName) const
{
    return new AcceptorI(ICE_SHARED_FROM_CONST_THIS(EndpointI), _instance, adapterName, _addr, _uuid, _name, _channel);
}

vector<IceInternal::EndpointIPtr>
IceBT::EndpointI::expandIfWildcard() const
{
    vector<IceInternal::EndpointIPtr> endps;

    if(_addr.empty())
    {
        //
        // getDefaultAdapterAddress will raise BluetoothException if no adapter is present.
        //
        string addr = _instance->engine()->getDefaultAdapterAddress();
        endps.push_back(ICE_MAKE_SHARED(EndpointI, _instance, addr, _uuid, _name, _channel, _timeout, _connectionId,
                                        _compress));
    }
    else
    {
        endps.push_back(ICE_SHARED_FROM_CONST_THIS(EndpointI));
    }

    return endps;
}

vector<IceInternal::EndpointIPtr>
IceBT::EndpointI::expandHost(IceInternal::EndpointIPtr&) const
{
    //
    // Nothing to do here.
    //
    vector<IceInternal::EndpointIPtr> endps;
    endps.push_back(ICE_SHARED_FROM_CONST_THIS(EndpointI));
    return endps;
}

bool
IceBT::EndpointI::equivalent(const IceInternal::EndpointIPtr& endpoint) const
{
    const EndpointI* btEndpointI = dynamic_cast<const EndpointI*>(endpoint.get());
    if(!btEndpointI)
    {
        return false;
    }
    return btEndpointI->type() == type() && btEndpointI->_addr == _addr && btEndpointI->_uuid == _uuid;
}

bool
#ifdef ICE_CPP11_MAPPING
IceBT::EndpointI::operator==(const Ice::Endpoint& r) const
#else
IceBT::EndpointI::operator==(const Ice::LocalObject& r) const
#endif
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_addr != p->_addr)
    {
        return false;
    }

    if(_uuid != p->_uuid)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    if(_channel != p->_channel)
    {
        return false;
    }

    if(_timeout != p->_timeout)
    {
        return false;
    }

    if(_compress != p->_compress)
    {
        return false;
    }

    return true;
}

bool
#ifdef ICE_CPP11_MAPPING
IceBT::EndpointI::operator<(const Ice::Endpoint& r) const
#else
IceBT::EndpointI::operator<(const Ice::LocalObject& r) const
#endif
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
    if(!p)
    {
        const IceInternal::EndpointI* e = dynamic_cast<const IceInternal::EndpointI*>(&r);
        if(!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if(this == p)
    {
        return false;
    }

    if(type() < p->type())
    {
        return true;
    }
    else if(p->type() < type())
    {
        return false;
    }

    if(_addr < p->_addr)
    {
        return true;
    }
    else if(p->_addr < _addr)
    {
        return false;
    }

    if(_uuid < p->_uuid)
    {
        return true;
    }
    else if(p->_uuid < _uuid)
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

    if(_channel < p->_channel)
    {
        return true;
    }
    else if(p->_channel < _channel)
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

    if(!_compress && p->_compress)
    {
        return true;
    }
    else if(p->_compress < _compress)
    {
        return false;
    }

    return false;
}

Ice::Int
IceBT::EndpointI::hash() const
{
    return _hashValue;
}

string
IceBT::EndpointI::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;

    if(!_addr.empty())
    {
        s << " -a ";
        bool addQuote = _addr.find(':') != string::npos;
        if(addQuote)
        {
            s << "\"";
        }
        s << _addr;
        if(addQuote)
        {
            s << "\"";
        }
    }

    if(!_uuid.empty())
    {
        s << " -u ";
        bool addQuote = _uuid.find(':') != string::npos;
        if(addQuote)
        {
            s << "\"";
        }
        s << _uuid;
        if(addQuote)
        {
            s << "\"";
        }
    }

    if(_channel > 0)
    {
        s << " -c " << _channel;
    }

    if(_timeout == -1)
    {
        s << " -t infinite";
    }
    else
    {
        s << " -t " << _timeout;
    }

    if(_compress)
    {
        s << " -z";
    }

    return s.str();
}

Ice::EndpointInfoPtr
IceBT::EndpointI::getInfo() const ICE_NOEXCEPT
{
    EndpointInfoPtr info = ICE_MAKE_SHARED(EndpointInfoI, ICE_SHARED_FROM_CONST_THIS(EndpointI));
    info->addr = _addr;
    info->uuid = _uuid;
    return info;
}

void
IceBT::EndpointI::initWithOptions(vector<string>& args, bool oaEndpoint)
{
    IceInternal::EndpointI::initWithOptions(args);

    if(_addr.empty())
    {
        const_cast<string&>(_addr) = _instance->defaultHost();
    }
    else if(_addr == "*")
    {
        if(oaEndpoint)
        {
            const_cast<string&>(_addr) = string();
        }
        else
        {
            throw EndpointParseException(__FILE__, __LINE__,
                                         "`-a *' not valid for proxy endpoint `" + toString() + "'");
        }
    }

    if(_name.empty())
    {
        const_cast<string&>(_name) = "Ice Service";
    }

    if(_uuid.empty())
    {
        if(oaEndpoint)
        {
            //
            // Generate a UUID for object adapters that don't specify one.
            //
            const_cast<string&>(_uuid) = generateUUID();
        }
        else
        {
            throw EndpointParseException(__FILE__, __LINE__, "a UUID must be specified using the -u option");
        }
    }

    if(_channel < 0)
    {
        const_cast<Int&>(_channel) = 0;
    }

    if(!oaEndpoint && _channel != 0)
    {
        throw EndpointParseException(__FILE__, __LINE__, "the -c option can only be used for object adapter endpoints");
    }

    hashInit();
}

IceBT::EndpointIPtr
IceBT::EndpointI::endpoint(const AcceptorIPtr& acceptor) const
{
    return ICE_MAKE_SHARED(EndpointI, _instance, _addr, _uuid, _name, acceptor->effectiveChannel(), _timeout,
                           _connectionId, _compress);
}

void
IceBT::EndpointI::hashInit()
{
    Int h = 5381;
    IceInternal::hashAdd(h, _addr);
    IceInternal::hashAdd(h, _uuid);
    IceInternal::hashAdd(h, _timeout);
    IceInternal::hashAdd(h, _connectionId);
    IceInternal::hashAdd(h, _compress);
    const_cast<Int&>(_hashValue) = h;
}

bool
IceBT::EndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    string arg = IceUtilInternal::trim(argument);
    if(option == "-a")
    {
        if(arg.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "no argument provided for -a option in endpoint " +
                                         endpoint);
        }
        if(arg != "*" && !isValidDeviceAddress(arg))
        {
            throw EndpointParseException(__FILE__, __LINE__, "invalid argument provided for -a option in endpoint " +
                                         endpoint);
        }
        const_cast<string&>(_addr) = arg;
    }
    else if(option == "-u")
    {
        if(arg.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "no argument provided for -u option in endpoint " +
                                         endpoint);
        }
        const_cast<string&>(_uuid) = arg;
    }
    else if(option == "-c")
    {
        if(arg.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "no argument provided for -c option in endpoint " +
                                         endpoint);
        }

        istringstream t(argument);
        if(!(t >> const_cast<Int&>(_channel)) || !t.eof() || _channel < 0 || _channel > 30)
        {
            throw EndpointParseException(__FILE__, __LINE__, "invalid channel value `" + arg + "' in endpoint " +
                                         endpoint);
        }
    }
    else if(option == "-t")
    {
        if(arg.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "no argument provided for -t option in endpoint " +
                                         endpoint);
        }

        if(arg == "infinite")
        {
            const_cast<Int&>(_timeout) = -1;
        }
        else
        {
            istringstream t(argument);
            if(!(t >> const_cast<Int&>(_timeout)) || !t.eof() || _timeout < 1)
            {
                throw EndpointParseException(__FILE__, __LINE__, "invalid timeout value `" + arg + "' in endpoint " +
                                             endpoint);
            }
        }
    }
    else if(option == "-z")
    {
        if(!arg.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "unexpected argument `" + arg +
                                         "' provided for -z option in " + endpoint);
        }
        const_cast<bool&>(_compress) = true;
    }
    else if(option == "--name")
    {
        if(arg.empty())
        {
            throw EndpointParseException(__FILE__, __LINE__, "no argument provided for --name option in endpoint " +
                                         endpoint);
        }
        const_cast<string&>(_name) = arg;
    }
    else
    {
        return false;
    }
    return true;
}

IceBT::EndpointInfoI::EndpointInfoI(const EndpointIPtr& endpoint) : _endpoint(endpoint)
{
}

IceBT::EndpointInfoI::~EndpointInfoI()
{
}

Ice::Short
IceBT::EndpointInfoI::type() const ICE_NOEXCEPT
{
    return _endpoint->type();
}

bool
IceBT::EndpointInfoI::datagram() const ICE_NOEXCEPT
{
    return _endpoint->datagram();
}

bool
IceBT::EndpointInfoI::secure() const ICE_NOEXCEPT
{
    return _endpoint->secure();
}

IceBT::EndpointFactoryI::EndpointFactoryI(const InstancePtr& instance) : _instance(instance)
{
}

IceBT::EndpointFactoryI::~EndpointFactoryI()
{
}

Short
IceBT::EndpointFactoryI::type() const
{
    return _instance->type();
}

string
IceBT::EndpointFactoryI::protocol() const
{
    return _instance->protocol();
}

IceInternal::EndpointIPtr
IceBT::EndpointFactoryI::create(vector<string>& args, bool oaEndpoint) const
{
    EndpointIPtr endpt = ICE_MAKE_SHARED(EndpointI, _instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

IceInternal::EndpointIPtr
IceBT::EndpointFactoryI::read(InputStream* s) const
{
    return ICE_MAKE_SHARED(EndpointI, _instance, s);
}

void
IceBT::EndpointFactoryI::destroy()
{
    _instance = 0;
}

IceInternal::EndpointFactoryPtr
IceBT::EndpointFactoryI::clone(const IceInternal::ProtocolInstancePtr& instance) const
{
    return new EndpointFactoryI(new Instance(_instance->engine(), instance->type(), instance->protocol()));
}
