// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

    if(!_ssl)
    {
        //
        // This static_cast is necessary due to 64bit windows. There SOCKET is a non-int type.
        //
        BIO* bio = BIO_new_socket(static_cast<int>(_stream->fd()), 0);
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
            {
                assert(SSL_is_init_finished(_ssl));
                break;
            }
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
                if(IceInternal::fdToRemoteAddress(_stream->fd(), remoteAddr))
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
    _engine->verifyPeer(_stream->fd(), _host, getNativeConnectionInfo());

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
        return IceInternal::SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    while(buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
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
    if(!_stream->isConnected())
    {
        return _stream->read(buf);
    }

    //
    // Note: we don't set the hasMoreData flag in this implementation.
    // We assume that OpenSSL doesn't read more SSL records than
    // necessary to fill the requested data and that the sender sends
    // Ice messages in individual SSL records.
    //

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
                ex.reason = "SSL protocol error during read:\n" + _engine->sslErrors();
                throw ex;
            }
            }
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
    return getNativeConnectionInfo();
}

void
IceSSL::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance, const IceInternal::StreamSocketPtr& stream,
                                   const string& hostOrAdapterName, bool incoming) :
    _instance(instance),
    _engine(OpenSSLEnginePtr::dynamicCast(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _stream(stream),
    _ssl(0)
{
}

IceSSL::TransceiverI::~TransceiverI()
{
}

NativeConnectionInfoPtr
IceSSL::TransceiverI::getNativeConnectionInfo() const
{
    NativeConnectionInfoPtr info = new NativeConnectionInfo();
    IceInternal::fdToAddressAndPort(_stream->fd(), info->localAddress, info->localPort, info->remoteAddress, 
                                    info->remotePort);

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

#endif
