//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceSSL/OpenSSLTransceiverI.h>
#include <IceSSL/OpenSSLEngine.h>

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
#include <Ice/Network.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

//
// BUGFIX: an openssl bug that affects OpenSSL < 1.0.0k
// could cause a deadlock when decoding public keys.
//
// See: http://cvs.openssl.org/chngview?cn=22569
//
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x100000bfL && !defined(LIBRESSL_VERSION_NUMBER)
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
    OpenSSL::TransceiverI* p = reinterpret_cast<OpenSSL::TransceiverI*>(SSL_get_ex_data(ssl, 0));
    return p->verifyCallback(ok, ctx);
}

}

IceInternal::NativeInfoPtr
OpenSSL::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
OpenSSL::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
    if(!_connected)
    {
        IceInternal::SocketOperation status = _delegate->initialize(readBuffer, writeBuffer);
        if(status != IceInternal::SocketOperationNone)
        {
            return status;
        }
        _connected = true;
    }

    if(!_ssl)
    {
        SOCKET fd = _delegate->getNativeInfo()->fd();
        BIO* bio = 0;
        if(fd == INVALID_SOCKET)
        {
            assert(_sentBytes == 0);
            _maxSendPacketSize = 128 * 1024; // 128KB
            _maxRecvPacketSize = 128 * 1024; // 128KB
            if(!BIO_new_bio_pair(&bio, _maxSendPacketSize, &_memBio, _maxRecvPacketSize))
            {
                bio = 0;
                _memBio = 0;
            }
        }
        else
        {
#ifdef ICE_USE_IOCP
            assert(_sentBytes == 0);
            _maxSendPacketSize = std::max(512, IceInternal::getSendBufferSize(fd));
            _maxRecvPacketSize = std::max(512, IceInternal::getRecvBufferSize(fd));
            if(!BIO_new_bio_pair(&bio, _maxSendPacketSize, &_memBio, _maxRecvPacketSize))
            {
                bio = 0;
                _memBio = 0;
            }
#else
            bio = BIO_new_socket(fd, 0);
#endif
        }

        if(!bio)
        {
            throw SecurityException(__FILE__, __LINE__, "openssl failure");
        }

        _ssl = SSL_new(_engine->context());
        if(!_ssl)
        {
            BIO_free(bio);
            if(_memBio)
            {
                BIO_free(_memBio);
                _memBio = 0;
            }
            throw SecurityException(__FILE__, __LINE__, "openssl failure");
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

            //
            // Hostname verification was included in OpenSSL 1.0.2
            //
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x10002000L
            if(_engine->getCheckCertName() && !_host.empty() && (sslVerifyMode & SSL_VERIFY_PEER))
            {
                X509_VERIFY_PARAM* param = SSL_get0_param(_ssl);
                if(IceInternal::isIpAddress(_host))
                {
                    X509_VERIFY_PARAM_set1_ip_asc(param, _host.c_str());
                }
                else
                {
                    X509_VERIFY_PARAM_set1_host(param, _host.c_str(), 0);
                }
            }
#endif

            SSL_set_verify(_ssl, sslVerifyMode, IceSSL_opensslVerifyCallback);
        }
    }

    while(!SSL_is_init_finished(_ssl))
    {
        //
        // Only one thread calls initialize(), so synchronization is not necessary here.
        //

        //
        // BUGFIX: an openssl bug that affects OpenSSL < 1.0.0k
        // could cause a deadlock when decoding public keys.
        //
        // See: http://cvs.openssl.org/chngview?cn=22569
        //
#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x100000bfL && !defined(LIBRESSL_VERSION_NUMBER)
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(sslMutex);
#endif

        int ret = _incoming ? SSL_accept(_ssl) : SSL_connect(_ssl);

#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x100000bfL && !defined(LIBRESSL_VERSION_NUMBER)
        sync.release();
#endif

        if(_memBio && BIO_ctrl_pending(_memBio))
        {
            if(!send())
            {
                return IceInternal::SocketOperationWrite;
            }
            continue;
        }

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
                throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            case SSL_ERROR_WANT_READ:
            {
                if(_memBio && receive())
                {
                    continue;
                }
                return IceInternal::SocketOperationRead;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                if(_memBio && send())
                {
                    continue;
                }
                return IceInternal::SocketOperationWrite;
            }
            case SSL_ERROR_SYSCALL:
            {
                if(!_memBio)
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

                    if(IceInternal::connectionLost() || IceInternal::getSocketErrno() == 0)
                    {
                        throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
                    }
                }
                throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            case SSL_ERROR_SSL:
            {
                ostringstream ostr;
                ostr << "SSL error occurred for new " << (_incoming ? "incoming" : "outgoing")
                     << " connection:\n" << _delegate->toString() << "\n" << _engine->sslErrors();
                throw ProtocolException(__FILE__, __LINE__, ostr.str());
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
            const string msg = ostr.str();
            if(_engine->securityTraceLevel() >= 1)
            {
                _instance->logger()->trace(_instance->traceCategory(), msg);
            }
            throw SecurityException(__FILE__, __LINE__,  msg);
        }
    }
    else
    {
        _verified = true;
    }

    _cipher = SSL_get_cipher_name(_ssl); // Nothing needs to be free'd.
    _engine->verifyPeer(_host, ICE_DYNAMIC_CAST(ConnectionInfo, getInfo()), toString());

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
        out << toString();
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
OpenSSL::TransceiverI::closing(bool initiator, const Ice::LocalException&)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
OpenSSL::TransceiverI::close()
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

    if(_memBio)
    {
        BIO_free(_memBio);
        _memBio = 0;
    }

    _delegate->close();
}

