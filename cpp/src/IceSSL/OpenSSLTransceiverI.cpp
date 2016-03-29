// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

extern "C"
{

int
IceSSL_opensslVerifyCallback(int ok, X509_STORE_CTX* ctx)
{
    SSL* ssl = reinterpret_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    TransceiverI* p = reinterpret_cast<TransceiverI*>(SSL_get_ex_data(ssl, 0));
    return p->verifyCallback(ok, ctx);
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

        //
        // Store a pointer to ourself for use in OpenSSL callbacks.
        //
        SSL_set_ex_data(_ssl, 0, this);

        //
        // Determine whether a certificate is required from the peer.
        //
        {
            int sslVerifyMode = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
            switch(_engine->getVerifyPeer())
            {
                case 0:
                    sslVerifyMode = SSL_VERIFY_NONE;
                    break;
                case 1:
                    sslVerifyMode = SSL_VERIFY_PEER;
                    break;
                case 2:
                    sslVerifyMode = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
                    break;
                default:
                {
                    assert(false);
                }
            }
            SSL_set_verify(_ssl, sslVerifyMode, IceSSL_opensslVerifyCallback);
        }
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
                ostr << "IceSSL: ignoring certificate verification failure:\n" << X509_verify_cert_error_string(result);
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
    else
    {
        _verified = true;
    }

    _engine->verifyPeer(_stream->fd(), _host, NativeConnectionInfoPtr::dynamicCast(getInfo()));

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
IceSSL::TransceiverI::read(IceInternal::Buffer& buf, bool& hasMoreData)
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

    //
    // Check if there's still buffered data to read. In this case, set hasMoreData to true.
    //
    hasMoreData = SSL_pending(_ssl) > 0;

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

int
IceSSL::TransceiverI::verifyCallback(int ok, X509_STORE_CTX* c)
{
    if(!ok && _engine->securityTraceLevel() >= 1)
    {
        X509* cert = X509_STORE_CTX_get_current_cert(c);
        int err = X509_STORE_CTX_get_error(c);
        char buf[256];

        Trace out(_engine->getLogger(), _engine->securityTraceCategory());
        out << "certificate verification failure\n";

        X509_NAME_oneline(X509_get_issuer_name(cert), buf, static_cast<int>(sizeof(buf)));
        out << "issuer = " << buf << '\n';
        X509_NAME_oneline(X509_get_subject_name(cert), buf, static_cast<int>(sizeof(buf)));
        out << "subject = " << buf << '\n';
        out << "depth = " << X509_STORE_CTX_get_error_depth(c) << '\n';
        out << "error = " << X509_verify_cert_error_string(err) << '\n';
        out << IceInternal::fdToString(SSL_get_fd(_ssl));
    }

    //
    // Initialize the native certs with the verified certificate chain. SSL_get_peer_cert_chain
    // doesn't return the verified chain, it returns the chain sent by the peer.
    //
    STACK_OF(X509)* chain = X509_STORE_CTX_get1_chain(c);
    if(chain != 0)
    {
        _nativeCerts.clear();
        for(int i = 0; i < sk_X509_num(chain); ++i)
        {
            _nativeCerts.push_back(new Certificate(X509_dup(sk_X509_value(chain, i))));
        }
        sk_X509_pop_free(chain, X509_free);
    }

    //
    // Always return 1 to prevent SSL_connect/SSL_accept from
    // returning SSL_ERROR_SSL for verification failures. This ensure
    // that we can raise SecurityException for verification failures
    // rather than a ProtocolException.
    //
    return 1;
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance, const IceInternal::StreamSocketPtr& stream,
                                   const string& hostOrAdapterName, bool incoming) :
    _instance(instance),
    _engine(OpenSSLEnginePtr::dynamicCast(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _stream(stream),
    _verified(false),
    _ssl(0)
{
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
    info->adapterName = _adapterName;
    info->incoming = _incoming;
    info->verified = _verified;
    nativeCerts = _nativeCerts;
    for(vector<CertificatePtr>::const_iterator p = _nativeCerts.begin(); p != _nativeCerts.end(); ++p)
    {
        info->certs.push_back((*p)->encode());
    }
    if(_ssl != 0)
    {
        info->cipher = SSL_get_cipher_name(_ssl); // Nothing needs to be free'd.
    }
    info->adapterName = _adapterName;
    info->incoming = _incoming;
}

#endif
