// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/NetworkProxy.h>
#include <Ice/HttpParser.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>

using namespace std;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(NetworkProxy* p) { return p; }

#ifndef ICE_OS_WINRT

namespace
{

class SOCKSNetworkProxy : public NetworkProxy
{
public:

    SOCKSNetworkProxy(const string&, int);
    SOCKSNetworkProxy(const Address&);

    virtual void beginWrite(const Address&, Buffer&);
    virtual SocketOperation endWrite(Buffer&);
    virtual void beginRead(Buffer&);
    virtual SocketOperation endRead(Buffer&);
    virtual void finish(Buffer&, Buffer&);
    virtual NetworkProxyPtr resolveHost(ProtocolSupport) const;
    virtual Address getAddress() const;
    virtual string getName() const;
    virtual ProtocolSupport getProtocolSupport() const;

private:

    string _host;
    int _port;
    Address _address;
};

class HTTPNetworkProxy : public NetworkProxy
{
public:

    HTTPNetworkProxy(const string&, int);
    HTTPNetworkProxy(const Address&, ProtocolSupport);

    virtual void beginWrite(const Address&, Buffer&);
    virtual SocketOperation endWrite(Buffer&);
    virtual void beginRead(Buffer&);
    virtual SocketOperation endRead(Buffer&);
    virtual void finish(Buffer&, Buffer&);
    virtual NetworkProxyPtr resolveHost(ProtocolSupport) const;
    virtual Address getAddress() const;
    virtual string getName() const;
    virtual ProtocolSupport getProtocolSupport() const;

private:

    string _host;
    int _port;
    Address _address;
    ProtocolSupport _protocol;
};

}

SOCKSNetworkProxy::SOCKSNetworkProxy(const string& host, int port) : _host(host), _port(port)
{
    assert(!host.empty());
    memset(&_address, 0, sizeof(_address));
}

SOCKSNetworkProxy::SOCKSNetworkProxy(const Address& addr) : _port(0), _address(addr)
{
}

void
SOCKSNetworkProxy::beginWrite(const Address& addr, Buffer& buf)
{
    //
    // SOCKS connect request
    //
    buf.b.resize(9);
    buf.i = buf.b.begin();
    Ice::Byte* dest = &buf.b[0];
    *dest++ = 0x04; // SOCKS version 4.
    *dest++ = 0x01; // Command, establish a TCP/IP stream connection

    const Ice::Byte* src;

    //
    // Port (already in big-endian order)
    //
    src = reinterpret_cast<const Ice::Byte*>(&addr.saIn.sin_port);
    *dest++ = *src++;
    *dest++ = *src;

    //
    // IPv4 address (already in big-endian order)
    //
    src = reinterpret_cast<const Ice::Byte*>(&addr.saIn.sin_addr.s_addr);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src;

    *dest = 0x00; // User ID.
}

SocketOperation
SOCKSNetworkProxy::endWrite(Buffer& buf)
{
    // Once the request is sent, read the response 
    return buf.i != buf.b.end() ? SocketOperationWrite : SocketOperationRead;
}

void
SOCKSNetworkProxy::beginRead(Buffer& buf)
{
    //
    // Read the SOCKS4 response whose size is 8 bytes.
    //
    buf.b.resize(8);
    buf.i = buf.b.begin();
}

SocketOperation
SOCKSNetworkProxy::endRead(Buffer& buf)
{
    // We're done once we read the response
    return buf.i != buf.b.end() ? SocketOperationRead : SocketOperationNone;
}

void
SOCKSNetworkProxy::finish(Buffer& readBuffer, Buffer&)
{
    readBuffer.i = readBuffer.b.begin();

    if(readBuffer.b.end() - readBuffer.i < 2)
    {
        throw Ice::UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }

    const Ice::Byte* src = &(*readBuffer.i);
    const Ice::Byte b1 = *src++;
    const Ice::Byte b2 = *src++;
    if(b1 != 0x00 || b2 != 0x5a)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__);
    }
}

NetworkProxyPtr
SOCKSNetworkProxy::resolveHost(ProtocolSupport protocol) const
{
    assert(!_host.empty());
    return new SOCKSNetworkProxy(getAddresses(_host, _port, protocol, Ice::Random, false, true)[0]);
}

Address
SOCKSNetworkProxy::getAddress() const
{
    assert(_host.empty()); // Host must be resolved.
    return _address;
}

string
SOCKSNetworkProxy::getName() const
{
    return "SOCKS";
}

