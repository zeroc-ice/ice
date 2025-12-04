// Copyright (c) ZeroC, Inc.

#include "NetworkProxy.h"
#include "HttpParser.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Properties.h"

#include <sstream>

using namespace std;
using namespace IceInternal;

NetworkProxy::~NetworkProxy() = default; // Out of line to avoid weak vtable

namespace
{
    class SOCKSNetworkProxy final : public NetworkProxy
    {
    public:
        SOCKSNetworkProxy(string, int);
        SOCKSNetworkProxy(const Address&);

        void beginWrite(const Address&, Buffer&) final;
        SocketOperation endWrite(Buffer&) final;
        void beginRead(Buffer&) final;
        SocketOperation endRead(Buffer&) final;
        void finish(Buffer&, Buffer&) final;
        [[nodiscard]] NetworkProxyPtr resolveHost(ProtocolSupport) const final;
        [[nodiscard]] Address getAddress() const final;
        [[nodiscard]] string getName() const final;
        [[nodiscard]] ProtocolSupport getProtocolSupport() const final;

    private:
        string _host;
        int _port;
        Address _address;
    };

    class HTTPNetworkProxy final : public NetworkProxy
    {
    public:
        HTTPNetworkProxy(string, int);
        HTTPNetworkProxy(const Address&, ProtocolSupport);

        void beginWrite(const Address&, Buffer&) final;
        SocketOperation endWrite(Buffer&) final;
        void beginRead(Buffer&) final;
        SocketOperation endRead(Buffer&) final;
        void finish(Buffer&, Buffer&) final;
        [[nodiscard]] NetworkProxyPtr resolveHost(ProtocolSupport) const final;
        [[nodiscard]] Address getAddress() const final;
        [[nodiscard]] string getName() const final;
        [[nodiscard]] ProtocolSupport getProtocolSupport() const final;

    private:
        string _host;
        int _port;
        Address _address;
        ProtocolSupport _protocol;
    };
}

SOCKSNetworkProxy::SOCKSNetworkProxy(string host, int port) : _host(std::move(host)), _port(port)
{
    assert(!_host.empty());
}

SOCKSNetworkProxy::SOCKSNetworkProxy(const Address& addr) : _port(0), _address(addr) {}

void
SOCKSNetworkProxy::beginWrite(const Address& addr, Buffer& buf)
{
    //
    // SOCKS connect request
    //
    buf.b.resize(9);
    buf.i = buf.b.begin();
    byte* dest = &buf.b[0];
    *dest++ = byte{0x04}; // SOCKS version 4.
    *dest++ = byte{0x01}; // Command, establish a TCP/IP stream connection

    const byte* src;

    //
    // Port (already in big-endian order)
    //
    src = reinterpret_cast<const byte*>(&addr.saIn.sin_port);
    *dest++ = *src++;
    *dest++ = *src;

    //
    // IPv4 address (already in big-endian order)
    //
    src = reinterpret_cast<const byte*>(&addr.saIn.sin_addr.s_addr);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src;

    *dest = byte{0x00}; // User ID.
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

    if (readBuffer.b.end() - readBuffer.i < 2)
    {
        throw Ice::MarshalException{__FILE__, __LINE__, "attempting to unmarshal past the end of the buffer"};
    }

    const byte* src = &(*readBuffer.i);
    const byte b1 = *src++;
    const byte b2 = *src++;
    if (b1 != byte{0x00} || b2 != byte{0x5a})
    {
        throw Ice::ConnectFailedException{
            __FILE__,
            __LINE__,
            "connection establishment failed due to an HTTP proxy error"};
    }
}

NetworkProxyPtr
SOCKSNetworkProxy::resolveHost(ProtocolSupport protocol) const
{
    assert(!_host.empty());
    return make_shared<SOCKSNetworkProxy>(getAddresses(_host, _port, protocol, false, true)[0]);
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

HTTPNetworkProxy::HTTPNetworkProxy(string host, int port) : _host(std::move(host)), _port(port), _protocol(EnableBoth)
{
    assert(!_host.empty());
}

HTTPNetworkProxy::HTTPNetworkProxy(const Address& addr, ProtocolSupport protocol)
    : _port(0),
      _address(addr),
      _protocol(protocol)
{
}

void
HTTPNetworkProxy::beginWrite(const Address& addr, Buffer& buf)
{
    //
    // HTTP connect request
    //
    ostringstream out;
    out << "CONNECT " << addrToString(addr) << " HTTP/1.1\r\n"
        << "Host: " << addrToString(addr) << "\r\n\r\n";
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
    const byte* end = HttpParser().isCompleteMessage(buf.b.begin(), buf.i);
    if (!end && buf.i == buf.b.end())
    {
        //
        // Read one more byte, we can't easily read bytes in advance
        // since the transport implementation might be be able to read
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
    if (parser.status() != 200)
    {
        throw Ice::ConnectFailedException{
            __FILE__,
            __LINE__,
            "connection establishment failed due to an HTTP proxy error: " + to_string(parser.status())};
    }
}

NetworkProxyPtr
HTTPNetworkProxy::resolveHost(ProtocolSupport protocol) const
{
    assert(!_host.empty());
    return make_shared<HTTPNetworkProxy>(getAddresses(_host, _port, protocol, false, true)[0], protocol);
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

NetworkProxyPtr
IceInternal::createNetworkProxy(const Ice::PropertiesPtr& properties, ProtocolSupport protocolSupport)
{
    string proxyHost;

    proxyHost = properties->getIceProperty("Ice.SOCKSProxyHost");
    if (!proxyHost.empty())
    {
        if (protocolSupport == EnableIPv6)
        {
            throw Ice::InitializationException(__FILE__, __LINE__, "IPv6 only is not supported with SOCKS4 proxies");
        }
        int proxyPort = properties->getIcePropertyAsInt("Ice.SOCKSProxyPort");
        return make_shared<SOCKSNetworkProxy>(proxyHost, proxyPort);
    }

    proxyHost = properties->getIceProperty("Ice.HTTPProxyHost");
    if (!proxyHost.empty())
    {
        return make_shared<HTTPNetworkProxy>(proxyHost, properties->getIcePropertyAsInt("Ice.HTTPProxyPort"));
    }

    return nullptr;
}
