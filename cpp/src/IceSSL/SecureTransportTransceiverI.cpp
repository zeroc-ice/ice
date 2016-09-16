// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/SecureTransportTransceiverI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>

#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>

#ifdef ICE_USE_SECURE_TRANSPORT

using namespace std;
using namespace Ice;
using namespace IceSSL;

namespace
{

string
trustResultDescription(SecTrustResultType result)
{
    switch(result)
    {
        case kSecTrustResultInvalid:
        {
            return "Invalid setting or result";
        }
        case kSecTrustResultDeny:
        {
            return "The user specified that the certificate should not be trusted";
        }
        case kSecTrustResultRecoverableTrustFailure:
        case kSecTrustResultFatalTrustFailure:
        {
            return "Trust denied";
        }
        case kSecTrustResultOtherError:
        {
            return "Other error internal error";
        }
        default:
        {
            assert(false);
            return "";
        }
    }
}

string
protocolName(SSLProtocol protocol)
{
    switch(protocol)
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
        default:
            return "Unknown";
    }
}

//
// Socket write callback
//
OSStatus
socketWrite(SSLConnectionRef connection, const void* data, size_t* length)
{
    const TransceiverI* transceiver = static_cast<const TransceiverI*>(connection);
    assert(transceiver);
    return transceiver->writeRaw(reinterpret_cast<const char*>(data), length);
}

//
// Socket read callback
//
OSStatus
socketRead(SSLConnectionRef connection, void* data, size_t* length)
{
    const TransceiverI* transceiver = static_cast<const TransceiverI*>(connection);
    assert(transceiver);
    return transceiver->readRaw(reinterpret_cast<char*>(data), length);
}

bool
checkTrustResult(SecTrustRef trust, const SecureTransportEnginePtr& engine, const InstancePtr& instance)
{
    OSStatus err = noErr;
    SecTrustResultType trustResult = kSecTrustResultOtherError;
    if(trust)
    {
        if((err = SecTrustSetAnchorCertificates(trust, engine->getCertificateAuthorities())))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + errorToString(err));
        }

        //
        // Disable network fetch, we don't want this to block.
        //
        if((err = SecTrustSetNetworkFetchAllowed(trust, false)))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + errorToString(err));
        }

        //
        // Evaluate the trust
        //
        if((err = SecTrustEvaluate(trust, &trustResult)))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + errorToString(err));
        }
    }

    switch(trustResult)
    {
    case kSecTrustResultUnspecified:
    case kSecTrustResultProceed:
    {
        //
        // Trust verify success.
        //
        return true;
    }
    default:
    // case kSecTrustResultInvalid:
    // case kSecTrustResultConfirm: // Used in old OS X versions
    // case kSecTrustResultDeny:
    // case kSecTrustResultRecoverableTrustFailure:
    // case kSecTrustResultFatalTrustFailure:
    // case kSecTrustResultOtherError:
    {
        if(engine->getVerifyPeer() == 0)
        {
            if(instance->traceLevel() >= 1)
            {
                ostringstream os;
                os << "IceSSL: ignoring certificate verification failure\n" << trustResultDescription(trustResult);
                instance->logger()->trace(instance->traceCategory(), os.str());
            }
            return false;
        }
        else
        {
            ostringstream os;
            os << "IceSSL: certificate verification failure\n" << trustResultDescription(trustResult);
            string msg = os.str();
            if(instance->traceLevel() >= 1)
            {
                instance->logger()->trace(instance->traceCategory(), msg);
            }
            throw SecurityException(__FILE__, __LINE__, msg);
        }
    }
    }
}
}

