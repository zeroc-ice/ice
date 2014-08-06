// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/OpenSSLTransceiverI.h>

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

#include <IceUtil/DisableWarnings.h>

#ifdef ICE_USE_OPENSSL

#include <openssl/err.h>
#include <openssl/bio.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

//
// BUGFIX: an openssl bug that affects OpensSSL < 1.0.0k
// could cause a deadlock when decoding public keys.
//
// See: http://cvs.openssl.org/chngview?cn=22569
//
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x100000bfL
namespace
{

IceUtil::Mutex* sslMutex = 0;

class Init
{
public:

    Init()
    {
        sslMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete sslMutex;
        sslMutex = 0;
    }
};

Init init;

}
#endif

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

        if(!_ssl)
        {
            //
            // This static_cast is necessary due to 64bit windows. There SOCKET is a non-int type.
            //
            BIO* bio = BIO_new_socket(static_cast<int>(_fd), 0);
            if(!bio)
            {
                SecurityException ex(__FILE__, __LINE__);
                ex.reason = "openssl failure";
                throw ex;
            }

            _ssl = SSL_new(_engine->context());
            if(!_ssl)
            {
                BIO_free(bio);
                SecurityException ex(__FILE__, __LINE__);
                ex.reason = "openssl failure";
                throw ex;
            }
            SSL_set_bio(_ssl, bio, bio);
        }

        while(!SSL_is_init_finished(_ssl))
        {
            //
            // Only one thread calls initialize(), so synchronization is not necessary here.
            //

            //
            // BUGFIX: an openssl bug that affects OpensSSL < 1.0.0k
            // could cause a deadlock when decoding public keys.
            //
            // See: http://cvs.openssl.org/chngview?cn=22569
            //
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x100000bfL
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(sslMutex);
#endif

            int ret = _incoming ? SSL_accept(_ssl) : SSL_connect(_ssl);

#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x100000bfL
            sync.release();
#endif

            if(ret <= 0)
            {
                switch(SSL_get_error(_ssl, ret))
                {
                case SSL_ERROR_NONE:
                    assert(SSL_is_init_finished(_ssl));
                    break;
                case SSL_ERROR_ZERO_RETURN:
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = IceInternal::getSocketErrno();
                    throw ex;
                }
                case SSL_ERROR_WANT_READ:
                {
                    return IceInternal::SocketOperationRead;
                }
                case SSL_ERROR_WANT_WRITE:
                {
                    return IceInternal::SocketOperationWrite;
                }
                case SSL_ERROR_SYSCALL:
                {
                    if(ret == 0)
                    {
                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = 0;
                        throw ex;
                    }

                    if(ret == -1)
                    {
                        if(IceInternal::interrupted())
                        {
                            break;
                        }

                        if(IceInternal::wouldBlock())
                        {
                            if(SSL_want_read(_ssl))
                            {
                                return IceInternal::SocketOperationRead;
                            }
                            else if(SSL_want_write(_ssl))
                            {
                                return IceInternal::SocketOperationWrite;
                            }

                            break;
                        }

                        if(IceInternal::connectionLost())
                        {
                            ConnectionLostException ex(__FILE__, __LINE__);
                            ex.error = IceInternal::getSocketErrno();
                            throw ex;
                        }
                    }
                    SocketException ex(__FILE__, __LINE__);
                    ex.error = IceInternal::getSocketErrno();
                    throw ex;
                }
                case SSL_ERROR_SSL:
                {
                    IceInternal::Address remoteAddr;
                    string desc = "<not available>";
                    if(IceInternal::fdToRemoteAddress(_fd, remoteAddr))
                    {
                        desc = IceInternal::addrToString(remoteAddr);
                    }
                    ostringstream ostr;
                    ostr << "SSL error occurred for new " << (_incoming ? "incoming" : "outgoing")
                         << " connection:\nremote address = " << desc << "\n" << _engine->sslErrors();
                    ProtocolException ex(__FILE__, __LINE__);
                    ex.reason = ostr.str();
                    throw ex;
                }
                }
            }
        }

        long result = SSL_get_verify_result(_ssl);
        if(result != X509_V_OK)
        {
            if(_engine->getVerifyPeer() == 0)
            {
                if(_engine->securityTraceLevel() >= 1)
                {
                    ostringstream ostr;
                    ostr << "IceSSL: ignoring certificate verification failure:\n" 
                         << X509_verify_cert_error_string(result);
                    _instance->logger()->trace(_instance->traceCategory(), ostr.str());
                }
            }
            else
            {
                ostringstream ostr;
                ostr << "IceSSL: certificate verification failed:\n" << X509_verify_cert_error_string(result);
                string msg = ostr.str();
                if(_engine->securityTraceLevel() >= 1)
                {
                    _instance->logger()->trace(_instance->traceCategory(), msg);
                }
                SecurityException ex(__FILE__, __LINE__);
                ex.reason = msg;
                throw ex;
            }
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

    if(_engine->securityTraceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "SSL summary for " << (_incoming ? "incoming" : "outgoing") << " connection\n";

        //
        // The const_cast is necesary because Solaris still uses OpenSSL 0.9.7.
        //
        //const SSL_CIPHER *cipher = SSL_get_current_cipher(ssl);
        SSL_CIPHER *cipher = const_cast<SSL_CIPHER*>(SSL_get_current_cipher(_ssl));
        if(!cipher)
        {
            out << "unknown cipher\n";
        }
        else
        {
            out << "cipher = " << SSL_CIPHER_get_name(cipher) << "\n";
            out << "bits = " << SSL_CIPHER_get_bits(cipher, 0) << "\n";
            out << "protocol = " << SSL_get_version(_ssl) << "\n";
        }
        out << IceInternal::fdToString(SSL_get_fd(_ssl));
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

    if(_ssl)
    {
        int err = SSL_shutdown(_ssl);

        //
        // Call it one more time if it returned 0.
        //
        if(err == 0)
        {
            SSL_shutdown(_ssl);
        }

        SSL_free(_ssl);
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

    if(buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    while(buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
        assert(_fd != INVALID_SOCKET);
        int ret = SSL_write(_ssl, reinterpret_cast<const void*>(&*buf.i), packetSize);
        if(ret <= 0)
        {
            switch(SSL_get_error(_ssl, ret))
            {
            case SSL_ERROR_NONE:
                assert(false);
                break;
            case SSL_ERROR_ZERO_RETURN:
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            case SSL_ERROR_WANT_READ:
            {
                assert(false);
                break;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                return IceInternal::SocketOperationWrite;
            }
            case SSL_ERROR_SYSCALL:
            {
                if(ret == -1)
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
                        assert(SSL_want_write(_ssl));
                        return IceInternal::SocketOperationWrite;
                    }

                    if(IceInternal::connectionLost())
                    {
                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = IceInternal::getSocketErrno();
                        throw ex;
                    }
                }
                if(ret == 0)
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }

                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            case SSL_ERROR_SSL:
            {
                ProtocolException ex(__FILE__, __LINE__);
                ex.reason = "SSL protocol error during write:\n" + _engine->sslErrors();
                throw ex;
            }
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
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::read(IceInternal::Buffer& buf, bool&)
{
    //
    // Note: we don't set the hasMoreData flag in this implementation.
    // We assume that OpenSSL doesn't read more SSL records than
    // necessary to fill the requested data and that the sender sends
    // Ice messages in individual SSL records.
    // 

    if(_state == StateProxyConnectRequestPending)
    {
        //
        // We need to read the proxy reply, but we have to use TCP and not SSL.
        //
        return readRaw(buf) ? IceInternal::SocketOperationNone : IceInternal::SocketOperationRead;
    }

    if(buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    while(buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
        assert(_fd != INVALID_SOCKET);
        int ret = SSL_read(_ssl, reinterpret_cast<void*>(&*buf.i), packetSize);
        if(ret <= 0)
        {
            switch(SSL_get_error(_ssl, ret))
            {
            case SSL_ERROR_NONE:
            {
                assert(false);
                break;
            }
            case SSL_ERROR_ZERO_RETURN:
            {
                //
                // If the connection is lost when reading data, we shut
                // down the write end of the socket. This helps to unblock
                // threads that are stuck in send() or select() while
                // sending data. Note: I don't really understand why
                // send() or select() sometimes don't detect a connection
                // loss. Therefore this helper to make them detect it.
                //
                //assert(_fd != INVALID_SOCKET);
                //shutdownSocketReadWrite(_fd);

                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = 0;
                throw ex;
            }
            case SSL_ERROR_WANT_READ:
            {
                return IceInternal::SocketOperationRead;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                assert(false);
                break;
            }
            case SSL_ERROR_SYSCALL:
            {
                if(ret == -1)
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
                        assert(SSL_want_read(_ssl));
                        return IceInternal::SocketOperationRead;
                    }

                    if(IceInternal::connectionLost())
                    {
                        //
                        // See the commment above about shutting down the
                        // socket if the connection is lost while reading
                        // data.
                        //
                        //assert(_fd != INVALID_SOCKET);
                        //shutdownSocketReadWrite(_fd);

                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = IceInternal::getSocketErrno();
                        throw ex;
                    }
                }

                if(ret == 0)
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }

                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            case SSL_ERROR_SSL:
            {
                //
                // Forcefully closing a connection can result in SSL_read reporting
                // "decryption failed or bad record mac". We trap that error and
                // treat it as the loss of a connection.
                //
                // NOTE: We have to compare the reason string instead
                // of the error codes because the error code values
                // changed between OpenSSL 0.9.7i and 0.9.7j and
                // between OpenSSL 0.9.8a and 0.9.8b...
                //
                //unsigned long e = ERR_peek_error();
                //if(ERR_GET_LIB(e) == ERR_LIB_SSL && ERR_GET_REASON(e) == SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC)
                //
                unsigned long e = ERR_peek_error();
                const char* estr = ERR_GET_LIB(e) == ERR_LIB_SSL ? ERR_reason_error_string(e) : 0;
                if(estr && strcmp(estr, "decryption failed or bad record mac") == 0)
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }
                else
                {
                    ProtocolException ex(__FILE__, __LINE__);
                    ex.reason = "SSL protocol error during read:\n" + _engine->sslErrors();
                    throw ex;
                }
            }
            }
        }

        if(_instance->traceLevel() >= 3)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "received " << ret << " of " << packetSize << " bytes via " << protocol() << "\n" << toString();
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = static_cast<int>(buf.b.end() - buf.i);
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
    _engine(OpenSSLEnginePtr::dynamicCast(instance->engine())),
    _proxy(proxy),
    _host(host),
    _addr(addr),
    _incoming(false),
    _ssl(0),
    _state(StateNeedConnect)
{
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
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance, SOCKET fd, const string& adapterName) :
    IceInternal::NativeInfo(fd),
    _instance(instance),
    _engine(OpenSSLEnginePtr::dynamicCast(instance->engine())),
    _addr(IceInternal::Address()),
    _adapterName(adapterName),
    _incoming(true),
    _ssl(0),
    _state(StateConnected),
    _desc(IceInternal::fdToString(fd))
{
    IceInternal::setBlock(fd, false);
    IceInternal::setTcpBufSize(fd, _instance->properties(), _instance->logger());
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

    if(_ssl != 0)
    {
        //
        // On the client side, SSL_get_peer_cert_chain returns the entire chain of certs.
        // On the server side, the peer certificate must be obtained separately.
        //
        // Since we have no clear idea whether the connection is server or client side,
        // the peer certificate is obtained separately and compared against the first
        // certificate in the chain. If they are not the same, it is added to the chain.
        //
        X509* cert = SSL_get_peer_certificate(_ssl);
        STACK_OF(X509)* chain = SSL_get_peer_cert_chain(_ssl);
        if(cert != 0 && (chain == 0 || sk_X509_num(chain) == 0 || cert != sk_X509_value(chain, 0)))
        {
            CertificatePtr certificate = new Certificate(cert);
            info->nativeCerts.push_back(certificate);
            info->certs.push_back(certificate->encode());
        }
        else
        {
            X509_free(cert);
        }

        if(chain != 0)
        {
            for(int i = 0; i < sk_X509_num(chain); ++i)
            {
                //
                // Duplicate the certificate since the stack comes straight from the SSL connection.
                //
                CertificatePtr certificate = new Certificate(X509_dup(sk_X509_value(chain, i)));
                info->nativeCerts.push_back(certificate);
                info->certs.push_back(certificate->encode());
            }
        }

        info->cipher = SSL_get_cipher_name(_ssl); // Nothing needs to be free'd.
    }

    info->adapterName = _adapterName;
    info->incoming = _incoming;
    return info;
}

bool
IceSSL::TransceiverI::writeRaw(IceInternal::Buffer& buf)
{
    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
#ifdef ICE_USE_IOCP
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
    {
        packetSize = _maxSendPacketSize;
    }
#endif
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
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }

    return true;
}

bool
IceSSL::TransceiverI::readRaw(IceInternal::Buffer& buf)
{
    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
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

        packetSize = static_cast<int>(buf.b.end() - buf.i);
    }

    return true;
}
#endif
