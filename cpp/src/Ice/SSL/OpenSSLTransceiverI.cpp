// Copyright (c) ZeroC, Inc.

// Default to OpenSSL 3.0.0 compatibility (decimal mmnnpp format)
// Hide deprecated APIs unless the user explicitly wants them
#define OPENSSL_API_COMPAT 30000
#define OPENSSL_NO_DEPRECATED

#include "OpenSSLTransceiverI.h"
#include "Ice/Buffer.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/SSL/ConnectionInfo.h"
#include "OpenSSLEngine.h"
#include "SSLEngine.h"
#include "SSLInstance.h"
#include "SSLUtil.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <iostream>
#include <mutex>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

extern "C"
{
    int Ice_SSL_opensslVerifyCallback(int ok, X509_STORE_CTX* ctx)
    {
        auto* ssl = reinterpret_cast<::SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
        auto* p = reinterpret_cast<OpenSSL::TransceiverI*>(SSL_get_ex_data(ssl, 0));
        return p->verifyCallback(ok, ctx);
    }
}

namespace
{
    bool defaultVerificationCallback(bool ok, X509_STORE_CTX*, const Ice::SSL::ConnectionInfoPtr&) { return ok; }

    SSL_CTX* defaultSSLContextSelectionCallback(const string&)
    {
        SSL_CTX* defaultSSLContext = SSL_CTX_new(TLS_method());
        SSL_CTX_set_default_verify_paths(defaultSSLContext);
        return defaultSSLContext;
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
    if (!_connected)
    {
        IceInternal::SocketOperation status = _delegate->initialize(readBuffer, writeBuffer);
        if (status != IceInternal::SocketOperationNone)
        {
            return status;
        }
        _connected = true;
    }

    if (!_ssl)
    {
        SOCKET fd = _delegate->getNativeInfo()->fd();
        BIO* bio = BIO_new_socket(fd, 0);
        if (!bio)
        {
            throw SecurityException(__FILE__, __LINE__, "openssl failure");
        }

        _sslCtx = _localSSLContextSelectionCallback(_incoming ? _adapterName : _host);
        if (!_sslCtx)
        {
            throw SecurityException(__FILE__, __LINE__, "SSL error: the SSL context selection callback returned null");
        }
        _ssl = SSL_new(_sslCtx);
        if (!_ssl)
        {
            BIO_free(bio);
            throw SecurityException(__FILE__, __LINE__, "openssl failure");
        }
        SSL_set_bio(_ssl, bio, bio);

        SSL_set_ex_data(_ssl, 0, this);
        SSL_set_verify(_ssl, SSL_get_verify_mode(_ssl), Ice_SSL_opensslVerifyCallback);
        // Enable SNI by default for outgoing connections. The SNI host name is always empty for incoming connections.
        if (!_host.empty() && !IceInternal::isIpAddress(_host) && !SSL_set_tlsext_host_name(_ssl, _host.c_str()))
        {
            ostringstream os;
            os << "IceSSL: setting SNI host failed '" << _host << "'";
            throw SecurityException(__FILE__, __LINE__, os.str());
        }

        if (_sslNewSessionCallback)
        {
            _sslNewSessionCallback(_ssl, _incoming ? _adapterName : _host);
        }
    }

    while (!SSL_is_init_finished(_ssl))
    {
        int ret = _incoming ? SSL_accept(_ssl) : SSL_connect(_ssl);

        if (ret <= 0)
        {
            switch (SSL_get_error(_ssl, ret))
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
                    return IceInternal::SocketOperationRead;
                }
                case SSL_ERROR_WANT_WRITE:
                {
                    return IceInternal::SocketOperationWrite;
                }
                case SSL_ERROR_SYSCALL:
                {
                    if (IceInternal::interrupted())
                    {
                        break;
                    }

                    if (IceInternal::wouldBlock())
                    {
                        if (SSL_want_read(_ssl))
                        {
                            return IceInternal::SocketOperationRead;
                        }
                        else if (SSL_want_write(_ssl))
                        {
                            return IceInternal::SocketOperationWrite;
                        }

                        break;
                    }

                    if (IceInternal::connectionLost() || IceInternal::getSocketErrno() == 0)
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
#if defined(SSL_R_UNEXPECTED_EOF_WHILE_READING)
                    if (SSL_R_UNEXPECTED_EOF_WHILE_READING == ERR_GET_REASON(ERR_peek_error()))
                    {
                        throw ConnectionLostException(__FILE__, __LINE__);
                    }
                    else
                    {
#endif
                        ostringstream os;
                        os << "SSL error occurred for new " << (_incoming ? "incoming" : "outgoing") << " connection:\n"
                           << _delegate->toString() << "\n"
                           << _engine->sslErrors();
                        throw ProtocolException(__FILE__, __LINE__, os.str());
#if defined(SSL_R_UNEXPECTED_EOF_WHILE_READING)
                    }
#endif
                }
            }
        }
    }

    if (_verificationException)
    {
        rethrow_exception(_verificationException);
    }

    // Retrieve the certificate chain if the verification callback has not already fill it.
    if (!_peerCertificate)
    {
        // When calling on the server side the peer certificate is not included in SSL_get_peer_cert_chain and must be
        // obtained separately using SSL_get1_peer_certificate.
        if (_incoming)
        {
            // SSL_get1_peer_certificate increases the reference count of the certificate before returning it.
            X509* peerCertificate = SSL_get1_peer_certificate(_ssl);
            if (peerCertificate)
            {
                _peerCertificate = peerCertificate;
            }
        }
        else
        {
            STACK_OF(X509)* chain = SSL_get_peer_cert_chain(_ssl);
            if (chain && sk_X509_num(chain) > 0)
            {
                _peerCertificate = X509_dup(sk_X509_value(chain, 0));
            }
        }
    }

    if (_engine->securityTraceLevel() >= 1)
    {
        Trace out(_instance->logger(), _engine->securityTraceCategory());
        out << "SSL summary for " << (_incoming ? "incoming" : "outgoing") << " connection\n";

        const SSL_CIPHER* cipher = SSL_get_current_cipher(_ssl);
        if (!cipher)
        {
            out << "unknown cipher\n";
        }
        else
        {
            out << "cipher = " << SSL_CIPHER_get_name(cipher) << "\n";
            out << "bits = " << SSL_CIPHER_get_bits(cipher, nullptr) << "\n";
            out << "protocol = " << SSL_get_version(_ssl) << "\n";
        }
        out << toString();
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
OpenSSL::TransceiverI::closing(bool initiator, exception_ptr)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
OpenSSL::TransceiverI::close()
{
    if (_ssl)
    {
        int err = SSL_shutdown(_ssl);

        //
        // Call it one more time if it returned 0.
        //
        if (err == 0)
        {
            SSL_shutdown(_ssl);
        }

        SSL_free(_ssl);
        _ssl = nullptr;
    }

    if (_sslCtx)
    {
        SSL_CTX_free(_sslCtx);
        _sslCtx = nullptr;
    }

    if (_peerCertificate)
    {
        X509_free(_peerCertificate);
        _peerCertificate = nullptr;
    }

    _delegate->close();
}

IceInternal::SocketOperation
OpenSSL::TransceiverI::write(IceInternal::Buffer& buf)
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
    int packetSize = static_cast<int>(buf.b.end() - buf.i);

    while (buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
        int ret = SSL_write(_ssl, reinterpret_cast<const void*>(&*buf.i), packetSize);
        if (ret <= 0)
        {
            switch (SSL_get_error(_ssl, ret))
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
                    return IceInternal::SocketOperationWrite;
                }
                case SSL_ERROR_SYSCALL:
                {
                    if (IceInternal::interrupted())
                    {
                        continue;
                    }

                    if (IceInternal::noBuffers() && packetSize > 1024)
                    {
                        packetSize /= 2;
                        continue;
                    }

                    if (IceInternal::wouldBlock())
                    {
                        assert(SSL_want_write(_ssl));
                        return IceInternal::SocketOperationWrite;
                    }

                    if (IceInternal::connectionLost() || IceInternal::getSocketErrno() == 0)
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
                    throw ProtocolException(
                        __FILE__,
                        __LINE__,
                        "SSL protocol error during write:\n" + _engine->sslErrors());
                }
            }
        }

        buf.i += ret;

        if (packetSize > buf.b.end() - buf.i)
        {
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
OpenSSL::TransceiverI::read(IceInternal::Buffer& buf)
{
    if (!_connected)
    {
        return _delegate->read(buf);
    }

    // We assume that OpenSSL doesn't read more SSL records than necessary to fill the requested data and that the
    // sender sends Ice messages in individual SSL records.
    if (buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }

    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);

    // It's impossible for packetSize to be more than an Int.
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    while (buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
        int ret = SSL_read(_ssl, reinterpret_cast<void*>(&*buf.i), packetSize);
        if (ret <= 0)
        {
            switch (SSL_get_error(_ssl, ret))
            {
                case SSL_ERROR_NONE:
                {
                    assert(false);
                    break;
                }
                case SSL_ERROR_ZERO_RETURN:
                {
                    throw ConnectionLostException(__FILE__, __LINE__);
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
                    if (IceInternal::interrupted())
                    {
                        continue;
                    }

                    if (IceInternal::noBuffers() && packetSize > 1024)
                    {
                        packetSize /= 2;
                        continue;
                    }

                    if (IceInternal::wouldBlock())
                    {
                        assert(SSL_want_read(_ssl));
                        return IceInternal::SocketOperationRead;
                    }

                    if (IceInternal::connectionLost() || IceInternal::getSocketErrno() == 0)
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
#if defined(SSL_R_UNEXPECTED_EOF_WHILE_READING)
                    if (SSL_R_UNEXPECTED_EOF_WHILE_READING == ERR_GET_REASON(ERR_peek_error()))
                    {
                        ERR_clear_error();
                        throw ConnectionLostException(__FILE__, __LINE__);
                    }
                    else
                    {
#endif
                        throw ProtocolException(
                            __FILE__,
                            __LINE__,
                            "SSL protocol error during read:\n" + _engine->sslErrors());
#if defined(SSL_R_UNEXPECTED_EOF_WHILE_READING)
                    }
#endif
                }
            }
        }

        buf.i += ret;

        if (packetSize > buf.b.end() - buf.i)
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
OpenSSL::TransceiverI::getInfo(bool incoming, string adapterName, string connectionId) const
{
    assert(incoming == _incoming);
    // adapterName is the name of the object adapter currently associated with this connection, while _adapterName
    // represents the name of the object adapter that created this connection (incoming only).

    X509* peerCertificate = nullptr;
    if (_peerCertificate)
    {
        peerCertificate = X509_dup(_peerCertificate);
    }

    return make_shared<ConnectionInfo>(
        _delegate->getInfo(incoming, std::move(adapterName), std::move(connectionId)),
        peerCertificate);
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
OpenSSL::TransceiverI::verifyCallback(int ok, X509_STORE_CTX* ctx)
{
    assert(_remoteCertificateVerificationCallback);
    try
    {
        if (!_peerCertificate)
        {
            // Retrieve the peer certificate if not already set by a previous call to the verification callback.
            STACK_OF(X509)* chain = X509_STORE_CTX_get1_chain(ctx);
            if (chain && sk_X509_num(chain) > 0)
            {
                _peerCertificate = X509_dup(sk_X509_value(chain, 0));
                sk_X509_pop_free(chain, X509_free);
            }
        }
        bool verified = _remoteCertificateVerificationCallback(
            ok,
            ctx,
            dynamic_pointer_cast<ConnectionInfo>(getInfo(_incoming, _adapterName, "")));
        if (!verified)
        {
            long result = SSL_get_verify_result(_ssl);
            if (result == X509_V_OK)
            {
                throw SecurityException(
                    __FILE__,
                    __LINE__,
                    "IceSSL: certificate verification failed. The certificate was rejected by the certificate "
                    "validation callback.");
            }
            else
            {
                ostringstream os;
                os << "IceSSL: certificate verification failed:\n" << X509_verify_cert_error_string(result);
                const string msg = os.str();
                if (_engine->securityTraceLevel() >= 1)
                {
                    _instance->logger()->trace(_instance->traceCategory(), msg);
                }
                throw SecurityException(__FILE__, __LINE__, msg);
            }
        }
    }
    catch (...)
    {
        _verificationException = current_exception();
    }
    // Allow the SSL handshake to continue, the transceiver initialization will throw an exception if the verification
    // fails.
    return 1;
}

OpenSSL::TransceiverI::TransceiverI(
    InstancePtr instance,
    IceInternal::TransceiverPtr delegate,
    string adapterName,
    const ServerAuthenticationOptions& serverAuthenticationOptions)
    : _instance(std::move(instance)),
      _engine(dynamic_pointer_cast<OpenSSL::SSLEngine>(_instance->engine())),
      _host(""),
      _adapterName(std::move(adapterName)),
      _incoming(true),
      _delegate(std::move(delegate)),
      _connected(false),
      _peerCertificate(nullptr),
      _ssl(nullptr),
      _sslCtx(nullptr),
      _localSSLContextSelectionCallback(
          serverAuthenticationOptions.serverSSLContextSelectionCallback
              ? serverAuthenticationOptions.serverSSLContextSelectionCallback
              : defaultSSLContextSelectionCallback),
      _remoteCertificateVerificationCallback(
          serverAuthenticationOptions.clientCertificateValidationCallback
              ? serverAuthenticationOptions.clientCertificateValidationCallback
              : defaultVerificationCallback),
      _sslNewSessionCallback(serverAuthenticationOptions.sslNewSessionCallback)
{
}

OpenSSL::TransceiverI::TransceiverI(
    InstancePtr instance,
    IceInternal::TransceiverPtr delegate,
    string host,
    const ClientAuthenticationOptions& clientAuthenticationOptions)
    : _instance(std::move(instance)),
      _engine(dynamic_pointer_cast<OpenSSL::SSLEngine>(_instance->engine())),
      _host(std::move(host)),
      _adapterName(""),
      _incoming(false),
      _delegate(std::move(delegate)),
      _connected(false),
      _peerCertificate(nullptr),
      _ssl(nullptr),
      _sslCtx(nullptr),
      _localSSLContextSelectionCallback(
          clientAuthenticationOptions.clientSSLContextSelectionCallback
              ? clientAuthenticationOptions.clientSSLContextSelectionCallback
              : defaultSSLContextSelectionCallback),
      _remoteCertificateVerificationCallback(
          clientAuthenticationOptions.serverCertificateValidationCallback
              ? clientAuthenticationOptions.serverCertificateValidationCallback
              : defaultVerificationCallback),
      _sslNewSessionCallback(clientAuthenticationOptions.sslNewSessionCallback)
{
}

OpenSSL::TransceiverI::~TransceiverI() = default;
