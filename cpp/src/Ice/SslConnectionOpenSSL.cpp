// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Note: This pragma is used to disable spurious warning messages having
//       to do with the length of debug symbols exceeding 255 characters.
//       This is due to STL template identifiers expansion.
//       The MSDN Library recommends that you put this pragma directive
//       in place to avoid the warnings.
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <openssl/err.h>
#include <string>
#include <sstream>
#include <Ice/Network.h>
#include <Ice/OpenSSL.h>
#include <Ice/SecurityException.h>
#include <Ice/SslFactory.h>
#include <Ice/SslConnection.h>
#include <Ice/SslConnectionOpenSSL.h>
#include <Ice/SslSystemOpenSSL.h>
#include <Ice/SslCertificateVerifierOpenSSL.h>
#include <Ice/SslOpenSSLUtils.h>

#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

using namespace std;
using namespace IceInternal;

using Ice::SocketException;
using Ice::TimeoutException;
using Ice::ConnectionLostException;
using Ice::LoggerPtr;
using Ice::Int;

using std::endl;

using IceSecurity::Ssl::Factory;
using IceSecurity::Ssl::SystemPtr;

////////////////////////////////////////////////
////////// DefaultCertificateVerifier //////////
////////////////////////////////////////////////

IceSecurity::Ssl::OpenSSL::DefaultCertificateVerifier::DefaultCertificateVerifier()
{
}

void
IceSecurity::Ssl::OpenSSL::DefaultCertificateVerifier::setTraceLevels(const TraceLevelsPtr& traceLevels)
{
    _traceLevels = traceLevels;
}

void
IceSecurity::Ssl::OpenSSL::DefaultCertificateVerifier::setLogger(const LoggerPtr& logger)
{
    _logger = logger;
}

int
IceSecurity::Ssl::OpenSSL::DefaultCertificateVerifier::verify(int preVerifyOkay,
                                                              X509_STORE_CTX* x509StoreContext,
                                                              SSL* sslConnection)
{
    //
    // Default verification steps.
    //

    int verifyError = X509_STORE_CTX_get_error(x509StoreContext);
    int errorDepth = X509_STORE_CTX_get_error_depth(x509StoreContext);
    int verifyDepth = SSL_get_verify_depth(sslConnection);

    // Verify Depth was set
    if (verifyError != X509_V_OK)
    {
        // If we have no errors so far, and the certificate chain is too long
        if ((verifyDepth != -1) && (verifyDepth < errorDepth))
        {
            verifyError = X509_V_ERR_CERT_CHAIN_TOO_LONG;
            X509_STORE_CTX_set_error(x509StoreContext, verifyError);
        }

        // If we have ANY errors, we bail out.
        preVerifyOkay = 0;
    }

    // Only if ICE_PROTOCOL level logging is on do we worry about this.
    if (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL)
    {
        char buf[256];

        X509* err_cert = X509_STORE_CTX_get_current_cert(x509StoreContext);

        X509_NAME_oneline(X509_get_subject_name(err_cert), buf, sizeof(buf));

        ostringstream outStringStream;

        outStringStream << "depth = " << dec << errorDepth << ":" << buf << std::endl;

        if (!preVerifyOkay)
        {
            outStringStream << "verify error: num = " << verifyError << " : " 
			    << X509_verify_cert_error_string(verifyError) << endl;

        }

        switch (verifyError)
        {
            case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            {
                X509_NAME_oneline(X509_get_issuer_name(err_cert), buf, sizeof(buf));
                outStringStream << "issuer = " << buf << endl;
                break;
            }

            case X509_V_ERR_CERT_NOT_YET_VALID:
            case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            {
                outStringStream << "notBefore = " << getASN1time(X509_get_notBefore(err_cert)) << endl;
                break;
            }

            case X509_V_ERR_CERT_HAS_EXPIRED:
            case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            {
                outStringStream << "notAfter = " << getASN1time(X509_get_notAfter(err_cert)) << endl;
                break;
            }
        }

        outStringStream << "verify return = " << preVerifyOkay << endl;

        _logger->trace(_traceLevels->securityCat, outStringStream.str());
    }

    return preVerifyOkay;
}

////////////////////////////////
////////// Connection //////////
////////////////////////////////

//
// Static Member Initialization
//
IceSecurity::Ssl::OpenSSL::SslConnectionMap IceSecurity::Ssl::OpenSSL::Connection::_connectionMap;
::IceUtil::Mutex IceSecurity::Ssl::OpenSSL::Connection::_connectionRepositoryMutex;

