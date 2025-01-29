// Copyright (c) ZeroC, Inc.

#include "SecureTransportTransceiverI.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/SSL/ConnectionInfo.h"
#include "SSLInstance.h"
#include "SecureTransportEngine.h"
#include "SecureTransportUtil.h"

#include <sstream>

// Disable deprecation warnings from SecureTransport APIs
#include "../DisableWarnings.h"

using namespace std;
using namespace Ice;
using namespace Ice::SSL;
using namespace Ice::SSL::SecureTransport;
using namespace IceInternal;

namespace
{
    string protocolName(SSLProtocol protocol)
    {
        switch (protocol)
        {
            case kSSLProtocol2:
                return "SSL 2.0";
            case kSSLProtocol3:
                return "SSL 3.0";
            case kTLSProtocol1:
                return "TLS 1.0";
            case kTLSProtocol11:
                return "TLS 1.1";
            case kTLSProtocol12:
                return "TLS 1.2";
            case kTLSProtocol13:
                return "TLS 1.3";
            default:
                return "Unknown";
        }
    }

    //
    // Socket write callback
    //
    OSStatus socketWrite(SSLConnectionRef connection, const void* data, size_t* length)
    {
        const auto* transceiver = static_cast<const TransceiverI*>(connection);
        assert(transceiver);
        return transceiver->writeRaw(reinterpret_cast<const byte*>(data), length);
    }

    //
    // Socket read callback
    //
    OSStatus socketRead(SSLConnectionRef connection, void* data, size_t* length)
    {
        const auto* transceiver = static_cast<const TransceiverI*>(connection);
        assert(transceiver);
        return transceiver->readRaw(reinterpret_cast<byte*>(data), length);
    }
}

