// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <openssl/err.h>
#include <string>
#include <sstream>
#include <Ice/Network.h>
#include <JTC/JTC.h>
#include <Ice/Security.h>
#include <Ice/SecurityException.h>
#include <Ice/SslConnection.h>
#include <Ice/SslSystemOpenSSL.h>

using namespace std;
using namespace IceInternal;

using std::endl;

////////////////////////////////
////////// Connection //////////
////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::OpenSSL::Connection::Connection(SSL* sslConnection, string& systemID)
{
    assert(sslConnection);

    // Get the system we were generated from
    _system = IceSecurity::Ssl::Factory::getSystem(systemID);

    _sslConnection = sslConnection;

    _lastError = SSL_ERROR_NONE;

    initWantRead = 0;
    initWantWrite = 0;
}

IceSecurity::Ssl::OpenSSL::Connection::~Connection()
{
    ICE_METHOD_INV("OpenSSL::Connection::~Connection()");

    shutdown();

    IceSecurity::Ssl::Factory::releaseSystem(_system);

    ICE_METHOD_RET("OpenSSL::Connection::~Connection()");
}

void
IceSecurity::Ssl::OpenSSL::Connection::shutdown()
{
    ICE_METHOD_INV("OpenSSL::Connection::shutdown()");

    if (_sslConnection != 0)
    {
        ICE_WARNING(string("shutting down SSL connection\n") + fdToString(SSL_get_fd(_sslConnection)));

        SSL_free(_sslConnection);
        _sslConnection = 0;
    }

    ICE_METHOD_RET("OpenSSL::Connection::shutdown()");
}

//
// Protected Methods
//

int
IceSecurity::Ssl::OpenSSL::Connection::connect()
{
    ICE_METHOD_INV("OpenSSL::Connection::connect()");

    int result = SSL_connect(_sslConnection);

    setLastError(result);

    ICE_METHOD_RET("OpenSSL::Connection::connect()");

    return result;
}

int
IceSecurity::Ssl::OpenSSL::Connection::accept()
{
    ICE_METHOD_INV("OpenSSL::Connection::accept()");

    int result = SSL_accept(_sslConnection);

    setLastError(result);

    ICE_METHOD_RET("OpenSSL::Connection::accept()");

    return result;
}

int
IceSecurity::Ssl::OpenSSL::Connection::renegotiate()
{
    ICE_METHOD_INS("OpenSSL::Connection::renegotiate()");

    return SSL_renegotiate(_sslConnection);
}

int
IceSecurity::Ssl::OpenSSL::Connection::sslRead(char* buffer, int bufferSize)
{
    ICE_METHOD_INV("OpenSSL::Connection::sslRead()");

    int bytesRead = SSL_read(_sslConnection, buffer, bufferSize);

    setLastError(bytesRead);

    ICE_METHOD_RET("OpenSSL::Connection::sslRead()");

    return bytesRead;
}

int
IceSecurity::Ssl::OpenSSL::Connection::sslWrite(char* buffer, int bufferSize)
{
    ICE_METHOD_INV("OpenSSL::Connection::sslWrite()");

    int bytesWritten = SSL_write(_sslConnection, buffer, bufferSize);

    setLastError(bytesWritten);

    ICE_METHOD_RET("OpenSSL::Connection::sslWrite()");

    return bytesWritten;
}


void
IceSecurity::Ssl::OpenSSL::Connection::printGetError(int errCode)
{
    if (ICE_SECURITY_LEVEL_PROTOCOL_DEBUG)
    {
        string errorString;

        switch (errCode)
        {
            case SSL_ERROR_NONE :
            {
                errorString = "SSL_ERROR_NONE";
                break;
            }

            case SSL_ERROR_ZERO_RETURN :
            {
                errorString = "SSL_ERROR_ZERO_RETURN";
                break;
            }

            case SSL_ERROR_WANT_READ :
            {
                errorString = "SSL_ERROR_WANT_READ";
                break;
            }

            case SSL_ERROR_WANT_WRITE :
            {
                errorString = "SSL_ERROR_WANT_WRITE";
                break;
            }

            case SSL_ERROR_WANT_CONNECT :
            {
                errorString = "SSL_ERROR_WANT_CONNECT";
                break;
            }

            case SSL_ERROR_WANT_X509_LOOKUP :
            {
                errorString = "SSL_ERROR_WANT_X509_LOOKUP";
                break;
            }

            case SSL_ERROR_SYSCALL :
            {
                errorString = "SSL_ERROR_SYSCALL";
                break;
            }

            case SSL_ERROR_SSL :
            {
                errorString = "SSL_ERROR_SSL";
                break;
            }
        }

        if (!errorString.empty())
        {
            ICE_SECURITY_LOGGER(string("Encountered: ") + errorString)
        }
    }
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
    ICE_METHOD_INV("OpenSSL::Connection::protocolWrite()");

    static char buffer[10];

    memset(buffer, 0, sizeof(buffer));

    // Note: We should be calling the write(char*,int) method here,
    //       not the write(Buffer&,int) method.  If things start acting
    //       strangely, check this!
    sslWrite(buffer,0);

    ICE_METHOD_RET("OpenSSL::Connection::protocolWrite()");
}