//
// Public Methods
//

void ::IceInternal::incRef(::IceSecurity::Ssl::OpenSSL::Connection* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::Ssl::OpenSSL::Connection* p) { p->__decRef(); }

// Note: I would use a using directive of the form:
//       using IceSecurity::Ssl::CertificateVerifierPtr;
//       but unfortunately, it appears that this is not properly picked up.
//

IceSecurity::Ssl::OpenSSL::Connection::Connection(
            const IceSecurity::Ssl::CertificateVerifierPtr& certificateVerifier,
            SSL* sslConnection,
            const SystemPtr& system) :
                                      IceSecurity::Ssl::Connection(certificateVerifier),
                                      _sslConnection(sslConnection),
                                      _system(system)
{
    assert(sslConnection);

    Factory::addSystemHandle(sslConnection, system);

    _lastError = SSL_ERROR_NONE;

    _initWantRead = 0;
    _initWantWrite = 0;

    // None configured, default to indicated timeout
    _handshakeReadTimeout = 0;

    // Set up the SSL to be able to refer back to our connection object.
    addConnection(_sslConnection, this);
}

IceSecurity::Ssl::OpenSSL::Connection::~Connection()
{
    if (_sslConnection != 0)
    {
        removeConnection(_sslConnection);
        Factory::removeSystemHandle(_sslConnection);
        SSL_free(_sslConnection);
        _sslConnection = 0;
    }
}

void
IceSecurity::Ssl::OpenSSL::Connection::shutdown()
{
    if (_sslConnection != 0)
    {
        if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat, "WRN " +
                           string("shutting down SSL connection\n") +
                           fdToString(SSL_get_fd(_sslConnection)));
        }

        int shutdown = 0;
        int retries = 100;

        do
        {
            shutdown = SSL_shutdown(_sslConnection);
            retries--;
        }
        while ((shutdown == 0) && (retries > 0));

        if ((_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL) && (shutdown <= 0))
        {
            ostringstream s;
            s << "SSL shutdown failure encountered: code[" << shutdown << "] retries[";
            s << retries << "]\n" << fdToString(SSL_get_fd(_sslConnection));
            _logger->trace(_traceLevels->securityCat, s.str());
        }
    }
}

void
IceSecurity::Ssl::OpenSSL::Connection::setTrace(const TraceLevelsPtr& traceLevels)
{
    _traceLevels = traceLevels;
}

void
IceSecurity::Ssl::OpenSSL::Connection::setLogger(const LoggerPtr& traceLevels)
{
    _logger = traceLevels;
}

void
IceSecurity::Ssl::OpenSSL::Connection::setHandshakeReadTimeout(int timeout)
{
    _handshakeReadTimeout = timeout;
}

IceSecurity::Ssl::OpenSSL::ConnectionPtr
IceSecurity::Ssl::OpenSSL::Connection::getConnection(SSL* sslPtr)
{
    IceUtil::Mutex::Lock sync(_connectionRepositoryMutex);

    assert(sslPtr);

    Connection* connection = _connectionMap[sslPtr];

    assert(connection);

    return ConnectionPtr(connection);
}

int
IceSecurity::Ssl::OpenSSL::Connection::verifyCertificate(int preVerifyOkay, X509_STORE_CTX* x509StoreContext)
{
    // Should NEVER be able to happen.
    assert(_certificateVerifier.get() != 0);

    // Get the verifier, make sure it is for OpenSSL connections
    IceSecurity::Ssl::OpenSSL::CertificateVerifier* verifier;
    verifier = dynamic_cast<IceSecurity::Ssl::OpenSSL::CertificateVerifier*>(_certificateVerifier.get());

    // Check to make sure we have a proper verifier for the operation.
    if (verifier)
    {
        // Use the verifier to verify the certificate
        preVerifyOkay = verifier->verify(preVerifyOkay, x509StoreContext, _sslConnection);
    }
    else
    {
        // Note: This code should NEVER be able to be reached, as we check each
        //       CertificateVerifier as it is added to the System.

        if (_traceLevels->security >= IceSecurity::SECURITY_WARNINGS)
        {
            string errorString;

            if (_certificateVerifier.get())
            {
                errorString = "WRN Improper CertificateVerifier type.";
            }
            else
            {
                // NOTE: This should NEVER be able to happen, but just in case.
                errorString = "WRN CertificateVerifier not set.";
            }

            _logger->trace(_traceLevels->securityCat, errorString);
        }
    }

    return preVerifyOkay;
}

