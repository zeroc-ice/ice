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
#include <Ice/SslException.h>
#include <Ice/SslFactory.h>
#include <Ice/SslConnection.h>
#include <Ice/SslConnectionOpenSSL.h>
#include <Ice/SystemOpenSSL.h>
#include <Ice/CertificateVerifierOpenSSL.h>
#include <Ice/OpenSSLUtils.h>

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

using IceSSL::Factory;
using IceSSL::SystemInternalPtr;

////////////////////////////////
////////// Connection //////////
////////////////////////////////

//
// Static Member Initialization
//
IceSSL::OpenSSL::SslConnectionMap IceSSL::OpenSSL::Connection::_connectionMap;
::IceUtil::Mutex IceSSL::OpenSSL::Connection::_connectionRepositoryMutex;

//
// Public Methods
//

void ::IceInternal::incRef(::IceSSL::OpenSSL::Connection* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::OpenSSL::Connection* p) { p->__decRef(); }

// Note: I would use a using directive of the form:
//       using IceSSL::CertificateVerifierPtr;
//       but unfortunately, it appears that this is not properly picked up.
//

IceSSL::OpenSSL::Connection::Connection(
            const IceInternal::TraceLevelsPtr& traceLevels,
            const Ice::LoggerPtr& logger,
            const IceSSL::CertificateVerifierPtr& certificateVerifier,
            SSL* sslConnection,
            const IceSSL::SystemInternalPtr& system) :
                                      IceSSL::Connection(traceLevels, logger, certificateVerifier),
                                      _sslConnection(sslConnection)
{
    assert(_sslConnection != 0);
    assert(system != 0);

    Factory::addSystemHandle(_sslConnection, system);

    _lastError = SSL_ERROR_NONE;

    _initWantRead = 0;
    _initWantWrite = 0;

    // None configured, default to indicated timeout
    _handshakeReadTimeout = 0;

    // Set up the SSL to be able to refer back to our connection object.
    addConnection(_sslConnection, this);
}

IceSSL::OpenSSL::Connection::~Connection()
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
IceSSL::OpenSSL::Connection::shutdown()
{
    if (_sslConnection != 0)
    {
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
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

        if ((_traceLevels->security >= IceSSL::SECURITY_PROTOCOL) && (shutdown <= 0))
        {
            ostringstream s;
            s << "SSL shutdown failure encountered: code[" << shutdown << "] retries[";
            s << retries << "]\n" << fdToString(SSL_get_fd(_sslConnection));
            _logger->trace(_traceLevels->securityCat, s.str());
        }
    }
}

void
IceSSL::OpenSSL::Connection::setHandshakeReadTimeout(int timeout)
{
    _handshakeReadTimeout = timeout;
}

IceSSL::OpenSSL::ConnectionPtr
IceSSL::OpenSSL::Connection::getConnection(SSL* sslPtr)
{
    IceUtil::Mutex::Lock sync(_connectionRepositoryMutex);

    assert(sslPtr);

    Connection* connection = _connectionMap[sslPtr];

    assert(connection);

    return ConnectionPtr(connection);
}

