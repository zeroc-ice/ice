// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <string>
#include <sstream>
#include <Ice/Network.h>
#include <Ice/OpenSSL.h>
#include <Ice/SecurityException.h>
#include <Ice/SslConnectionOpenSSLServer.h>

#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>

using IceSSL::CertificateException;
using IceSSL::ProtocolException;
using IceSSL::SystemInternalPtr;

using Ice::ConnectionLostException;
using Ice::SocketException;

using namespace IceInternal;
using namespace std;

using std::string;
using std::ostringstream;
using std::hex;
using std::dec;

//////////////////////////////////////
////////// ServerConnection //////////
//////////////////////////////////////

//
// Public Methods
//

// Note: I would use a using directive of the form:
//       using IceSSL::CertificateVerifierPtr;
//       but unfortunately, it appears that this is not properly picked up.
//

IceSSL::OpenSSL::ServerConnection::ServerConnection(
            const IceInternal::TraceLevelsPtr& traceLevels,
            const Ice::LoggerPtr& logger,
            const IceSSL::CertificateVerifierPtr& certificateVerifier,
            SSL* connection,
            const IceSSL::SystemInternalPtr& system) :
                                            Connection(traceLevels, logger, certificateVerifier, connection, system)
{
    // Set the Accept Connection state for this connection.
    SSL_set_accept_state(_sslConnection);
}

IceSSL::OpenSSL::ServerConnection::~ServerConnection()
{
}

void
IceSSL::OpenSSL::ServerConnection::shutdown()
{
    Connection::shutdown();
}