IceInternal::NativeInfoPtr
IceSSL::TransceiverI::getNativeInfo()
{
    return _stream;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer, bool&)
{
    IceInternal::SocketOperation status = _stream->connect(readBuffer, writeBuffer);
    if(status != IceInternal::SocketOperationNone)
    {
        return status;
    }

    OSStatus err = 0;
    if(!_ssl)
    {
        //
        // Initialize SSL context
        //
        _ssl = _engine->newContext(_incoming);
        if((err = SSLSetIOFuncs(_ssl, socketRead, socketWrite)))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: setting IO functions failed\n" +
                                    errorToString(err));
        }

        if((err = SSLSetConnection(_ssl, reinterpret_cast<SSLConnectionRef>(this))))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: setting SSL connection failed\n" +
                                    errorToString(err));
        }
    }

    SSLSessionState state;
    SSLGetSessionState(_ssl, &state);

    //
    // SSL Handshake
    //
    while(state == kSSLHandshake || state == kSSLIdle)
    {
        err = SSLHandshake(_ssl);
        if(err == noErr)
        {
            break; // We're done!
        }
        else if(err == errSSLWouldBlock)
        {
            assert(_flags & SSLWantRead || _flags & SSLWantWrite);
            return _flags & SSLWantRead ? IceInternal::SocketOperationRead : IceInternal::SocketOperationWrite;
        }
        else if(err == errSSLPeerAuthCompleted)
        {
            assert(!_trust);
            err = SSLCopyPeerTrust(_ssl, &_trust);
            if(_incoming && _engine->getVerifyPeer() == 1 && (err == errSSLBadCert || _trust == 0))
            {
                // This is expected if the client doesn't provide a certificate. With 10.10 and 10.11 errSSLBadCert
                // is expected, the server is configured to verify but not require the client
                // certificate so we ignore the failure. In 10.12 there is no error and trust is 0.
                continue;
            }
            if(err == noErr)
            {
                _verified = checkTrustResult(_trust, _engine, _instance);
                continue; // Call SSLHandshake to resume the handsake.
            }
            // Let it fall through, this will raise a SecurityException with the SSLCopyPeerTrust error.
        }
        else if(err == errSSLClosedGraceful || err == errSSLClosedAbort)
        {
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }

        IceInternal::Address remoteAddr;
        string desc = "<not available>";
        if(IceInternal::fdToRemoteAddress(_stream->fd(), remoteAddr))
        {
            desc = IceInternal::addrToString(remoteAddr);
        }
        ostringstream os;
        os << "IceSSL: ssl error occurred for new " << (_incoming ? "incoming" : "outgoing") << " connection:\n"
           << "remote address = " << desc << "\n" << errorToString(err);
        throw ProtocolException(__FILE__, __LINE__, os.str());
    }
    _engine->verifyPeer(_stream->fd(), _host, NativeConnectionInfoPtr::dynamicCast(getInfo()));

    if(_instance->engine()->securityTraceLevel() >= 1)
    {
        assert(_ssl);
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "SSL summary for " << (_incoming ? "incoming" : "outgoing") << " connection\n";

        SSLProtocol protocol;
        SSLGetNegotiatedProtocolVersion(_ssl, &protocol);
        const string sslProtocolName = protocolName(protocol);

        SSLCipherSuite cipher;
        SSLGetNegotiatedCipher(_ssl, &cipher);
        const string sslCipherName = _engine->getCipherName(cipher);

        if(sslCipherName.empty())
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
IceSSL::TransceiverI::closing(bool initiator, const Ice::LocalException&)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
IceSSL::TransceiverI::close()
{
    if(_trust)
    {
        CFRelease(_trust);
        _trust = 0;
    }

    if(_ssl)
    {
        SSLClose(_ssl);
        CFRelease(_ssl);
        _ssl = 0;
    }

    _stream->close();
}

IceInternal::SocketOperation
IceSSL::TransceiverI::write(IceInternal::Buffer& buf)
{
    if(!_stream->isConnected())
    {
        return _stream->write(buf);
    }

    if(buf.i == buf.b.end())
    {
        return  IceInternal::SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    size_t packetSize = std::min(static_cast<size_t>(buf.b.end() - buf.i), _maxSendPacketSize);
    while(buf.i != buf.b.end())
    {
        size_t processed = 0;
        OSStatus err = _buffered ? SSLWrite(_ssl, 0, 0, &processed) :
                                   SSLWrite(_ssl, reinterpret_cast<const void*>(buf.i), packetSize, &processed);

        if(err)
        {
            if(err == errSSLWouldBlock)
            {
                if(_buffered == 0)
                {
                    _buffered = processed;
                }
                assert(_flags & SSLWantWrite);
                return IceInternal::SocketOperationWrite;
            }

            if(err == errSSLClosedGraceful)
            {
                throw ConnectionLostException(__FILE__, __LINE__, 0);
            }

            //
            // SSL protocol errors are defined in SecureTransport.h are in the range
            // -9800 to -9849
            //
            if(err <= -9800 && err >= -9849)
            {
                throw ProtocolException(__FILE__, __LINE__, "IceSSL: error during write:\n" + errorToString(err));
            }

            errno = err;
            if(IceInternal::connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
        }

        if(_buffered)
        {
            buf.i += _buffered;
            _buffered = 0;
        }
        else
        {
            buf.i += processed;
        }

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::read(IceInternal::Buffer& buf, bool& hasMoreData)
{
    if(!_stream->isConnected())
    {
        return _stream->read(buf);
    }

    if(buf.i == buf.b.end())
    {
        return  IceInternal::SocketOperationNone;
    }

    size_t packetSize = std::min(static_cast<size_t>(buf.b.end() - buf.i), _maxRecvPacketSize);
    while(buf.i != buf.b.end())
    {
        size_t processed = 0;
        OSStatus err = SSLRead(_ssl, reinterpret_cast<void*>(buf.i), packetSize, &processed);
        if(err)
        {
            if(err == errSSLWouldBlock)
            {
                buf.i += processed;
                assert(_flags & SSLWantRead);
                return IceInternal::SocketOperationRead;
            }

            if(err == errSSLClosedGraceful || err == errSSLClosedAbort)
            {
                throw ConnectionLostException(__FILE__, __LINE__, 0);
            }

            //
            // SSL protocol errors are defined in SecureTransport.h are in the range
            // -9800 to -9849
            //
            if(err <= -9800 && err >= -9849)
            {
                throw ProtocolException(__FILE__, __LINE__, "IceSSL: error during read:\n" + errorToString(err));
            }

            errno = err;
            if(IceInternal::connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
        }

        buf.i += processed;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }

    //
    // Check if there's still buffered data to read. In this case, set hasMoreData to true.
    //
    size_t buffered = 0;
    OSStatus err = SSLGetBufferedReadSize(_ssl, &buffered);
    if(err)
    {
        errno = err;
        throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
    }
    hasMoreData = buffered > 0;
    return IceInternal::SocketOperationNone;
}

string
IceSSL::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
IceSSL::TransceiverI::toString() const
{
    return _stream->toString();
}

string
IceSSL::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getInfo() const
{
    NativeConnectionInfoPtr info = new NativeConnectionInfo();
    fillConnectionInfo(info, info->nativeCerts);
    return info;
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getWSInfo(const Ice::HeaderDict& headers) const
{
    WSSNativeConnectionInfoPtr info = new WSSNativeConnectionInfo();
    fillConnectionInfo(info, info->nativeCerts);
    info->headers = headers;
    return info;
}

void
IceSSL::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
IceSSL::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _stream->setBufferSize(rcvSize, sndSize);
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance,
                                   const IceInternal::StreamSocketPtr& stream,
                                   const string& hostOrAdapterName,
                                   bool incoming) :
    _instance(instance),
    _engine(SecureTransportEnginePtr::dynamicCast(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _stream(stream),
    _ssl(0),
    _trust(0),
    _verified(false),
    _buffered(0)
{
    //
    // Limit the size of packets passed to SSLWrite/SSLRead to avoid
    // blocking and holding too much memory.
    //
    _maxSendPacketSize = std::max(512, IceInternal::getSendBufferSize(_stream->fd()));
    _maxRecvPacketSize = std::max(512, IceInternal::getRecvBufferSize(_stream->fd()));
}

IceSSL::TransceiverI::~TransceiverI()
{
}

void
IceSSL::TransceiverI::fillConnectionInfo(const ConnectionInfoPtr& info, std::vector<CertificatePtr>& nativeCerts) const
{
    IceInternal::fdToAddressAndPort(_stream->fd(), info->localAddress, info->localPort, info->remoteAddress,
                                    info->remotePort);
    if(_stream->fd() != INVALID_SOCKET)
    {
        info->rcvSize = IceInternal::getRecvBufferSize(_stream->fd());
        info->sndSize = IceInternal::getSendBufferSize(_stream->fd());
    }

    if(_ssl)
    {
        for(int i = 0, count = SecTrustGetCertificateCount(_trust); i < count; ++i)
        {
            SecCertificateRef cert = SecTrustGetCertificateAtIndex(_trust, i);
            CFRetain(cert);

            CertificatePtr certificate = new Certificate(cert);
            nativeCerts.push_back(certificate);
            info->certs.push_back(certificate->encode());
        }

        SSLCipherSuite cipher;
        SSLGetNegotiatedCipher(_ssl, &cipher);
        info->cipher = _engine->getCipherName(cipher);
        info->verified = _verified;
    }
    else
    {
        info->verified = false;
    }

    info->adapterName = _adapterName;
    info->incoming = _incoming;
}

OSStatus
IceSSL::TransceiverI::writeRaw(const char* data, size_t* length) const
{
    _flags &= ~SSLWantWrite;

    try
    {
        ssize_t ret = _stream->write(data, *length);
        if(ret < *length)
        {
            *length = static_cast<size_t>(ret);
            _flags |= SSLWantWrite;
            return errSSLWouldBlock;
        }
    }
    catch(const Ice::ConnectionLostException&)
    {
        return errSSLClosedGraceful;
    }
    catch(const Ice::SocketException& ex)
    {
        return ex.error;
    }
    catch(...)
    {
        assert(false);
        return IceInternal::getSocketErrno();
    }
    return noErr;
}

OSStatus
IceSSL::TransceiverI::readRaw(char* data, size_t* length) const
{
    _flags &= ~SSLWantRead;

    try
    {
        ssize_t ret = _stream->read(data, *length);
        if(ret < *length)
        {
            *length = static_cast<size_t>(ret);
            _flags |= SSLWantRead;
            return errSSLWouldBlock;
        }
    }
    catch(const Ice::ConnectionLostException&)
    {
        return errSSLClosedGraceful;
    }
    catch(const Ice::SocketException& ex)
    {
        return ex.error;
    }
    catch(...)
    {
        assert(false);
        return IceInternal::getSocketErrno();
    }
    return noErr;
}

#endif