//
// Protected Methods
//

int
IceSecurity::Ssl::OpenSSL::Connection::connect()
{
    int result = SSL_connect(_sslConnection);

    setLastError(result);

    return result;
}

int
IceSecurity::Ssl::OpenSSL::Connection::accept()
{
    int result = SSL_accept(_sslConnection);

    setLastError(result);

    return result;
}

int
IceSecurity::Ssl::OpenSSL::Connection::renegotiate()
{
    return SSL_renegotiate(_sslConnection);
}

int
IceSecurity::Ssl::OpenSSL::Connection::initialize(int timeout)
{
    int retCode = 0;

    while (true)
    {
        // One lucky thread will get the honor of carrying out the hanshake,
        // if there is one to perform.  The HandshakeSentinel effectively
        // establishes a first-come, first-serve policy.  One thread will own
        // the handshake, and the others will either return rejected to the
        // caller (who will figure out what to do with them) OR wait until
        // our lead thread is done.  Then, the shuffle begins again.
        // Eventually, all threads will filter through.

        HandshakeSentinel handshakeSentinel(_handshakeFlag);

        if (!handshakeSentinel.ownHandshake())
        {
            if (timeout >= 0)
            {
                // We should return immediately here - do not block,
                // leave it to the caller to figure this out.
                retCode = -1;
                break;
            }
            else
            {
                // We will wait here - blocking IO is being used.
                IceUtil::Mutex::Lock sync(_handshakeWaitMutex);
            }
        }
        else
        {
            // Perform our init(), then leave.
            IceUtil::Mutex::Lock sync(_handshakeWaitMutex);
            retCode = init(timeout);
            break;
        }
    }

    return retCode;
}

int
IceSecurity::Ssl::OpenSSL::Connection::sslRead(char* buffer, int bufferSize)
{
    int bytesRead = SSL_read(_sslConnection, buffer, bufferSize);

    setLastError(bytesRead);

    return bytesRead;
}

int
IceSecurity::Ssl::OpenSSL::Connection::sslWrite(char* buffer, int bufferSize)
{
    int bytesWritten = SSL_write(_sslConnection, buffer, bufferSize);

    setLastError(bytesWritten);

    return bytesWritten;
}

// protocolWrite()
//
// The entire purpose of this strange little routine is to provide OpenSSL with a
// SSL_write() when they request one (this is for handshaking purposes).  It writes
// nothing at all.  Its entire purpose is jut to call the SSL_write() through one.
// of our defined methods.  The SSL_write() will end up only writing protocol handshake
// packets, not application packets.  This looks wierd, but it is essentially what
// the demo programs are doing, so I feel okay copying them.  The only reason that I
// have defined the buffer[] array is so that I have a valid buffer pointer.
void
IceSecurity::Ssl::OpenSSL::Connection::protocolWrite()
{
    static char buffer[10];

    memset(buffer, 0, sizeof(buffer));

    // Note: We should be calling the write(char*,int) method here,
    //       not the write(Buffer&,int) method.  If things start acting
    //       strangely, check this!
    sslWrite(buffer,0);
}

int
IceSecurity::Ssl::OpenSSL::Connection::readInBuffer(Buffer& buf)
{
    IceUtil::Mutex::Lock sync(_inBufferMutex);

    int bytesRead = 0;

    if (!_inBuffer.b.empty())
    {
        // Just how big is the destination?
        int bufferSize = buf.b.end() - buf.i;

        // And how much do we have in our _inBuffer to copy?
        int inBufferSize = _inBuffer.i - _inBuffer.b.begin();

        // Select how many bytes we can handle.
        bytesRead = min(bufferSize, inBufferSize);

        // Iterators that indicate how much of the _inBuffer we're going to copy
        Buffer::Container::iterator inBufferBegin = _inBuffer.b.begin();
        Buffer::Container::iterator inBufferEndAt = (_inBuffer.b.begin() + bytesRead);

        // Copy over the bytes from the _inBuffer to our destination buffer
        buf.i = copy(inBufferBegin, inBufferEndAt, buf.i);

        // Erase the data that we've copied out of the _inBuffer.
        _inBuffer.b.erase(inBufferBegin, inBufferEndAt);

        if (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL)
        {
            ostringstream protocolMsg;
            protocolMsg << "Copied " << dec << bytesRead << " bytes from SSL buffer\n";
            protocolMsg << fdToString(SSL_get_fd(_sslConnection));

            _logger->trace(_traceLevels->securityCat, protocolMsg.str());
        }
    }

    return bytesRead;
}