int
IceSSL::OpenSSL::ServerConnection::init(int timeout)
{
    int retCode = SSL_is_init_finished(_sslConnection);
    
    while (!retCode)
    {
        int i = 0;

        _readTimeout = timeout > _handshakeReadTimeout ? timeout : _handshakeReadTimeout;

        if (_initWantRead)
        {
            i = readSelect(_readTimeout);
        }
        else if (_initWantWrite)
        {
            i = writeSelect(timeout);
        }

        if (_initWantRead && i == 0)
        {
            return 0;
        }

        if (_initWantWrite && i == 0)
        {
            return 0;
        }

        _initWantRead = 0;
        _initWantWrite = 0;

        int result = accept();

        // Find out what the error was (if any).
        int code = getLastError();

        // We're doing an Accept and we don't get a retry on the socket.
        if ((result <= 0) && (BIO_sock_should_retry(result) == 0))
        {
            // Socket can't retry - bad scene, find out why.
            long verify_error = SSL_get_verify_result(_sslConnection);

            if (verify_error != X509_V_OK)
            {
                CertificateException certEx(__FILE__, __LINE__);

                certEx._message = "SSL certificate verification error.";

                throw certEx;
            }
            else
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx._message = "Encountered an SSL Protocol violation during handshake.\n";
                protocolEx._message += sslGetErrors();

                throw protocolEx;
            }
        }

        switch (code)
        {
            case SSL_ERROR_WANT_READ:
            {
                _initWantRead = 1;
                break;
            }

            case SSL_ERROR_WANT_WRITE:
            {
                _initWantWrite = 1;
                break;
            }

            
            case SSL_ERROR_NONE:
            case SSL_ERROR_WANT_X509_LOOKUP:
            {
                // Do nothing, life is good!
                break;
            }

            case SSL_ERROR_SYSCALL:
            {
                // This is a SOCKET_ERROR, but we don't use
                // this define here as OpenSSL doesn't refer
                // to it as a SOCKET_ERROR (but that's what it is
                // if you look at their code).
                if(result == -1)
                {
                    if (interrupted())
                    {
                        break;
                    }

                    if (wouldBlock())
                    {
                        readSelect(_readTimeout);
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
                else
                {
                    ProtocolException protocolEx(__FILE__, __LINE__);

                    // Protocol Error: Unexpected EOF
                    protocolEx._message = "Encountered an EOF during handshake that violates the SSL Protocol.\n";
                    protocolEx._message += sslGetErrors();

                    throw protocolEx;
                }
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx._message = "Encountered a violation of the SSL Protocol during handshake.\n";
                protocolEx._message += sslGetErrors();

                throw protocolEx;
            }
        }

        retCode = SSL_is_init_finished(_sslConnection);

        if (retCode > 0)
        {
            // Init finished, look at the connection information.
            showConnectionInfo();
        }
    }

    return retCode;
}

int
IceSSL::OpenSSL::ServerConnection::read(Buffer& buf, int timeout)
{
    int bytesRead = 1;
    int totalBytesRead = 0;

    // We keep reading until we're done.
    while ((buf.i != buf.b.end()) && bytesRead)
    {
        // Copy over bytes from _inBuffer to buf.
        bytesRead = readInBuffer(buf);

        // Nothing in the _inBuffer?
        if (!bytesRead)
        {
            // Read from SSL.
            bytesRead = readSSL(buf, timeout);
        }

        // Keep track of the total bytes read.
        totalBytesRead += bytesRead;
    }

    return totalBytesRead;
}

int
IceSSL::OpenSSL::ServerConnection::write(Buffer& buf, int timeout)
{
    int totalBytesWritten = 0;
    int bytesWritten = 0;

    int packetSize = buf.b.end() - buf.i;

#ifdef WIN32
    //
    // Limit packet size to avoid performance problems on WIN32.
    //
    if (packetSize > 64 * 1024)
    {
        packetSize = 64 * 1024;
    }
#endif

    int initReturn = 0;

    // We keep writing until we're done.
    while (buf.i != buf.b.end())
    {
        // Ensure we're initialized.
        initReturn = initialize(timeout);

        if (initReturn <= 0)
        {
            // Retry the initialize call
            continue;
        }

        // initReturn must be > 0, so we're okay to try a write

        // Perform a select on the socket.
        if (!writeSelect(timeout))
        {
            // We're done here.
            break;
        }

        bytesWritten = sslWrite((char *)buf.i, packetSize);

        switch (getLastError())
        {
            case SSL_ERROR_NONE:
            {
	        if (_traceLevels->network >= 3)
	        {
                    ostringstream s;
                    s << "sent " << bytesWritten << " of " << packetSize;
                    s << " bytes via ssl\n" << fdToString(SSL_get_fd(_sslConnection));
	            _logger->trace(_traceLevels->networkCat, s.str());
                }

                totalBytesWritten += bytesWritten;

                buf.i += bytesWritten;

                if (packetSize > buf.b.end() - buf.i)
                {
                    packetSize = buf.b.end() - buf.i;
                }
                continue;
            }

            case SSL_ERROR_WANT_WRITE:  // Retry...
            case SSL_ERROR_WANT_READ:   // The demo server ignores this error.
            case SSL_ERROR_WANT_X509_LOOKUP:    // The demo server ignores this error.
            {
                continue;
            }

            case SSL_ERROR_SYSCALL:
            {
                if (bytesWritten == -1)
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
                else
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
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }
    }

    return totalBytesWritten;
}

//
// Protected Methods
//

void
IceSSL::OpenSSL::ServerConnection::showConnectionInfo()
{
    // Only in extreme cases do we enable this, partially because it doesn't use the Logger.
    if ((_traceLevels->security >= IceSSL::SECURITY_PROTOCOL_DEBUG) && 0)
    {
        BIO* bio = BIO_new_fp(stdout, BIO_NOCLOSE);

        showCertificateChain(bio);

        showPeerCertificate(bio,"Server");

        showSharedCiphers(bio);

        showSelectedCipherInfo(bio);

        showHandshakeStats(bio);

        showSessionInfo(bio);

        if (bio != 0)
        {
            BIO_free(bio);
            bio = 0;
        }
    }
}
