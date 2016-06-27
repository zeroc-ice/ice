// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "StreamEndpointI.h"
#include "StreamAcceptor.h"
#include "StreamConnector.h"

#include <IceUtil/StringUtil.h>

#include <Ice/Network.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <Ice/LocalException.h>
#include <Ice/Communicator.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/Properties.h>
#include <Ice/HashUtil.h>
#include <Ice/NetworkProxy.h>

#include <CoreFoundation/CoreFoundation.h>

#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

extern "C"
{

Plugin*
createIceTCP(const CommunicatorPtr& com, const string&, const StringSeq&)
{
    IceObjC::InstancePtr tcpInstance = new IceObjC::Instance(com, TCPEndpointType, "tcp", false);
    return new EndpointFactoryPlugin(com, new IceObjC::StreamEndpointFactory(tcpInstance));
}

}

namespace
{

inline CFStringRef
toCFString(const string& s)
{
    return CFStringCreateWithCString(NULL, s.c_str(), kCFStringEncodingUTF8);
}

}

IceObjC::Instance::Instance(const Ice::CommunicatorPtr& com, Short type, const string& protocol, bool secure) :
    ProtocolInstance(com, type, protocol, secure),
    _voip(com->getProperties()->getPropertyAsIntWithDefault("Ice.Voip", 0) > 0),
    _communicator(com),
    _proxySettings(0)
{
    const Ice::PropertiesPtr properties = com->getProperties();

    //
    // Proxy settings
    //
    _proxyHost = properties->getProperty("Ice.SOCKSProxyHost");
    if(!_proxyHost.empty())
    {
#if TARGET_IPHONE_SIMULATOR != 0
        throw Ice::FeatureNotSupportedException(__FILE__, __LINE__, "SOCKS proxy not supported");
#endif
        _proxySettings = CFDictionaryCreateMutable(0, 3, &kCFTypeDictionaryKeyCallBacks,
                                                   &kCFTypeDictionaryValueCallBacks);

        _proxyPort = properties->getPropertyAsIntWithDefault("Ice.SOCKSProxyPort", 1080);

        CFStringRef host = toCFString(_proxyHost);
        CFDictionarySetValue(_proxySettings, kCFStreamPropertySOCKSProxyHost, host);
        CFRelease(host);

        CFNumberRef port = CFNumberCreate(0, kCFNumberSInt32Type, &_proxyPort);
        CFDictionarySetValue(_proxySettings, kCFStreamPropertySOCKSProxyPort, port);
        CFRelease(port);

        CFDictionarySetValue(_proxySettings, kCFStreamPropertySOCKSVersion, kCFStreamSocketSOCKSVersion4);
    }
}

IceObjC::Instance::~Instance()
{
    if(_proxySettings)
    {
        CFRelease(_proxySettings);
    }
}

void
IceObjC::Instance::setupStreams(CFReadStreamRef readStream,
                                CFWriteStreamRef writeStream,
                                bool server,
                                const string& host) const
{
    if(_voip)
    {
#if TARGET_IPHONE_SIMULATOR == 0
        if(!CFReadStreamSetProperty(readStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP) ||
           !CFWriteStreamSetProperty(writeStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP))
        {
            throw Ice::SyscallException(__FILE__, __LINE__);
        }
#endif
    }

    if(!server && _proxySettings)
    {
        if(!CFReadStreamSetProperty(readStream, kCFStreamPropertySOCKSProxy, _proxySettings) ||
           !CFWriteStreamSetProperty(writeStream, kCFStreamPropertySOCKSProxy, _proxySettings))
        {
            throw Ice::SyscallException(__FILE__, __LINE__);
        }
    }
}

IceObjC::Instance*
IceObjC::Instance::clone(const ProtocolInstancePtr& instance)
{
    return new Instance(_communicator, instance->type(), instance->protocol(), instance->secure());
}

IceObjC::StreamEndpointI::StreamEndpointI(const InstancePtr& instance, const string& ho, Int po,
                                          const Address& sourceAddr, Int ti, const string& conId, bool co) :
    IceInternal::IPEndpointI(instance, ho, po, sourceAddr, conId),
    _instance(instance),
    _timeout(ti),
    _compress(co)
{
}

IceObjC::StreamEndpointI::StreamEndpointI(const InstancePtr& instance) :
    IceInternal::IPEndpointI(instance),
    _instance(instance),
    _timeout(instance->defaultTimeout()),
    _compress(false)
{
}

IceObjC::StreamEndpointI::StreamEndpointI(const InstancePtr& instance, Ice::InputStream* s) :
    IPEndpointI(instance, s),
    _instance(instance),
    _timeout(-1),
    _compress(false)
{
    s->read(const_cast<Int&>(_timeout));
    s->read(const_cast<bool&>(_compress));
}

EndpointInfoPtr
IceObjC::StreamEndpointI::getInfo() const
{
    TCPEndpointInfoPtr info = ICE_MAKE_SHARED(InfoI<Ice::TCPEndpointInfo>, shared_from_this());
    IPEndpointI::fillEndpointInfo(info.get());
    info->timeout = _timeout;
    info->compress = _compress;
    return info;
}

Int
IceObjC::StreamEndpointI::timeout() const
{
    return _timeout;
}