//
// Note: Do not throw exceptions from verifyCertificate - it would rip through the OpenSSL system,
//       interfering with the usual handling and alert system of the handshake.  Exceptions should
//       be caught here (if they can be generated), logged and then a fail return code (0) should 
//       returned.
//
int
IceSSL::OpenSSL::Connection::verifyCertificate(int preVerifyOkay, X509_STORE_CTX* x509StoreContext)
{
    // Should NEVER be able to happen.
    assert(_certificateVerifier.get() != 0);

    // Get the verifier, make sure it is for OpenSSL connections
    IceSSL::OpenSSL::CertificateVerifier* verifier;
    verifier = dynamic_cast<IceSSL::OpenSSL::CertificateVerifier*>(_certificateVerifier.get());

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

        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
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
IceSSL::OpenSSL::Connection::connect()
{
    assert(_sslConnection != 0);

    int result = SSL_connect(_sslConnection);

    setLastError(result);

    return result;
}

int
IceSSL::OpenSSL::Connection::accept()
{
    assert(_sslConnection != 0);

    int result = SSL_accept(_sslConnection);

    setLastError(result);

    return result;
}

// NOTE: Currently not used, maybe later.
int
IceSSL::OpenSSL::Connection::renegotiate()
{
    assert(_sslConnection != 0);
    return SSL_renegotiate(_sslConnection);
}

int
IceSSL::OpenSSL::Connection::initialize(int timeout)
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
IceSSL::OpenSSL::Connection::pending()
{
    assert(_sslConnection != 0);
    return SSL_pending(_sslConnection);
}

int
IceSSL::OpenSSL::Connection::getLastError() const
{
    assert(_sslConnection != 0);
    return SSL_get_error(_sslConnection, _lastError);
}

int
IceSSL::OpenSSL::Connection::sslRead(char* buffer, int bufferSize)
{
    assert(_sslConnection != 0);

    int bytesRead = SSL_read(_sslConnection, buffer, bufferSize);

    setLastError(bytesRead);

    return bytesRead;
}

int
IceSSL::OpenSSL::Connection::sslWrite(char* buffer, int bufferSize)
{
    assert(_sslConnection != 0);

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
/*
void
IceSSL::OpenSSL::Connection::protocolWrite()
{
    static char buffer[10];

    memset(buffer, 0, sizeof(buffer));

    // Note: We should be calling the write(char*,int) method here,
    //       not the write(Buffer&,int) method.  If things start acting
    //       strangely, check this!
    sslWrite(buffer,0);
}
*/

int
IceSSL::OpenSSL::Connection::readInBuffer(Buffer& buf)
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

        if (_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
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
IceSSL::OpenSSL::Connection::select(int timeout, bool write)
{
    int ret;

    assert(_sslConnection != 0);
    SOCKET fd = SSL_get_fd(_sslConnection);

    fd_set rwFdSet;
    struct timeval tv;

    if (timeout >= 0)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
    }

    do
    {
        FD_ZERO(&rwFdSet);
        FD_SET(fd, &rwFdSet);

        if (timeout >= 0)
        {
            if (write)
            {
                ret = ::select(fd + 1, 0, &rwFdSet, 0, &tv);
            }
            else
            {
                ret = ::select(fd + 1, &rwFdSet, 0, 0, &tv);
            }
        }
        else
        {
            if (write)
            {
                ret = ::select(fd + 1, 0, &rwFdSet, 0, 0);
            }
            else
            {
                ret = ::select(fd + 1, &rwFdSet, 0, 0, 0);
            }
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

    return FD_ISSET(fd, &rwFdSet);
}

int
IceSSL::OpenSSL::Connection::readSelect(int timeout)
{
    return select(timeout, false);
}

int
IceSSL::OpenSSL::Connection::writeSelect(int timeout)
{
    return select(timeout, true);
}

int
IceSSL::OpenSSL::Connection::readSSL(Buffer& buf, int timeout)
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
            if (_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
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
                // TODO: This can most likely be removed.

                // If we get this error here, it HAS to be because the protocol wants
                // to do something handshake related.  As such, We're going to call
                // write with an empty buffer.  I've seen this done in the demo
                // programs, so this should be valid.  No actual application data
                // will be sent, just protocol packets.
                // protocolWrite();
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

void
IceSSL::OpenSSL::Connection::addConnection(SSL* sslPtr, Connection* connection)
{
    assert(sslPtr);
    assert(connection);
    IceUtil::Mutex::Lock sync(_connectionRepositoryMutex);
    _connectionMap[sslPtr] = connection;
}

void
IceSSL::OpenSSL::Connection::removeConnection(SSL* sslPtr)
{
    assert(sslPtr);
    IceUtil::Mutex::Lock sync(_connectionRepositoryMutex);
    _connectionMap.erase(sslPtr);
}

void
IceSSL::OpenSSL::Connection::showCertificateChain(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

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
IceSSL::OpenSSL::Connection::showPeerCertificate(BIO* bio, const char* connType)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

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
IceSSL::OpenSSL::Connection::showSharedCiphers(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

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
IceSSL::OpenSSL::Connection::showSessionInfo(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

    if (_sslConnection->hit)
    {
        BIO_printf(bio, "Reused session-id\n");
    }

    PEM_write_bio_SSL_SESSION(bio, SSL_get_session(_sslConnection));
}

void
IceSSL::OpenSSL::Connection::showSelectedCipherInfo(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

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
IceSSL::OpenSSL::Connection::showHandshakeStats(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

    BIO_printf(bio, "---\nSSL handshake has read %ld bytes and written %ld bytes\n",
            BIO_number_read(SSL_get_rbio(_sslConnection)),
            BIO_number_written(SSL_get_wbio(_sslConnection)));
}

void
IceSSL::OpenSSL::Connection::showClientCAList(BIO* bio, const char* connType)
{
    assert(_sslConnection != 0);
    assert(bio != 0);
    assert(connType != 0);

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