IceInternal::SocketOperation
OpenSSL::TransceiverI::write(IceInternal::Buffer& buf)
{
    if(!_connected)
    {
        return _delegate->write(buf);
    }

    if(_memBio && _writeBuffer.i != _writeBuffer.b.end())
    {
        if(!send())
        {
            return IceInternal::SocketOperationWrite;
        }
    }

    if(buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = _memBio ?
        std::min(static_cast<int>(_maxSendPacketSize), static_cast<int>(buf.b.end() - buf.i)) :
        static_cast<int>(buf.b.end() - buf.i);

    while(buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
        int ret;
        if(_memBio)
        {
            if(_sentBytes)
            {
                ret = _sentBytes;
                _sentBytes = 0;
            }
            else
            {
                ret = SSL_write(_ssl, reinterpret_cast<const void*>(&*buf.i), packetSize);
                if(ret > 0)
                {
                    if(!send())
                    {
                        _sentBytes = ret;
                        return IceInternal::SocketOperationWrite;
                    }
                }
            }
        }
        else
        {
            ret = SSL_write(_ssl, reinterpret_cast<const void*>(&*buf.i), packetSize);
        }

        if(ret <= 0)
        {
            switch(SSL_get_error(_ssl, ret))
            {
            case SSL_ERROR_NONE:
                assert(false);
                break;
            case SSL_ERROR_ZERO_RETURN:
            {
                throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            case SSL_ERROR_WANT_READ:
            {
                assert(false);
                break;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                if(_memBio && send())
                {
                    continue;
                }
                return IceInternal::SocketOperationWrite;
            }
            case SSL_ERROR_SYSCALL:
            {
                if(!_memBio)
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
                }

                if(IceInternal::connectionLost() || IceInternal::getSocketErrno() == 0)
                {
                    throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
                }
                else
                {
                    throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
                }
            }
            case SSL_ERROR_SSL:
            {
                throw ProtocolException(__FILE__, __LINE__,
                                        "SSL protocol error during write:\n" + _engine->sslErrors());
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
OpenSSL::TransceiverI::read(IceInternal::Buffer& buf)
{
    if(!_connected)
    {
        return _delegate->read(buf);
    }

    if(_memBio && _readBuffer.i != _readBuffer.b.end())
    {
        if(!receive())
        {
            return IceInternal::SocketOperationRead;
        }
    }

    //
    // Note: We assume that OpenSSL doesn't read more SSL records than
    // necessary to fill the requested data and that the sender sends
    // Ice messages in individual SSL records.
    //

    if(buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }

    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);

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
#if defined(_AIX)
            //
            // WORKAROUND: OpenSSL SSL_read on AIX sometime ends up reporting a error
            // with SSL_get_error but there's no error in the error queue. This occurs
            // when SSL_read needs more data. So we just return SocketOperationRead in
            // this case.
            //
            if(SSL_get_error(_ssl, ret) == SSL_ERROR_SSL && ERR_peek_error() == 0)
            {
                if(SSL_want_read(_ssl))
                {
                    return IceInternal::SocketOperationRead;
                }
            }
#endif
            switch(SSL_get_error(_ssl, ret))
            {
            case SSL_ERROR_NONE:
            {
                assert(false);
                break;
            }
            case SSL_ERROR_ZERO_RETURN:
            {
                throw ConnectionLostException(__FILE__, __LINE__, 0);
            }
            case SSL_ERROR_WANT_READ:
            {
                if(_memBio && receive())
                {
                    continue;
                }
                return IceInternal::SocketOperationRead;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                assert(false);
                break;
            }
            case SSL_ERROR_SYSCALL:
            {
                if(!_memBio)
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
                }

                if(IceInternal::connectionLost() || IceInternal::getSocketErrno() == 0)
                {
                    throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
                }
                else
                {
                    throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
                }
            }
            case SSL_ERROR_SSL:
            {
                throw ProtocolException(__FILE__, __LINE__,
                                        "SSL protocol error during read:\n" + _engine->sslErrors());
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
    // Check if there's still buffered data to read, set the read ready status.
    //
    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, SSL_pending(_ssl) > 0);

    return IceInternal::SocketOperationNone;
}

#ifdef ICE_USE_IOCP

bool
OpenSSL::TransceiverI::startWrite(IceInternal::Buffer& buffer)
{
    if(!_connected)
    {
        return _delegate->startWrite(buffer);
    }

    if(_writeBuffer.i == _writeBuffer.b.end())
    {
        assert(_sentBytes == 0);
        int packetSize = std::min(static_cast<int>(_maxSendPacketSize), static_cast<int>(buffer.b.end() - buffer.i));
        _sentBytes = SSL_write(_ssl, reinterpret_cast<void*>(&*buffer.i), packetSize);

        assert(BIO_ctrl_pending(_memBio));
        _writeBuffer.b.resize( BIO_ctrl_pending(_memBio));
        _writeBuffer.i = _writeBuffer.b.begin();
        BIO_read(_memBio, _writeBuffer.i, static_cast<int>(_writeBuffer.b.size()));
    }

    return _delegate->startWrite(_writeBuffer) && buffer.i == buffer.b.end();
}

void
OpenSSL::TransceiverI::finishWrite(IceInternal::Buffer& buffer)
{
    if(!_connected)
    {
        _delegate->finishWrite(buffer);
        return;
    }

    _delegate->finishWrite(_writeBuffer);
    if(_sentBytes)
    {
        buffer.i += _sentBytes;
        _sentBytes = 0;
    }
}

void
OpenSSL::TransceiverI::startRead(IceInternal::Buffer& buffer)
{
    if(!_connected)
    {
        _delegate->startRead(buffer);
        return;
    }

    if(_readBuffer.i == _readBuffer.b.end())
    {
        assert(!buffer.b.empty() && buffer.i != buffer.b.end());
        ERR_clear_error(); // Clear any spurious errors.
#ifndef NDEBUG
        int ret =
#endif
            SSL_read(_ssl, reinterpret_cast<void*>(&*buffer.i), static_cast<int>(buffer.b.end() - buffer.i));
        assert(ret <= 0 && SSL_get_error(_ssl, ret) == SSL_ERROR_WANT_READ);

        assert(BIO_ctrl_get_read_request(_memBio));
        _readBuffer.b.resize(BIO_ctrl_get_read_request(_memBio));
        _readBuffer.i = _readBuffer.b.begin();
    }

    assert(!_readBuffer.b.empty() && _readBuffer.i != _readBuffer.b.end());

    _delegate->startRead(_readBuffer);
}

void
OpenSSL::TransceiverI::finishRead(IceInternal::Buffer& buffer)
{
    if(!_connected)
    {
        _delegate->finishRead(buffer);
        return;
    }

    _delegate->finishRead(_readBuffer);
    if(_readBuffer.i == _readBuffer.b.end())
    {
        int n = BIO_write(_memBio, _readBuffer.b.begin(), static_cast<int>(_readBuffer.b.size()));
        if(n < 0) // Expected if the transceiver was closed.
        {
            throw SecurityException(__FILE__, __LINE__, "SSL bio write failed");
        }

        assert(n == static_cast<int>(_readBuffer.b.size()));
        ERR_clear_error(); // Clear any spurious errors.
        int ret = SSL_read(_ssl, reinterpret_cast<void*>(&*buffer.i), static_cast<int>(buffer.b.end() - buffer.i));
        if(ret <= 0)
        {
            switch(SSL_get_error(_ssl, ret))
            {
                case SSL_ERROR_NONE:
                case SSL_ERROR_WANT_WRITE:
                {
                    assert(false);
                    return;
                }
                case SSL_ERROR_ZERO_RETURN:
                {
                    throw ConnectionLostException(__FILE__, __LINE__, 0);
                }
                case SSL_ERROR_WANT_READ:
                {
                    return;
                }
                case SSL_ERROR_SYSCALL:
                {
                    if(IceInternal::connectionLost() || IceInternal::getSocketErrno() == 0)
                    {
                        throw ConnectionLostException(__FILE__, __LINE__,  IceInternal::getSocketErrno());
                    }
                    else
                    {
                        throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
                    }
                }
                case SSL_ERROR_SSL:
                {
                    throw ProtocolException(__FILE__, __LINE__,
                                            "SSL protocol error during read:\n" + _engine->sslErrors());
                }
            }
        }
        buffer.i += ret;
    }
}
#endif

string
OpenSSL::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
OpenSSL::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
OpenSSL::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
OpenSSL::TransceiverI::getInfo() const
{
    ConnectionInfoPtr info = ICE_MAKE_SHARED(ConnectionInfo);
    info->underlying = _delegate->getInfo();
    info->incoming = _incoming;
    info->adapterName = _adapterName;
    info->cipher = _cipher;
    info->certs = _certs;
    info->verified = _verified;
    return info;
}

void
OpenSSL::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
OpenSSL::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

int
OpenSSL::TransceiverI::verifyCallback(int ok, X509_STORE_CTX* c)
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
        out << toString();
    }

    //
    // Initialize the native certs with the verified certificate chain. SSL_get_peer_cert_chain
    // doesn't return the verified chain, it returns the chain sent by the peer.
    //
    STACK_OF(X509)* chain = X509_STORE_CTX_get1_chain(c);
    if(chain != 0)
    {
        _certs.clear();
        for(int i = 0; i < sk_X509_num(chain); ++i)
        {
            CertificatePtr cert = OpenSSL::Certificate::create(X509_dup(sk_X509_value(chain, i)));
            _certs.push_back(cert);
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

OpenSSL::TransceiverI::TransceiverI(const InstancePtr& instance,
                                    const IceInternal::TransceiverPtr& delegate,
                                    const string& hostOrAdapterName,
                                    bool incoming) :
    _instance(instance),
    _engine(OpenSSL::SSLEnginePtr::dynamicCast(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _delegate(delegate),
    _connected(false),
    _verified(false),
    _ssl(0),
    _memBio(0),
    _sentBytes(0),
    _maxSendPacketSize(0),
    _maxRecvPacketSize(0)
{
}

OpenSSL::TransceiverI::~TransceiverI()
{
}

bool
OpenSSL::TransceiverI::receive()
{
    if(_readBuffer.i == _readBuffer.b.end())
    {
        assert(BIO_ctrl_get_read_request(_memBio));
        _readBuffer.b.resize(BIO_ctrl_get_read_request(_memBio));
        _readBuffer.i = _readBuffer.b.begin();
    }

    while(_readBuffer.i != _readBuffer.b.end())
    {
        if(_delegate->read(_readBuffer) != IceInternal::SocketOperationNone)
        {
            return false;
        }
    }

    assert(_readBuffer.i == _readBuffer.b.end());

#ifndef NDEBUG
    int n =
#endif
        BIO_write(_memBio, &_readBuffer.b[0], static_cast<int>(_readBuffer.b.end() - _readBuffer.b.begin()));

    assert(n == static_cast<int>(_readBuffer.b.end() - _readBuffer.b.begin()));

    return true;
}

bool
OpenSSL::TransceiverI::send()
{
    if(_writeBuffer.i == _writeBuffer.b.end())
    {
        assert(BIO_ctrl_pending(_memBio));
        _writeBuffer.b.resize( BIO_ctrl_pending(_memBio));
        _writeBuffer.i = _writeBuffer.b.begin();
        BIO_read(_memBio, _writeBuffer.i, static_cast<int>(_writeBuffer.b.size()));
    }

    if(_writeBuffer.i != _writeBuffer.b.end())
    {
        if(_delegate->write(_writeBuffer) != IceInternal::SocketOperationNone)
        {
            return false;
        }
    }
    return _writeBuffer.i == _writeBuffer.b.end();
}