IceInternal::NativeInfoPtr
Ice::SSL::SecureTransport::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
Ice::SSL::SecureTransport::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
    if (!_connected)
    {
        IceInternal::SocketOperation status = _delegate->initialize(readBuffer, writeBuffer);
        if (status != IceInternal::SocketOperationNone)
        {
            return status;
        }
        _connected = true;
    }

    //
    // Limit the size of packets passed to SSLWrite/SSLRead to avoid
    // blocking and holding too much memory.
    //
    if (_delegate->getNativeInfo()->fd() != INVALID_SOCKET)
    {
        _maxSendPacketSize =
            static_cast<size_t>(std::max(512, IceInternal::getSendBufferSize(_delegate->getNativeInfo()->fd())));
        _maxRecvPacketSize =
            static_cast<size_t>(std::max(512, IceInternal::getRecvBufferSize(_delegate->getNativeInfo()->fd())));
    }
    else
    {
        _maxSendPacketSize = 128 * 1024; // 128KB
        _maxRecvPacketSize = 128 * 1024; // 128KB
    }

    OSStatus err = 0;
    if (!_ssl)
    {
        // Initialize SSL context
        _ssl.reset(_engine->newContext(_incoming));

        // Enable SNI by default for outgoing connections. The SNI host name is always empty for incoming connections.
        if (!_host.empty() && !IceInternal::isIpAddress(_host) &&
            (err = SSLSetPeerDomainName(_ssl.get(), _host.data(), _host.length())))
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                "SSL transport: setting SNI host failed '" + _host + "'\n" + sslErrorToString(err));
        }

        if (_sslNewSessionCallback)
        {
            _sslNewSessionCallback(_ssl.get(), _incoming ? _adapterName : _host);
        }

        if (_incoming)
        {
            SSLSetClientSideAuthenticate(_ssl.get(), _clientCertificateRequired);
        }

        if (_localCertificateSelectionCallback)
        {
            _certificates = _localCertificateSelectionCallback(_incoming ? _adapterName : _host);
            if (_certificates)
            {
                err = SSLSetCertificate(_ssl.get(), _certificates);
                if (err)
                {
                    throw SecurityException(
                        __FILE__,
                        __LINE__,
                        "SSL transport: error while setting the SSL context certificate:\n" + sslErrorToString(err));
                }
            }
        }

        if ((err = SSLSetIOFuncs(_ssl.get(), socketRead, socketWrite)))
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                "SSL transport: setting IO functions failed\n" + sslErrorToString(err));
        }

        if ((err = SSLSetConnection(_ssl.get(), reinterpret_cast<SSLConnectionRef>(this))))
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                "SSL transport: setting SSL connection failed\n" + sslErrorToString(err));
        }
    }

    SSLSessionState state;
    SSLGetSessionState(_ssl.get(), &state);

    //
    // SSL Handshake
    //
    while (state == kSSLHandshake || state == kSSLIdle)
    {
        err = SSLHandshake(_ssl.get());
        if (err == noErr)
        {
            break; // We're done!
        }
        else if (err == errSSLWouldBlock)
        {
            assert(_tflags & SSLWantRead || _tflags & SSLWantWrite);
            return _tflags & SSLWantRead ? IceInternal::SocketOperationRead : IceInternal::SocketOperationWrite;
        }
        else if (err == errSSLPeerAuthCompleted)
        {
            assert(!_trust);
            err = SSLCopyPeerTrust(_ssl.get(), &_trust.get());

            if (err == noErr)
            {
                if (_incoming &&
                    (_clientCertificateRequired == kTryAuthenticate ||
                     _clientCertificateRequired == kNeverAuthenticate) &&
                    !_trust)
                {
                    // This is expected if the client doesn't provide a certificate.
                    continue; // Call SSLHandshake to resume the handshake.
                }
                else if (_trust)
                {
                    if (SecTrustGetCertificateCount(_trust.get()) > 0)
                    {
                        SecCertificateRef peerCertificate = SecTrustGetCertificateAtIndex(_trust.get(), 0);
                        CFRetain(peerCertificate);
                        _peerCertificate.reset(peerCertificate);
                    }

                    if (_trustedRootCertificates)
                    {
                        if ((err = SecTrustSetAnchorCertificates(_trust.get(), _trustedRootCertificates)))
                        {
                            throw SecurityException(
                                __FILE__,
                                __LINE__,
                                "SSL transport: handshake failure:\n" + sslErrorToString(err));
                        }

                        if ((err = SecTrustSetAnchorCertificatesOnly(_trust.get(), true)))
                        {
                            throw SecurityException(
                                __FILE__,
                                __LINE__,
                                "SSL transport: handshake failure:\n" + sslErrorToString(err));
                        }
                    }

                    function<bool(SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr& info)>
                        remoteCertificateValidationCallback =
                            _remoteCertificateValidationCallback
                                ? _remoteCertificateValidationCallback
                                : [this](SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr& info)
                    { return _engine->validationCallback(trust, info, _incoming ? _adapterName : _host); };

                    if (remoteCertificateValidationCallback(
                            _trust.get(),
                            dynamic_pointer_cast<ConnectionInfo>(getInfo(_incoming, _adapterName, ""))))
                    {
                        continue; // Call SSLHandshake to resume the handshake.
                    }
                    else
                    {
                        throw SecurityException(
                            __FILE__,
                            __LINE__,
                            "IceSSL: certificate verification failed. The certificate was rejected by the certificate "
                            "validation callback.");
                    }
                }
                else
                {
                    continue; // Call SSLHandshake to resume the handshake.
                }
            }
            // Let it fall through, this will raise a SecurityException with the SSLCopyPeerTrust error.
        }
        else if (err == errSSLClosedGraceful || err == errSSLClosedAbort)
        {
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }

        ostringstream os;
        os << "SSL transport: ssl error occurred for new " << (_incoming ? "incoming" : "outgoing") << " connection:\n"
           << _delegate->toString() << "\n"
           << sslErrorToString(err);
        throw ProtocolException(__FILE__, __LINE__, os.str());
    }

    assert(_ssl);

    if (_instance->engine()->securityTraceLevel() >= 1)
    {
        Trace out(_instance->logger(), _engine->securityTraceCategory());
        out << "SSL summary for " << (_incoming ? "incoming" : "outgoing") << " connection\n";

        SSLProtocol protocol;
        SSLGetNegotiatedProtocolVersion(_ssl.get(), &protocol);
        const string sslProtocolName = protocolName(protocol);

        SSLCipherSuite cipher;
        SSLGetNegotiatedCipher(_ssl.get(), &cipher);
        const string sslCipherName = _engine->getCipherName(cipher);

        if (sslCipherName.empty())
        {
            out << "unknown cipher\n";
        }
        else
        {
            out << "cipher = " << sslCipherName << "\n";
            out << "protocol = " << sslProtocolName << "\n";
        }
        out << toString();
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
Ice::SSL::SecureTransport::TransceiverI::closing(bool initiator, exception_ptr)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
Ice::SSL::SecureTransport::TransceiverI::close()
{
    if (_trust)
    {
        _trust.reset(nullptr);
    }

    if (_ssl)
    {
        SSLClose(_ssl.get());
        _ssl.reset(nullptr);
    }

    if (_certificates)
    {
        CFRelease(_certificates);
        _certificates = nullptr;
    }

    _delegate->close();
}

IceInternal::SocketOperation
Ice::SSL::SecureTransport::TransceiverI::write(IceInternal::Buffer& buf)
{
    if (!_connected)
    {
        return _delegate->write(buf);
    }

    if (buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    size_t packetSize = std::min(static_cast<size_t>(buf.b.end() - buf.i), _maxSendPacketSize);
    while (buf.i != buf.b.end())
    {
        size_t processed = 0;
        OSStatus err = _buffered ? SSLWrite(_ssl.get(), nullptr, 0, &processed)
                                 : SSLWrite(_ssl.get(), reinterpret_cast<const void*>(buf.i), packetSize, &processed);

        if (err)
        {
            if (err == errSSLWouldBlock)
            {
                if (_buffered == 0)
                {
                    _buffered = processed;
                }
                assert(_tflags & SSLWantWrite);
                return IceInternal::SocketOperationWrite;
            }

            if (err == errSSLClosedGraceful)
            {
                throw ConnectionLostException(__FILE__, __LINE__, 0);
            }

            //
            // SSL protocol errors are defined in SecureTransport.h are in the range
            // -9800 to -9849
            //
            if (err <= -9800 && err >= -9849)
            {
                throw ProtocolException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: error during write:\n" + sslErrorToString(err));
            }

            errno = err;
            if (IceInternal::connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
        }

        if (_buffered)
        {
            buf.i += _buffered;
            _buffered = 0;
        }
        else
        {
            buf.i += processed;
        }

        if (packetSize > static_cast<size_t>(buf.b.end() - buf.i))
        {
            packetSize = static_cast<size_t>(buf.b.end() - buf.i);
        }
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
Ice::SSL::SecureTransport::TransceiverI::read(IceInternal::Buffer& buf)
{
    if (!_connected)
    {
        return _delegate->read(buf);
    }

    if (buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }

    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);

    size_t packetSize = std::min(static_cast<size_t>(buf.b.end() - buf.i), _maxRecvPacketSize);
    while (buf.i != buf.b.end())
    {
        size_t processed = 0;
        OSStatus err = SSLRead(_ssl.get(), reinterpret_cast<void*>(buf.i), packetSize, &processed);
        if (err)
        {
            if (err == errSSLWouldBlock)
            {
                buf.i += processed;
                assert(_tflags & SSLWantRead);
                return IceInternal::SocketOperationRead;
            }

            if (err == errSSLClosedGraceful || err == errSSLClosedAbort)
            {
                throw ConnectionLostException(__FILE__, __LINE__, 0);
            }

            //
            // SSL protocol errors are defined in SecureTransport.h are in the range
            // -9800 to -9849
            //
            if (err <= -9800 && err >= -9849)
            {
                throw ProtocolException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: error during read:\n" + sslErrorToString(err));
            }

            errno = err;
            if (IceInternal::connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
        }

        buf.i += processed;

        if (packetSize > static_cast<size_t>(buf.b.end() - buf.i))
        {
            packetSize = static_cast<size_t>(buf.b.end() - buf.i);
        }
    }

    //
    // Check if there's still buffered data to read. In this case, set the read ready status.
    //
    size_t buffered = 0;
    OSStatus err = SSLGetBufferedReadSize(_ssl.get(), &buffered);
    if (err)
    {
        errno = err;
        throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
    }
    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, buffered > 0);
    return IceInternal::SocketOperationNone;
}

string
Ice::SSL::SecureTransport::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
Ice::SSL::SecureTransport::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
Ice::SSL::SecureTransport::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
Ice::SSL::SecureTransport::TransceiverI::getInfo(bool incoming, string adapterName, string connectionId) const
{
    assert(incoming == _incoming);
    // adapterName is the name of the object adapter currently associated with this connection, while _adapterName
    // represents the name of the object adapter that created this connection (incoming only).

    SecCertificateRef peerCertificate = nullptr;

    if (_peerCertificate)
    {
        peerCertificate = _peerCertificate.get();
        CFRetain(peerCertificate);
    }

    return make_shared<Ice::SSL::ConnectionInfo>(
        _delegate->getInfo(incoming, std::move(adapterName), std::move(connectionId)),
        peerCertificate);
}

void
Ice::SSL::SecureTransport::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
Ice::SSL::SecureTransport::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

Ice::SSL::SecureTransport::TransceiverI::TransceiverI(
    const Ice::SSL::InstancePtr& instance,
    IceInternal::TransceiverPtr delegate,
    string adapterName,
    const ServerAuthenticationOptions& serverAuthenticationOptions)
    : _instance(instance),
      _engine(dynamic_pointer_cast<Ice::SSL::SecureTransport::SSLEngine>(instance->engine())),
      _host(""),
      _adapterName(std::move(adapterName)),
      _incoming(true),
      _delegate(std::move(delegate)),
      _connected(false),
      _buffered(0),
      _sslNewSessionCallback(serverAuthenticationOptions.sslNewSessionCallback),
      _remoteCertificateValidationCallback(serverAuthenticationOptions.clientCertificateValidationCallback),
      _localCertificateSelectionCallback(serverAuthenticationOptions.serverCertificateSelectionCallback),
      _clientCertificateRequired(serverAuthenticationOptions.clientCertificateRequired),
      _certificates(nullptr),
      _trustedRootCertificates(serverAuthenticationOptions.trustedRootCertificates)
{
}

Ice::SSL::SecureTransport::TransceiverI::TransceiverI(
    const Ice::SSL::InstancePtr& instance,
    IceInternal::TransceiverPtr delegate,
    string host,
    const ClientAuthenticationOptions& clientAuthenticationOptions)
    : _instance(instance),
      _engine(dynamic_pointer_cast<Ice::SSL::SecureTransport::SSLEngine>(instance->engine())),
      _host(std::move(host)),
      _adapterName(""),
      _incoming(false),
      _delegate(std::move(delegate)),
      _connected(false),
      _buffered(0),
      _sslNewSessionCallback(clientAuthenticationOptions.sslNewSessionCallback),
      _remoteCertificateValidationCallback(clientAuthenticationOptions.serverCertificateValidationCallback),
      _localCertificateSelectionCallback(clientAuthenticationOptions.clientCertificateSelectionCallback),
      _clientCertificateRequired(kNeverAuthenticate),
      _certificates(nullptr),
      _trustedRootCertificates(clientAuthenticationOptions.trustedRootCertificates)
{
}

Ice::SSL::SecureTransport::TransceiverI::~TransceiverI() = default;

OSStatus
Ice::SSL::SecureTransport::TransceiverI::writeRaw(const byte* data, size_t* length) const
{
    _tflags &= ~SSLWantWrite;

    try
    {
        IceInternal::Buffer buf(data, data + *length);
        IceInternal::SocketOperation op = _delegate->write(buf);
        if (op == IceInternal::SocketOperationWrite)
        {
            *length = static_cast<size_t>(buf.i - buf.b.begin());
            _tflags |= SSLWantWrite;
            return errSSLWouldBlock;
        }
        assert(op == IceInternal::SocketOperationNone);
    }
    catch (const Ice::ConnectionLostException&)
    {
        return errSSLClosedGraceful;
    }
    catch (const Ice::SocketException& ex)
    {
        return ex.error();
    }
    catch (...)
    {
        assert(false);
        return IceInternal::getSocketErrno();
    }
    return noErr;
}

OSStatus
Ice::SSL::SecureTransport::TransceiverI::readRaw(byte* data, size_t* length) const
{
    _tflags &= ~SSLWantRead;

    try
    {
        IceInternal::Buffer buf(data, data + *length);
        IceInternal::SocketOperation op = _delegate->read(buf);
        if (op == IceInternal::SocketOperationRead)
        {
            *length = static_cast<size_t>(buf.i - buf.b.begin());
            _tflags |= SSLWantRead;
            return errSSLWouldBlock;
        }
        assert(op == IceInternal::SocketOperationNone);
    }
    catch (const Ice::ConnectionLostException&)
    {
        return errSSLClosedGraceful;
    }
    catch (const Ice::SocketException& ex)
    {
        return ex.error();
    }
    catch (...)
    {
        assert(false);
        return IceInternal::getSocketErrno();
    }
    return noErr;
}
