// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/SecureTransportTransceiverI.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

#include <IceSSL/ConnectionInfo.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/Util.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
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
            return "Trust denied; no simple fix is available";
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

}

IceInternal::NativeInfoPtr
IceSSL::TransceiverI::getNativeInfo()
{
    return this;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer, bool&)
{
    try
    {
        if(_state == StateNeedConnect)
        {
            _state = StateConnectPending;
            return IceInternal::SocketOperationConnect;
        }
        else if(_state <= StateConnectPending)
        {
            IceInternal::doFinishConnect(_fd);
            _desc = IceInternal::fdToString(_fd, _proxy, _addr, true);

            if(_proxy)
            {
                //
                // Prepare the read & write buffers in advance.
                //
                _proxy->beginWriteConnectRequest(_addr, writeBuffer);
                _proxy->beginReadConnectRequestResponse(readBuffer);

                //
                // Write the proxy connection message using TCP.
                //
                if(writeRaw(writeBuffer))
                {
                    //
                    // Write completed without blocking.
                    //
                    _proxy->endWriteConnectRequest(writeBuffer);

                    //
                    // Try to read the response using TCP.
                    //
                    if(readRaw(readBuffer))
                    {
                        //
                        // Read completed without blocking - fall through.
                        //
                        _proxy->endReadConnectRequestResponse(readBuffer);
                    }
                    else
                    {
                        //
                        // Return SocketOperationRead to indicate we need to complete the read.
                        //
                        _state = StateProxyConnectRequestPending; // Wait for proxy response
                        return IceInternal::SocketOperationRead;
                    }
                }
                else
                {
                    //
                    // Return SocketOperationWrite to indicate we need to complete the write.
                    //
                    _state = StateProxyConnectRequest; // Send proxy connect request
                    return IceInternal::SocketOperationWrite;
                }
            }

            _state = StateConnected;
        }
        else if(_state == StateProxyConnectRequest)
        {
            //
            // Write completed.
            //
            _proxy->endWriteConnectRequest(writeBuffer);
            _state = StateProxyConnectRequestPending; // Wait for proxy response
            return IceInternal::SocketOperationRead;
        }
        else if(_state == StateProxyConnectRequestPending)
        {
            //
            // Read completed.
            //
            _proxy->endReadConnectRequestResponse(readBuffer);
            _state = StateConnected;
        }

        assert(_state == StateConnected);

        OSStatus err = noErr;
        
        if(!_ssl)
        {
            //
            // Initialize SSL context
            //
            _ssl = _engine->newContext(_incoming);
            if((err = SSLSetIOFuncs(_ssl, socketRead, socketWrite)) != noErr)
            {
                ostringstream os;
                os << "IceSSL: cannot set SSL IO functions\n" << errorToString(err);
                throw PluginInitializationException(__FILE__, __LINE__, os.str());
            }
            
            if((err = SSLSetConnection(_ssl, reinterpret_cast<SSLConnectionRef>(this))) != noErr)
            {
                ostringstream os;
                os << "IceSSL: cannot set SSL connection\n" << errorToString(err);
                throw PluginInitializationException(__FILE__, __LINE__, os.str());
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
            if(err != noErr)
            {
                switch(err)
                {
                    case errSSLWouldBlock:
                    {
                        assert(_flags & SSLWantRead || _flags & SSLWantWrite);
                        return _flags & SSLWantRead ? IceInternal::SocketOperationRead : IceInternal::SocketOperationWrite;
                    }
                    case errSSLPeerAuthCompleted:
                    {
                        assert(!_trust);
                        err = SSLCopyPeerTrust(_ssl, &_trust);
                        if(err != noErr)
                        {
                            break;
                        }
                        
                        SecTrustResultType trustResult = kSecTrustResultOtherError;

                        if(_trust)
                        {
                            err = SecTrustSetAnchorCertificates(_trust, _engine->getCertificateAuthorities());
                            
                            if(err != noErr)
                            {
                                ostringstream os;
                                os << "SSL handsake failure:\n" << errorToString(err);
                                throw SecurityException(__FILE__, __LINE__, os.str());
                            }
                            
                            //
                            // Disable network fetch, we don't want this to block.
                            //
                            err = SecTrustSetNetworkFetchAllowed(_trust, false);
                            if(err != noErr)
                            {
                                ostringstream os;
                                os << "SSL handsake failure:\n" << errorToString(err);
                                throw SecurityException(__FILE__, __LINE__, os.str());
                            }
                            
                            //
                            // Evaluate the trust
                            //
                            err = SecTrustEvaluate(_trust, &trustResult);
                            if(err != noErr)
                            {
                                ostringstream os;
                                os << "SSL handsake failure:\n" << errorToString(err);
                                throw SecurityException(__FILE__, __LINE__, os.str());;
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
                                break;
                            }
                            case kSecTrustResultInvalid:
                            //case kSecTrustResultConfirm: // Used in old OS X versions
                            case kSecTrustResultDeny:
                            case kSecTrustResultRecoverableTrustFailure:
                            case kSecTrustResultFatalTrustFailure:
                            case kSecTrustResultOtherError:
                            {
                                if(_engine->getVerifyPeer() == 0)
                                {
                                    if(_instance->traceLevel() >= 1)
                                    {
                                        ostringstream os;
                                        os << "IceSSL: ignoring certificate verification failure\n" 
                                           << trustResultDescription(trustResult);
                                        _instance->logger()->trace(_instance->traceCategory(), os.str());
                                    }
                                    break;
                                }
                                else
                                {
                                    ostringstream os;
                                    os << "IceSSL: certificate verification failure\n" 
                                       << trustResultDescription(trustResult);
                                    string msg = os.str();
                                    if(_instance->traceLevel() >= 1)
                                    {
                                        _instance->logger()->trace(_instance->traceCategory(), msg);
                                    }
                                    throw ProtocolException(__FILE__, __LINE__, msg);
                                }
                            }
                        }
                        //
                        // Call SSLHandshake to resume the handsake.
                        //
                        continue;
                    }
                    default:
                    {
                        break;
                    }
                }

                if(err == errSSLClosedGraceful || err == errSSLClosedAbort)
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }
                else
                {
                    IceInternal::Address remoteAddr;
                    string desc = "<not available>";
                    if(IceInternal::fdToRemoteAddress(_fd, remoteAddr))
                    {
                        desc = IceInternal::addrToString(remoteAddr);
                    }
                    ostringstream os;
                    os << "SSL error occurred for new " << (_incoming ? "incoming" : "outgoing")
                    << " connection:\nremote address = " << desc << "\n" 
                    << errorToString(err);
                    ProtocolException ex(__FILE__, __LINE__, os.str());
                    throw ex;
                }
            }
            break;
        }
        _engine->verifyPeer(_fd, _host, getNativeConnectionInfo());
        _state = StateHandshakeComplete;
    }
    catch(const Ice::LocalException& ex)
    {
        if(_instance->traceLevel() >= 2)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "failed to establish " << _instance->protocol() << " connection\n";
            if(_incoming)
            {
                out << IceInternal::fdToString(_fd) << "\n" << ex;
            }
            else
            {
                out << IceInternal::fdToString(_fd, _proxy, _addr, false) << "\n" << ex;
            }
        }
        throw;
    }

    if(_instance->traceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        if(_incoming)
        {
            out << "accepted " << _instance->protocol() << " connection\n" << _desc;
        }
        else
        {
            out << _instance->protocol() << " connection established\n" << _desc;
        }
    }

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
        out << IceInternal::fdToString(_fd);
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
    if(_state == StateHandshakeComplete && _instance->traceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "closing " << _instance->protocol() << " connection\n" << toString();
    }

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

    assert(_fd != INVALID_SOCKET);
    try
    {
        IceInternal::closeSocket(_fd);
        _fd = INVALID_SOCKET;
    }
    catch(const SocketException&)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
}