int
IceSecurity::Ssl::OpenSSL::Connection::readSelect(int timeout)
{
    int ret;
    SOCKET fd = SSL_get_fd(_sslConnection);
    fd_set rFdSet;

    struct timeval tv;

    if (timeout >= 0)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
    }

    do
    {
        FD_ZERO(&rFdSet);
        FD_SET(fd, &rFdSet);

        if (timeout >= 0)
        {
            ret = ::select(fd + 1, &rFdSet, 0, 0, &tv);
        }
        else
        {
            ret = ::select(fd + 1, &rFdSet, 0, 0, 0);
        }
    }
    while (ret == SOCKET_ERROR && interrupted());

    if (ret == SOCKET_ERROR)
    {
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }

    if (ret == 0)
    {
        throw TimeoutException(__FILE__, __LINE__);
    }

    return FD_ISSET(fd, &rFdSet);
}

int
IceSecurity::Ssl::OpenSSL::Connection::writeSelect(int timeout)
{
    int ret;
    SOCKET fd = SSL_get_fd(_sslConnection);
    fd_set wFdSet;

    struct timeval tv;

    if (timeout >= 0)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
    }

    do
    {
        FD_ZERO(&wFdSet);
        FD_SET(fd, &wFdSet);

        if (timeout >= 0)
        {
            ret = ::select(fd + 1, 0, &wFdSet, 0, &tv);
        }
        else
        {
            ret = ::select(fd + 1, 0, &wFdSet, 0, 0);
        }
    }
    while (ret == SOCKET_ERROR && interrupted());

    if (ret == SOCKET_ERROR)
    {
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }

    if (ret == 0)
    {
        throw TimeoutException(__FILE__, __LINE__);
    }

    return FD_ISSET(fd, &wFdSet);
}