int
IceSecurity::Ssl::OpenSSL::Connection::readInBuffer(Buffer& buf)
{
    JTCSyncT<JTCMutex> sync(_inBufferMutex);

    ICE_METHOD_INV("OpenSSL::Connection::readInBuffer()");

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

        if (ICE_SECURITY_LEVEL_PROTOCOL)
        {
            string protocolString = "Copied ";
            protocolString += Int(bytesRead);
            protocolString += string(" bytes from SSL buffer\n");
            protocolString += fdToString(SSL_get_fd(_sslConnection));

            ICE_PROTOCOL(protocolString);
        }
    }

    ICE_METHOD_RET("OpenSSL::Connection::readInBuffer()");

    return bytesRead;
}

int
IceSecurity::Ssl::OpenSSL::Connection::readSelect(int timeout)
{
    ICE_METHOD_INV("OpenSSL::Connection::readSelect()");

    int ret;
    int fd = SSL_get_fd(_sslConnection);
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

    ICE_METHOD_RET("OpenSSL::Connection::readSelect()");

    return FD_ISSET(fd, &rFdSet);
}

int
IceSecurity::Ssl::OpenSSL::Connection::writeSelect(int timeout)
{
    ICE_METHOD_INV("OpenSSL::Connection::writeSelect()");

    int ret;
    int fd = SSL_get_fd(_sslConnection);
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

    ICE_METHOD_RET("OpenSSL::Connection::writeSelect()");
    
    return FD_ISSET(fd, &wFdSet);
}

int
IceSecurity::Ssl::OpenSSL::Connection::readSSL(Buffer& buf, int timeout)
{
    ICE_METHOD_INV("OpenSSL::Connection::readSSL()");

    int packetSize = buf.b.end() - buf.i;
    int totalBytesRead = 0;
    int bytesPending;
    int bytesRead;

    // We keep reading until we're done.
    while (buf.i != buf.b.end())
    {
        // Ensure we're initialized.
        if (init(timeout))
        {
            bytesPending = pending();

            if (!bytesPending && readSelect(timeout))
            {
                bytesPending = 1;
            }

            if (!bytesPending)
            {
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
                        ostringstream s;

                        s << "received " << bytesRead << " of " << packetSize;
                        s << " bytes via SSL\n" << fdToString(SSL_get_fd(_sslConnection));

                        ICE_PROTOCOL(s.str());

                        totalBytesRead += bytesRead;

                        buf.i += bytesRead;

                        if (packetSize > buf.b.end() - buf.i)
                        {
                            packetSize = buf.b.end() - buf.i;
                        }
                    }
                    else
                    {
                        // TODO: The client application performs a cleanup at this point,
                        //       not even shutting down SSL - it just frees the SSL
                        //       structure. The server does nothing.  I'm ignoring this,
                        //       at the moment, I'm sure it will come back at me.

                        ICE_PROTOCOL("Error SSL_ERROR_NONE: Repeating as per protocol.");
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

                    ICE_PROTOCOL("Error SSL_ERROR_WANT_WRITE.");

                    protocolWrite();

                    continue;
                }

                case SSL_ERROR_WANT_READ:
                {
                    // Repeat with the same arguments! (as in the OpenSSL documentation)
                    // Whatever happened, the last read didn't actually read anything for
                    // us.  This is effectively a retry.

                    ICE_PROTOCOL("Error SSL_ERROR_WANT_READ: Repeating as per protocol.");

                    continue;
                }

                case SSL_ERROR_WANT_X509_LOOKUP:
                {
                    // Perform another read.  The read should take care of this.

                    ICE_PROTOCOL("Error SSL_ERROR_WANT_X509_LOOKUP: Repeating as per protocol.");

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
                        protocolEx._message = "Encountered an EOF that violates the SSL Protocol.";

                        ICE_SSLERRORS(protocolEx._message);
                        ICE_EXCEPTION(protocolEx._message);

                        throw protocolEx;
                    }
                }

                case SSL_ERROR_SSL:
                {
                    ProtocolException protocolEx(__FILE__, __LINE__);

                    protocolEx._message = "Encountered a violation of the SSL Protocol.";

                    ICE_SSLERRORS(protocolEx._message);
                    ICE_EXCEPTION(protocolEx._message);

                    throw protocolEx;
                }

                case SSL_ERROR_ZERO_RETURN:
                {
                    // string errorString = "SSL_ERROR_ZERO_RETURN";
                    // ICE_EXCEPTION(errorString);
                    // throw ShutdownException(errorString.c_str(), __FILE__, __LINE__);

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
            }
        }
    }

    ICE_METHOD_RET("OpenSSL::Connection::readSSL()");

    return totalBytesRead;
}

string
IceSecurity::Ssl::OpenSSL::Connection::sslGetErrors()
{
    ICE_METHOD_INV("OpenSSL::Connection::sslGetErrors()");

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

    ICE_METHOD_RET("OpenSSL::Connection::sslGetErrors()");

    return errorMessage;
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