IceInternal::SocketOperation
IceSSL::TransceiverI::write(IceInternal::Buffer& buf)
{
    if(_state == StateProxyConnectRequest)
    {
        //
        // We need to write the proxy message, but we have to use TCP and not SSL.
        //
        return writeRaw(buf) ? IceInternal::SocketOperationNone : IceInternal::SocketOperationWrite;
    }
    
    size_t processed = 0;

    if(_buffered > 0)
    {
        //
        // Required to flush SSL buffers
        //
        if(SSLWrite(_ssl, 0, 0, &processed) == errSSLWouldBlock)
        {
            return IceInternal::SocketOperationWrite;
        }

        buf.i += _buffered;
        _buffered = 0;
    }
    
    if(buf.i == buf.b.end())
    {
        return  IceInternal::SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    size_t packetSize = buf.b.end() - buf.i;
    packetSize = std::min(packetSize, _maxSendPacketSize);
    while(buf.i != buf.b.end())
    {
        assert(_fd != INVALID_SOCKET);
        OSStatus ret = SSLWrite(_ssl, reinterpret_cast<const void*>(buf.i), packetSize, &processed);
        if(ret != noErr)
        {
            if(ret == errSSLWouldBlock)
            {
                _buffered = processed;
                assert(_flags & SSLWantWrite);
                return IceInternal::SocketOperationWrite;
            }
            
            if(ret == errSSLClosedGraceful)
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = 0;
                throw ex;
            }
            
            //
            // SSL protocol errors are defined in SecureTransport.h are in the range
            // -9800 to -9849
            //
            if(ret <= -9800 && ret >= -9849)
            {
                ProtocolException ex(__FILE__, __LINE__);
                ostringstream os;
                os << "SSL protocol error during read:\n" << errorToString(ret);
                ex.reason = os.str();
                throw ex;
            }
            
            errno = ret;
            if(IceInternal::connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
        }

        if(_instance->traceLevel() >= 3)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "sent " << processed << " of " << packetSize << " bytes via " << protocol() << "\n" << toString();
        }

        buf.i += processed;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::read(IceInternal::Buffer& buf, bool&)
{
    if(_state == StateProxyConnectRequestPending)
    {
        //
        // We need to read the proxy reply, but we have to use TCP and not SSL.
        //
        return readRaw(buf) ? IceInternal::SocketOperationNone : IceInternal::SocketOperationRead;
    }

    if(buf.i == buf.b.end())
    {
        return  IceInternal::SocketOperationNone;
    }
    //
    // It's impossible for packetSize to be more than an Int.
    //
    size_t packetSize = buf.b.end() - buf.i;
    size_t processed = 0;
    
    packetSize = std::min(packetSize, _maxReceivePacketSize);
    
    while(buf.i != buf.b.end())
    {
        assert(_fd != INVALID_SOCKET);
        OSStatus ret = SSLRead(_ssl, reinterpret_cast<void*>(buf.i), packetSize, &processed);
        if(ret != noErr)
        {
            if(ret == errSSLWouldBlock)
            {
                buf.i += processed;
                assert(_flags & SSLWantRead);
                return IceInternal::SocketOperationRead;
            }
            
            if(ret == errSSLClosedGraceful || ret == errSSLPeerBadRecordMac || ret == errSSLPeerDecryptionFail)
            {
                //
                // Forcefully closing a connection can result in SSLRead reporting
                // "decryption failed or bad record mac". We trap that error and
                // treat it as the loss of a connection.
                //
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = 0;
                throw ex;
            }
            
            //
            // SSL protocol errors are defined in SecureTransport.h are in the range
            // -9800 to -9849
            //
            if(ret <= -9800 && ret >= -9849)
            {
                ProtocolException ex(__FILE__, __LINE__);
                ostringstream os;
                os << "SSL protocol error during read:\n" << errorToString(ret);
                ex.reason = os.str();
                throw ex;
            }
            
            errno = ret;
            if(IceInternal::connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
        }

        if(_instance->traceLevel() >= 3)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "received " << processed << " of " << packetSize << " bytes via " << protocol() << "\n" 
                << toString();
        }
        buf.i += processed;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }
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
    return _desc;
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getInfo() const
{
    return getNativeConnectionInfo();
}

void
IceSSL::TransceiverI::checkSendSize(const IceInternal::Buffer& buf, size_t messageSizeMax)
{
    if(buf.b.size() > messageSizeMax)
    {
        IceInternal::Ex::throwMemoryLimitException(__FILE__, __LINE__, buf.b.size(), messageSizeMax);
    }
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance, SOCKET fd, const IceInternal::NetworkProxyPtr& proxy,
                                   const string& host, const IceInternal::Address& addr) :
    IceInternal::NativeInfo(fd),
    _instance(instance),
    _engine(SecureTransportEnginePtr::dynamicCast(instance->engine())),
    _proxy(proxy),
    _host(host),
    _addr(addr),
    _incoming(false),
    _ssl(0),
    _trust(0),
    _buffered(0),
    _state(StateNeedConnect)
{
    assert(_engine);
    IceInternal::setBlock(fd, false);
    IceInternal::setTcpBufSize(fd, _instance->properties(), _instance->logger());
    
    IceInternal::Address connectAddr = proxy ? proxy->getAddress() : addr;
    if(IceInternal::doConnect(_fd, connectAddr))
    {
        _state = StateConnected;
        _desc = IceInternal::fdToString(_fd, _proxy, _addr, true);
        if(_instance->traceLevel() >= 1)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << _instance->protocol() << " connection established\n" << _desc;
        }
    }
    else
    {
        _desc = IceInternal::fdToString(_fd, _proxy, _addr, true);
    }
    
    //
    // Limit the size of packet passed to SSLWrite/SSLRead to avoid blocking and
    // holding too much memory.
    //
    _maxSendPacketSize = std::max(512, IceInternal::getSendBufferSize(fd));
    _maxReceivePacketSize = std::max(512, IceInternal::getRecvBufferSize(fd));
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance, SOCKET fd, const string& adapterName) :
    IceInternal::NativeInfo(fd),
    _instance(instance),
    _engine(SecureTransportEnginePtr::dynamicCast(instance->engine())),
    _addr(IceInternal::Address()),
    _adapterName(adapterName),
    _incoming(true),
    _ssl(0),
    _trust(0),
    _buffered(0),
    _state(StateConnected),
    _desc(IceInternal::fdToString(fd))
{
    assert(_engine);
    IceInternal::setBlock(fd, false);
    IceInternal::setTcpBufSize(fd, _instance->properties(), _instance->logger());
    
    //
    // Limit the size of packet passed to SSLWrite/SSLRead to avoid blocking and
    // holding too much memory.
    //
    _maxSendPacketSize = std::max(512, IceInternal::getSendBufferSize(fd));
    _maxReceivePacketSize = std::max(512, IceInternal::getRecvBufferSize(fd));
}

IceSSL::TransceiverI::~TransceiverI()
{
    assert(_fd == INVALID_SOCKET);
}

NativeConnectionInfoPtr
IceSSL::TransceiverI::getNativeConnectionInfo() const
{
    NativeConnectionInfoPtr info = new NativeConnectionInfo();
    IceInternal::fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);

    if(_ssl)
    {
        for(int i = 0, count = SecTrustGetCertificateCount(_trust); i < count; ++i)
        {
            SecCertificateRef cert = SecTrustGetCertificateAtIndex(_trust, i);
            CFRetain(cert);
            
            CertificatePtr certificate = new Certificate(cert);
            info->nativeCerts.push_back(certificate);
            info->certs.push_back(certificate->encode());
        }
        
        SSLCipherSuite cipher;
        SSLGetNegotiatedCipher(_ssl, &cipher);
        info->cipher = _engine->getCipherName(cipher);
    }

    info->adapterName = _adapterName;
    info->incoming = _incoming;
    return info;
}

bool
IceSSL::TransceiverI::writeRaw(IceInternal::Buffer& buf)
{
    int packetSize = buf.b.end() - buf.i;
    while(buf.i != buf.b.end())
    {
        assert(_fd != INVALID_SOCKET);

        ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&*buf.i), packetSize, 0);
        if(ret == 0)
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

        if(ret == SOCKET_ERROR)
        {
            if(IceInternal::interrupted())
            {
                continue;
            }

            if(IceInternal::noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if(IceInternal::wouldBlock())
            {
                return false;
            }

            if(IceInternal::connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
        }

        if(_instance->traceLevel() >= 3)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "sent " << ret << " of " << packetSize << " bytes via " << protocol() << "\n" << toString();
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }

    return true;
}

bool
IceSSL::TransceiverI::readRaw(IceInternal::Buffer& buf)
{
    int packetSize = buf.b.end() - buf.i;
    while(buf.i != buf.b.end())
    {
        assert(_fd != INVALID_SOCKET);
        ssize_t ret = ::recv(_fd, reinterpret_cast<char*>(&*buf.i), packetSize, 0);

        if(ret == 0)
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

        if(ret == SOCKET_ERROR)
        {
            if(IceInternal::interrupted())
            {
                continue;
            }

            if(IceInternal::noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if(IceInternal::wouldBlock())
            {
                return false;
            }

            if(IceInternal::connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
        }

        if(_instance->traceLevel() >= 3)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "received " << ret << " of " << packetSize << " bytes via " << protocol() << "\n" << toString();
        }

        buf.i += ret;
        packetSize = buf.b.end() - buf.i;
    }

    return true;
}

OSStatus
IceSSL::TransceiverI::writeRaw(const char* data, size_t* length) const
{
    _flags &= ~SSLWantWrite;
    
    assert(_fd != INVALID_SOCKET);
    
    char* i = const_cast<char*>(data);
    int packetSize = *length;
    char* end = i + packetSize;
    
    while(i != end)
    {
        ssize_t ret = ::send(_fd, const_cast<const char*>(i), packetSize, 0);
        if(ret == 0)
        {
            return errSSLClosedGraceful;
        }
        
        if(ret == SOCKET_ERROR)
        {
            if(IceInternal::interrupted())
            {
                continue;
            }

            if(IceInternal::noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }
            
            if(IceInternal::wouldBlock())
            {
                *length = i - data;
                _flags |= SSLWantWrite;
                return errSSLWouldBlock;
            }
            return errno;
        }
        
        i += ret;

        if(packetSize > end - i)
        {
            packetSize = end - i;
        }
    }
    *length = i - data;
    return noErr;
}

OSStatus
IceSSL::TransceiverI::readRaw(char* data, size_t* length) const
{
    _flags &= ~SSLWantRead;

    assert(_fd != INVALID_SOCKET);

    char* i = data;
    int packetSize = *length;
    char* end = i + packetSize;
    while(i != end)
    {
        ssize_t ret = ::recv(_fd, i, packetSize, 0);
        if(ret == 0)
        {
            return errSSLClosedGraceful;
        }
        
        if(ret == SOCKET_ERROR)
        {
            if(IceInternal::interrupted())
            {
                continue;
            }

            if(IceInternal::noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }
            
            if(IceInternal::wouldBlock())
            {
                *length = i - data;
                _flags |= SSLWantRead;
                return errSSLWouldBlock;
            }
            return errno;
        }
        
        i += ret;
        packetSize = end - i;
    }
    
    *length = i - data;
    return noErr;
}

#endif
