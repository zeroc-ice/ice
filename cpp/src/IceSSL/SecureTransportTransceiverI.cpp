//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SecureTransportTransceiverI.h"
#include "Ice/LocalException.h"
#include "Ice/LoggerUtil.h"
#include "Ice/SSLConnectionInfo.h"
#include "SSLInstance.h"
#include "SecureTransportEngine.h"
#include "SecureTransportUtil.h"

// Disable deprecation warnings from SecureTransport APIs
#include "IceUtil/DisableWarnings.h"

using namespace std;
using namespace Ice;
using namespace Ice::SSL;
using namespace IceInternal;
using namespace IceSSL;
using namespace IceSSL::SecureTransport;

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
        const TransceiverI* transceiver = static_cast<const TransceiverI*>(connection);
        assert(transceiver);
        return transceiver->writeRaw(reinterpret_cast<const byte*>(data), length);
    }

    //
    // Socket read callback
    //
    OSStatus socketRead(SSLConnectionRef connection, void* data, size_t* length)
    {
        const TransceiverI* transceiver = static_cast<const TransceiverI*>(connection);
        assert(transceiver);
        return transceiver->readRaw(reinterpret_cast<byte*>(data), length);
    }
}

IceInternal::NativeInfoPtr
IceSSL::SecureTransport::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
IceSSL::SecureTransport::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
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
        //
        // Initialize SSL context
        //
        _ssl.reset(_engine->newContext(_incoming));
        if (_sslContextSetupCallback)
        {
            _sslContextSetupCallback(_ssl.get());
        }

        if (_incoming)
        {
            SSLSetClientSideAuthenticate(_ssl.get(), _clientCertificateRequired);
        }

        CFArrayRef certificateChain = _localCertificateSelectionCallback
                                          ? _localCertificateSelectionCallback(_incoming ? _adapterName : _host)
                                          : _certificateChain;
        if (certificateChain && (err = SSLSetCertificate(_ssl.get(), certificateChain)))
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                "IceSSL: error while setting the SSL context certificate:\n" + sslErrorToString(err));
        }

        if ((err = SSLSetIOFuncs(_ssl.get(), socketRead, socketWrite)))
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                "IceSSL: setting IO functions failed\n" + sslErrorToString(err));
        }

        if ((err = SSLSetConnection(_ssl.get(), reinterpret_cast<SSLConnectionRef>(this))))
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                "IceSSL: setting SSL connection failed\n" + sslErrorToString(err));
        }

        //
        // Enable SNI
        //
        if (!_incoming && _engine->getServerNameIndication() && !_host.empty() && !IceInternal::isIpAddress(_host))
        {
            if ((err = SSLSetPeerDomainName(_ssl.get(), _host.data(), _host.length())))
            {
                throw SecurityException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: setting SNI host failed `" + _host + "'\n" + sslErrorToString(err));
            }
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

            if (_incoming && _engine->getVerifyPeer() == 1 && (err == errSSLBadCert || !_trust))
            {
                // This is expected if the client doesn't provide a certificate. With 10.10 and 10.11 errSSLBadCert
                // is expected, the server is configured to verify but not require the client
                // certificate so we ignore the failure. In 10.12 there is no error and trust is 0.
                continue;
            }

            for (CFIndex i = 0, count = SecTrustGetCertificateCount(_trust.get()); i < count; ++i)
            {
                SecCertificateRef cert = SecTrustGetCertificateAtIndex(_trust.get(), i);
                CFRetain(cert);
                _certs.push_back(IceSSL::SecureTransport::Certificate::create(cert));
            }

            function<bool(SecTrustRef trust, const IceSSL::ConnectionInfoPtr& info)>
                remotCertificateValidationCallback =
                    _remotCertificateValidationCallback
                        ? _remotCertificateValidationCallback
                        : [this](SecTrustRef trust, const IceSSL::ConnectionInfoPtr& info)
            { return _engine->validationCallback(trust, info, _incoming, _host, _trustedRootCertificates); };

            if (err == noErr &&
                remotCertificateValidationCallback(_trust.get(), dynamic_pointer_cast<ConnectionInfo>(getInfo())))
            {
                continue; // Call SSLHandshake to resume the handsake.
            }
            // Let it fall through, this will raise a SecurityException with the SSLCopyPeerTrust error.
        }
        else if (err == errSSLClosedGraceful || err == errSSLClosedAbort)
        {
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }

        ostringstream os;
        os << "IceSSL: ssl error occurred for new " << (_incoming ? "incoming" : "outgoing") << " connection:\n"
           << _delegate->toString() << "\n"
           << sslErrorToString(err);
        throw ProtocolException(__FILE__, __LINE__, os.str());
    }

    assert(_ssl);

    if (_instance->engine()->securityTraceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
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
IceSSL::SecureTransport::TransceiverI::closing(bool initiator, exception_ptr)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
IceSSL::SecureTransport::TransceiverI::close()
{
    _trust.reset(0);
    if (_ssl)
    {
        SSLClose(_ssl.get());
    }
    _ssl.reset(0);

    _delegate->close();
}

IceInternal::SocketOperation
IceSSL::SecureTransport::TransceiverI::write(IceInternal::Buffer& buf)
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
        OSStatus err = _buffered ? SSLWrite(_ssl.get(), 0, 0, &processed)
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
                throw ProtocolException(__FILE__, __LINE__, "IceSSL: error during write:\n" + sslErrorToString(err));
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
IceSSL::SecureTransport::TransceiverI::read(IceInternal::Buffer& buf)
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
                throw ProtocolException(__FILE__, __LINE__, "IceSSL: error during read:\n" + sslErrorToString(err));
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

bool
IceSSL::SecureTransport::TransceiverI::isWaitingToBeRead() const noexcept
{
    return _delegate->isWaitingToBeRead();
}

string
IceSSL::SecureTransport::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
IceSSL::SecureTransport::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
IceSSL::SecureTransport::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceSSL::SecureTransport::TransceiverI::getInfo() const
{
    auto info = make_shared<IceSSL::ConnectionInfo>();
    info->underlying = _delegate->getInfo();
    info->incoming = _incoming;
    info->adapterName = _adapterName;
    info->certs = _certs;
    info->host = _host;
    return info;
}

void
IceSSL::SecureTransport::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
IceSSL::SecureTransport::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

IceSSL::SecureTransport::TransceiverI::TransceiverI(
    const IceSSL::InstancePtr& instance,
    const IceInternal::TransceiverPtr& delegate,
    const string& adapterName,
    const ServerAuthenticationOptions& serverAuthenticationOptions)
    : _instance(instance),
      _engine(dynamic_pointer_cast<IceSSL::SecureTransport::SSLEngine>(instance->engine())),
      _host(""),
      _adapterName(adapterName),
      _incoming(true),
      _delegate(delegate),
      _connected(false),
      _buffered(0),
      _sslContextSetupCallback(serverAuthenticationOptions.sslContextSetup),
      _remotCertificateValidationCallback(serverAuthenticationOptions.clientCertificateValidationCallback),
      _certificateChain(serverAuthenticationOptions.serverCeriticateChain),
      _trustedRootCertificates(serverAuthenticationOptions.trustedRootCertificates),
      _localCertificateSelectionCallback(serverAuthenticationOptions.serverCertificateSelectionCallback),
      _clientCertificateRequired(serverAuthenticationOptions.clientCertificateRequired)
{
}

IceSSL::SecureTransport::TransceiverI::TransceiverI(
    const IceSSL::InstancePtr& instance,
    const IceInternal::TransceiverPtr& delegate,
    const string& host,
    const ClientAuthenticationOptions& clientAuthenticationOptions)
    : _instance(instance),
      _engine(dynamic_pointer_cast<IceSSL::SecureTransport::SSLEngine>(instance->engine())),
      _host(host),
      _adapterName(""),
      _incoming(false),
      _delegate(delegate),
      _connected(false),
      _buffered(0),
      _sslContextSetupCallback(clientAuthenticationOptions.sslContextSetup),
      _remotCertificateValidationCallback(clientAuthenticationOptions.serverCertificateValidationCallback),
      _certificateChain(clientAuthenticationOptions.clientCeriticateChain),
      _trustedRootCertificates(clientAuthenticationOptions.trustedRootCertificates),
      _localCertificateSelectionCallback(clientAuthenticationOptions.clientCertificateSelectionCallback),
      _clientCertificateRequired(kNeverAuthenticate)
{
}

IceSSL::SecureTransport::TransceiverI::~TransceiverI() {}

OSStatus
IceSSL::SecureTransport::TransceiverI::writeRaw(const byte* data, size_t* length) const
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
        return ex.error;
    }
    catch (...)
    {
        assert(false);
        return IceInternal::getSocketErrno();
    }
    return noErr;
}

OSStatus
IceSSL::SecureTransport::TransceiverI::readRaw(byte* data, size_t* length) const
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
        return ex.error;
    }
    catch (...)
    {
        assert(false);
        return IceInternal::getSocketErrno();
    }
    return noErr;
}