int
IceSecurity::Ssl::OpenSSL::Connection::readSSL(Buffer& buf, int timeout)
{
    int packetSize = buf.b.end() - buf.i;
    int totalBytesRead = 0;
    int bytesPending;
    int bytesRead;

    int initReturn = 0;

    // We keep reading until we're done.
    while (buf.i != buf.b.end())
    {
        // Ensure we're initialized.
        initReturn = initialize(timeout);

        if (initReturn == -1)
        {
            // Handshake underway, timeout immediately, easy way to deal with this.
            // _logger->trace(_traceLevels->securityCat, "Throwing TimeoutException, Line 566");
            throw TimeoutException(__FILE__, __LINE__);
        }

        if (initReturn == 0)
        {
            // Retry the initialize call
            continue;
        }

        // initReturn must be > 0, so we're okay to try a write

        bytesPending = pending();

        if (!bytesPending && readSelect(_readTimeout))
        {
            bytesPending = 1;
        }

        _readTimeout = timeout;

        if (!bytesPending)
        {
            if (_traceLevels->security >= IceSecurity::SECURITY_PROTOCOL)
            {
                _logger->trace(_traceLevels->securityCat, "No pending application-level bytes.");
            }

            // We're done here.
            break;
        }

        bytesRead = sslRead((char *)buf.i, packetSize);

        switch (getLastError())
        {
            case SSL_ERROR_NONE:
            {
                if (bytesRead > 0)
                {
                    if (_traceLevels->network >= 3)
                    {
                        ostringstream s;
                        s << "received " << bytesRead << " of " << packetSize;
                        s << " bytes via ssl\n" << fdToString(SSL_get_fd(_sslConnection));
                        _logger->trace(_traceLevels->networkCat, s.str());
                    }

                    totalBytesRead += bytesRead;

                    buf.i += bytesRead;

                    if (packetSize > buf.b.end() - buf.i)
                    {
                        packetSize = buf.b.end() - buf.i;
                    }
                }
                continue;
            }

            case SSL_ERROR_WANT_WRITE:
            {
                // If we get this error here, it HAS to be because the protocol wants
                // to do something handshake related.  As such, We're going to call
                // write with an empty buffer.  I've seen this done in the demo
                // programs, so this should be valid.  No actual application data
                // will be sent, just protocol packets.
                protocolWrite();
                continue;
            }

            case SSL_ERROR_WANT_READ:
            {
                // Repeat with the same arguments! (as in the OpenSSL documentation)
                // Whatever happened, the last read didn't actually read anything for
                // us.  This is effectively a retry.
                continue;
            }

            case SSL_ERROR_WANT_X509_LOOKUP:
            {
                // Perform another read.  The read should take care of this.
                continue;
            }

            case SSL_ERROR_SYSCALL:
            {
                if(bytesRead == -1)
                {
                    // IO Error in underlying BIO

                    if (interrupted())
                    {
                        break;
                    }

                    if (wouldBlock())
                    {
                        break;
                    }

                    if (connectionLost())
                    {
                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = getSocketErrno();
                        throw ex;
                    }
                    else
                    {
                        SocketException ex(__FILE__, __LINE__);
                        ex.error = getSocketErrno();
                        throw ex;
                    }
                }
                else // (bytesRead == 0)
                {
                    ProtocolException protocolEx(__FILE__, __LINE__);

                    // Protocol Error: Unexpected EOF
                    protocolEx._message = "Encountered an EOF that violates the SSL Protocol.\n";
                    protocolEx._message += sslGetErrors();

                    throw protocolEx;
                }
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx._message = "Encountered a violation of the SSL Protocol.\n";
                protocolEx._message += sslGetErrors();

                throw protocolEx;
            }

            case SSL_ERROR_ZERO_RETURN:
            {
                // Indicates that that the SSL Connection has been closed.
                // But does not necessarily indicate that the underlying transport
                // has been closed (in the case of Ice, it definitely hasn't yet).

                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }
    }

    return totalBytesRead;
}

string
IceSecurity::Ssl::OpenSSL::Connection::sslGetErrors()
{
    string errorMessage;
    char buf[200];
    char bigBuffer[1024];
    const char* file = 0;
    const char* data = 0;
    int line = 0;
    int flags = 0;
    unsigned errorCode = 0;
    int errorNum = 1;

    unsigned long es = CRYPTO_thread_id();

    while ((errorCode = ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
    {
        sprintf(bigBuffer,"%6d - Thread ID: %lu\n", errorNum, es);
        errorMessage += bigBuffer;

        sprintf(bigBuffer,"%6d - Error:     %u\n", errorNum, errorCode);
        errorMessage += bigBuffer;

        // Request an error from the OpenSSL library
        ERR_error_string_n(errorCode, buf, sizeof(buf));
        sprintf(bigBuffer,"%6d - Message:   %s\n", errorNum, buf);
        errorMessage += bigBuffer;

        sprintf(bigBuffer,"%6d - Location:  %s, %d\n", errorNum, file, line);
        errorMessage += bigBuffer;

        if (flags & ERR_TXT_STRING)
        {
            sprintf(bigBuffer,"%6d - Data:      %s\n", errorNum, data);
            errorMessage += bigBuffer;
        }

        errorNum++;
    }

    return errorMessage;
}

void
IceSecurity::Ssl::OpenSSL::Connection::addConnection(SSL* sslPtr, Connection* connection)
{
    assert(sslPtr);
    assert(connection);
    IceUtil::Mutex::Lock sync(_connectionRepositoryMutex);
    _connectionMap[sslPtr] = connection;
}

void
IceSecurity::Ssl::OpenSSL::Connection::removeConnection(SSL* sslPtr)
{
    assert(sslPtr);
    IceUtil::Mutex::Lock sync(_connectionRepositoryMutex);
    _connectionMap.erase(sslPtr);
}

void
IceSecurity::Ssl::OpenSSL::Connection::showCertificateChain(BIO* bio)
{
    STACK_OF(X509)* sk;

    // Big nasty buffer
    char buffer[4096];

    if ((sk = SSL_get_peer_cert_chain(_sslConnection)) != 0)
    {
        BIO_printf(bio,"---\nCertificate chain\n");

        for (int i = 0; i < sk_X509_num(sk); i++)
        {
            X509_NAME_oneline(X509_get_subject_name(sk_X509_value(sk,i)), buffer, sizeof(buffer));
            BIO_printf(bio, "%2d s:%s\n", i, buffer);

            X509_NAME_oneline(X509_get_issuer_name(sk_X509_value(sk,i)), buffer, sizeof(buffer));
            BIO_printf(bio, "   i:%s\n", buffer);

            PEM_write_bio_X509(bio, sk_X509_value(sk, i));
        }
    }
    else
    {
        BIO_printf(bio, "---\nNo peer certificate chain available.\n");
    }
}

void
IceSecurity::Ssl::OpenSSL::Connection::showPeerCertificate(BIO* bio, const char* connType)
{
    X509* peerCert = 0;
    char buffer[4096];

    if ((peerCert = SSL_get_peer_certificate(_sslConnection)) != 0)
    {
        BIO_printf(bio, "%s Certificate\n", connType);
        PEM_write_bio_X509(bio, peerCert);

        X509_NAME_oneline(X509_get_subject_name(peerCert), buffer, sizeof(buffer));
        BIO_printf(bio, "subject=%s\n", buffer);

        X509_NAME_oneline(X509_get_issuer_name(peerCert), buffer, sizeof(buffer));
        BIO_printf(bio, "issuer=%s\n", buffer);

        EVP_PKEY *pktmp;
        pktmp = X509_get_pubkey(peerCert);
        BIO_printf(bio,"%s public key is %d bit\n", connType, EVP_PKEY_bits(pktmp));
        EVP_PKEY_free(pktmp);

        X509_free(peerCert);
    }
    else
    {
        BIO_printf(bio, "No %s certificate available.\n", connType);
    }
}

void
IceSecurity::Ssl::OpenSSL::Connection::showSharedCiphers(BIO* bio)
{
    char buffer[4096];
    char* strpointer = 0;

    if ((strpointer = SSL_get_shared_ciphers(_sslConnection, buffer, sizeof(buffer))) != 0)
    {
        // This works only for SSL 2.  In later protocol
        // versions, the client does not know what other
        // ciphers (in addition to the one to be used
        // in the current connection) the server supports.

        BIO_printf(bio, "---\nShared Ciphers:\n");

        int j = 0;
        int i = 0;

        while (*strpointer)
        {
            if (*strpointer == ':')
            {
                BIO_write(bio, "                ", (15-j%25));
                i++;
                j=0;
                BIO_write(bio, ((i%3)?" ":"\n"), 1);
            }
            else
            {
                BIO_write(bio, strpointer, 1);
                j++;
            }

            strpointer++;
        }

        BIO_write(bio,"\n",1);
    }
}

void
IceSecurity::Ssl::OpenSSL::Connection::showSessionInfo(BIO* bio)
{
    if (_sslConnection->hit)
    {
        BIO_printf(bio, "Reused session-id\n");
    }

    PEM_write_bio_SSL_SESSION(bio, SSL_get_session(_sslConnection));
}

void
IceSecurity::Ssl::OpenSSL::Connection::showSelectedCipherInfo(BIO* bio)
{
    const char* str;
    SSL_CIPHER* cipher;

    // Show the cipher that was finally selected.
    cipher = SSL_get_current_cipher(_sslConnection);

    str = SSL_CIPHER_get_name(cipher);
    BIO_printf(bio, "Cipher Version: %s\n", ((str != 0) ? str : "(NONE)"));

    str = SSL_CIPHER_get_version(cipher);
    BIO_printf(bio, "Cipher Name:    %s\n", ((str != 0) ? str : "(NONE)"));
}

void
IceSecurity::Ssl::OpenSSL::Connection::showHandshakeStats(BIO* bio)
{
    BIO_printf(bio, "---\nSSL handshake has read %ld bytes and written %ld bytes\n",
            BIO_number_read(SSL_get_rbio(_sslConnection)),
            BIO_number_written(SSL_get_wbio(_sslConnection)));
}

void
IceSecurity::Ssl::OpenSSL::Connection::showClientCAList(BIO* bio, const char* connType)
{
    char buffer[4096];
    STACK_OF(X509_NAME)* sk = SSL_get_client_CA_list(_sslConnection);

    if ((sk != 0) && (sk_X509_NAME_num(sk) > 0))
    {
        BIO_printf(bio,"---\nAcceptable %s certificate CA names\n", connType);

        for (int i = 0; i < sk_X509_NAME_num(sk); i++)
        {
            X509_NAME_oneline(sk_X509_NAME_value(sk, i), buffer, sizeof(buffer));
            BIO_write(bio, buffer, strlen(buffer));
            BIO_write(bio,"\n", 1);
        }
    }
    else
    {
        BIO_printf(bio,"---\nNo %s certificate CA names sent\n", connType);
    }
}