EndpointIPtr
IceObjC::StreamEndpointI::timeout(Int t) const
{
    if(t == _timeout)
    {
        return shared_from_this();
    }
    else
    {
        return ICE_MAKE_SHARED(StreamEndpointI, _instance, _host, _port, _sourceAddr, t, _connectionId, _compress);
    }
}

bool
IceObjC::StreamEndpointI::compress() const
{
    return _compress;
}

EndpointIPtr
IceObjC::StreamEndpointI::compress(bool c) const
{
    if(c == _compress)
    {
        return shared_from_this();
    }
    else
    {
        return ICE_MAKE_SHARED(StreamEndpointI, _instance, _host, _port, _sourceAddr, _timeout, _connectionId, c);
    }
}

bool
IceObjC::StreamEndpointI::datagram() const
{
    return false;
}

bool
IceObjC::StreamEndpointI::secure() const
{
    return _instance->secure();
}

void
IceObjC::StreamEndpointI::connectors_async(Ice::EndpointSelectionType selType, const EndpointI_connectorsPtr& cb) const
{
    vector<ConnectorPtr> connectors;
    connectors.push_back(new StreamConnector(_instance, _host, _port, _timeout, _connectionId));
    cb->connectors(connectors);
}

TransceiverPtr
IceObjC::StreamEndpointI::transceiver() const
{
    return 0;
}

AcceptorPtr
IceObjC::StreamEndpointI::acceptor(const string&) const
{
    return new StreamAcceptor(shared_from_this(), _instance, _host, _port);
}

IceObjC::StreamEndpointIPtr
IceObjC::StreamEndpointI::endpoint(const StreamAcceptorPtr& a) const
{
    return ICE_MAKE_SHARED(StreamEndpointI, _instance, _host, a->effectivePort(), _sourceAddr, _timeout, _connectionId,
                           _compress);
}

string
IceObjC::StreamEndpointI::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;

    s << IPEndpointI::options();

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

bool
#ifdef ICE_CPP11_MAPPING
IceObjC::StreamEndpointI::operator==(const Endpoint& r) const
#else
IceObjC::StreamEndpointI::operator==(const LocalObject& r) const
#endif
{
    if(!IPEndpointI::operator==(r))
    {
        return false;
    }

    const StreamEndpointI* p = dynamic_cast<const StreamEndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
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
IceObjC::StreamEndpointI::operator<(const Endpoint& r) const
#else
IceObjC::StreamEndpointI::operator<(const LocalObject& r) const
#endif
{
    const StreamEndpointI* p = dynamic_cast<const StreamEndpointI*>(&r);
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

    return IPEndpointI::operator<(r);
}

void
IceObjC::StreamEndpointI::streamWriteImpl(Ice::OutputStream* s) const
{
    IPEndpointI::streamWriteImpl(s);
    s->write(_timeout);
    s->write(_compress);
}

void
IceObjC::StreamEndpointI::hashInit(Ice::Int& h) const
{
    IPEndpointI::hashInit(h);
    hashAdd(h, _timeout);
    hashAdd(h, _compress);
}

bool
IceObjC::StreamEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    if(IPEndpointI::checkOption(option, argument, endpoint))
    {
        return true;
    }

    switch(option[1])
    {
    case 't':
    {
        if(argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -t option in endpoint " + endpoint;
            throw ex;
        }

        if(argument == "infinite")
        {
            const_cast<Int&>(_timeout) = -1;
        }
        else
        {
            istringstream t(argument);
            if(!(t >> const_cast<Int&>(_timeout)) || !t.eof() || _timeout < 1)
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "invalid timeout value `" + argument + "' in endpoint " + endpoint;
                throw ex;
            }
        }
        return true;
    }

    case 'z':
    {
        if(!argument.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "unexpected argument `" + argument + "' provided for -z option in " + endpoint;
            throw ex;
        }
        const_cast<bool&>(_compress) = true;
        return true;
    }

    default:
    {
        return false;
    }
    }
}

ConnectorPtr
IceObjC::StreamEndpointI::createConnector(const Address& address, const NetworkProxyPtr& proxy) const
{
    assert(false);
    return 0;
}

IPEndpointIPtr
IceObjC::StreamEndpointI::createEndpoint(const string& host, int port, const string& connectionId) const
{
    return ICE_MAKE_SHARED(StreamEndpointI, _instance, host, port, _sourceAddr, _timeout, connectionId, _compress);
}

IceObjC::StreamEndpointFactory::StreamEndpointFactory(const InstancePtr& instance) : _instance(instance)
{
}

IceObjC::StreamEndpointFactory::~StreamEndpointFactory()
{
}

Short
IceObjC::StreamEndpointFactory::type() const
{
    return _instance->type();
}

string
IceObjC::StreamEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceObjC::StreamEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    IPEndpointIPtr endpt = ICE_MAKE_SHARED(StreamEndpointI, _instance);
    endpt->initWithOptions(args, oaEndpoint);
    return endpt;
}

EndpointIPtr
IceObjC::StreamEndpointFactory::read(Ice::InputStream* s) const
{
    return ICE_MAKE_SHARED(StreamEndpointI, _instance, s);
}

void
IceObjC::StreamEndpointFactory::destroy()
{
    _instance = 0;
}

EndpointFactoryPtr
IceObjC::StreamEndpointFactory::clone(const ProtocolInstancePtr& instance, const EndpointFactoryPtr&) const
{
    return new StreamEndpointFactory(_instance->clone(instance));
}