ProtocolSupport
SOCKSNetworkProxy::getProtocolSupport() const
{
    return EnableIPv4;
}

HTTPNetworkProxy::HTTPNetworkProxy(const string& host, int port) : 
    _host(host), _port(port), _protocol(EnableBoth)
{
    assert(!host.empty());
    memset(&_address, 0, sizeof(_address));
}

HTTPNetworkProxy::HTTPNetworkProxy(const Address& addr, ProtocolSupport protocol) : 
    _port(0), _address(addr), _protocol(protocol)
{
}

void
HTTPNetworkProxy::beginWrite(const Address& addr, Buffer& buf)
{
    //
    // HTTP connect request
    //
    ostringstream out;
    out << "CONNECT " << addrToString(addr) << " HTTP/1.1\r\n" << "Host: " << addrToString(addr) << "\r\n\r\n";
    string str = out.str();
    buf.b.resize(str.size());
    memcpy(&buf.b[0], str.c_str(), str.size());
    buf.i = buf.b.begin();
}

SocketOperation
HTTPNetworkProxy::endWrite(Buffer& buf)
{
    // Once the request is sent, read the response 
    return buf.i != buf.b.end() ? SocketOperationWrite : SocketOperationRead;
}

void
HTTPNetworkProxy::beginRead(Buffer& buf)
{
    //
    // Read the Http response
    //
    buf.b.resize(7); // Enough space for reading at least HTTP1.1
    buf.i = buf.b.begin();
}

SocketOperation
HTTPNetworkProxy::endRead(Buffer& buf)
{
    //
    // Check if we received the full HTTP response, if not, continue
    // reading otherwise we're done.
    //
    const Ice::Byte* end = HttpParser().isCompleteMessage(buf.b.begin(), buf.i);
    if(!end && buf.i == buf.b.end())
    {
        //
        // Read one more byte, we can't easily read bytes in advance
        // since the transport implenentation might be be able to read
        // the data from the memory instead of the socket. This is for
        // instance the case with the OpenSSL transport (or we would 
        // have to use a buffering BIO).
        //
        buf.b.resize(buf.b.size() + 1);
        buf.i = buf.b.begin() + buf.b.size() - 1;
        return SocketOperationRead;
    }
    return SocketOperationNone;
}

void
HTTPNetworkProxy::finish(Buffer& readBuffer, Buffer&)
{
    HttpParser parser;
    parser.parse(readBuffer.b.begin(), readBuffer.b.end());
    if(parser.status() != 200)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__);
    }
}

NetworkProxyPtr
HTTPNetworkProxy::resolveHost(ProtocolSupport protocol) const
{
    assert(!_host.empty());
    return new HTTPNetworkProxy(getAddresses(_host, _port, protocol, Ice::Random, false, true)[0], protocol);
}

Address
HTTPNetworkProxy::getAddress() const
{
    assert(_host.empty()); // Host must be resolved.
    return _address;
}

string
HTTPNetworkProxy::getName() const
{
    return "HTTP";
}

ProtocolSupport
HTTPNetworkProxy::getProtocolSupport() const
{
    return _protocol;
}

#endif

NetworkProxyPtr
IceInternal::createNetworkProxy(const Ice::PropertiesPtr& properties, ProtocolSupport protocolSupport)
{
    string proxyHost;

    proxyHost = properties->getProperty("Ice.SOCKSProxyHost");
    if(!proxyHost.empty())
    {
#ifdef ICE_OS_WINRT
        throw Ice::InitializationException(__FILE__, __LINE__, "SOCKS proxy not supported with WinRT");
#else
        if(protocolSupport == EnableIPv6)
        {
            throw Ice::InitializationException(__FILE__, __LINE__, "IPv6 only is not supported with SOCKS4 proxies");
        }
        int proxyPort = properties->getPropertyAsIntWithDefault("Ice.SOCKSProxyPort", 1080);
        return new SOCKSNetworkProxy(proxyHost, proxyPort);
#endif
    }

    proxyHost = properties->getProperty("Ice.HTTPProxyHost");
    if(!proxyHost.empty())
    {
#ifdef ICE_OS_WINRT
        throw Ice::InitializationException(__FILE__, __LINE__, "HTTP proxy not supported with WinRT");
#else
        return new HTTPNetworkProxy(proxyHost, properties->getPropertyAsIntWithDefault("Ice.HTTPProxyPort", 1080));
#endif
    }
    
    return 0;
}
