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
#include <Ice/Security.h>
#include <Ice/SecurityException.h>
#include <Ice/SslConnectionOpenSSLServer.h>

using IceSecurity::Ssl::CertificateException;
using IceSecurity::Ssl::ProtocolException;
using namespace IceInternal;
using namespace std;

//////////////////////////////////////
////////// ServerConnection //////////
//////////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::OpenSSL::ServerConnection::ServerConnection(SSL* connection, string& systemID) :
                                            Connection(connection, systemID)
{
}

IceSecurity::Ssl::OpenSSL::ServerConnection::~ServerConnection()
{
    ICE_METHOD_INV("OpenSSL::ServerConnection::~ServerConnection()");

    ICE_METHOD_RET("OpenSSL::ServerConnection::~ServerConnection()");
}

void
IceSecurity::Ssl::OpenSSL::ServerConnection::shutdown()
{
    ICE_METHOD_INV("OpenSSL::ServerConnection::shutdown()");

    Connection::shutdown();

    ICE_METHOD_RET("OpenSSL::ServerConnection::shutdown()");
}

int
IceSecurity::Ssl::OpenSSL::ServerConnection::init(int timeout)
{
    ICE_METHOD_INV("OpenSSL::ServerConnection::init()");

    if (_timeoutEncountered)
    {
        throw TimeoutException(__FILE__, __LINE__);
    }

    int retCode = SSL_is_init_finished(_sslConnection);
    
    while (!retCode)
    {
        int i = 0;

        _readTimeout = timeout > _handshakeReadTimeout ? timeout : _handshakeReadTimeout;

        try
        {
            if (_initWantRead)
            {
                i = readSelect(_readTimeout);
            }
            else if (_initWantWrite)
            {
                i = writeSelect(timeout);
            }
        }
        catch (const TimeoutException&)
        {
            _timeoutEncountered = true;
            throw;
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

                ICE_EXCEPTION(certEx._message);

                throw certEx;
            }
            else
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx._message = "Encountered an SSL Protocol violation during handshake.";

                ICE_SSLERRORS(protocolEx._message);
                ICE_EXCEPTION(protocolEx._message);

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
                        ICE_DEV_DEBUG("ServerConnection::init(): Throwing ConnectionLostException... SslConnectionOpenSSLServer.cpp, 207");
                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = getSocketErrno();
                        throw ex;
                    }
                    else
                    {
                        ICE_DEV_DEBUG("ServerConnection::init(): Throwing SocketException... SslConnectionOpenSSLServer.cpp, 214");
                        SocketException ex(__FILE__, __LINE__);
                        ex.error = getSocketErrno();
                        throw ex;
                    }
                }
                else
                {
                    ProtocolException protocolEx(__FILE__, __LINE__);

                    // Protocol Error: Unexpected EOF
                    protocolEx._message = "Encountered an EOF during handshake that violates the SSL Protocol.";

                    ICE_SSLERRORS(protocolEx._message);
                    ICE_EXCEPTION(protocolEx._message);

                    throw protocolEx;
                }
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx._message = "Encountered a violation of the SSL Protocol during handshake.";

                ICE_SSLERRORS(protocolEx._message);
                ICE_EXCEPTION(protocolEx._message);

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

    ICE_METHOD_RET("OpenSSL::ServerConnection::init()");

    return retCode;
}

int
IceSecurity::Ssl::OpenSSL::ServerConnection::read(Buffer& buf, int timeout)
{
    ICE_METHOD_INV("OpenSSL::ServerConnection::read(Buffer&, int)");

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

    ICE_METHOD_RET("OpenSSL::ServerConnection::read(Buffer&, int)");

    return totalBytesRead;
}

int
IceSecurity::Ssl::OpenSSL::ServerConnection::write(Buffer& buf, int timeout)
{
    ICE_METHOD_INV("OpenSSL::ServerConnection::write(Buffer&, int)");

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

    // We keep writing until we're done.
    while (buf.i != buf.b.end())
    {
        // Ensure we're initialized.
        int initReturn = initialize(timeout);

        if (initReturn == -1)
        {
            // Handshake underway, we should just return with what we've got (even if that's nothing).
            break;
        }

        if (initReturn == 0)
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
            {
                ICE_PROTOCOL("Error SSL_ERROR_WANT_WRITE: Repeating as per protocol.");

                continue;
            }

            case SSL_ERROR_WANT_READ:   // The demo server ignores this error.
            {
                ICE_PROTOCOL("Error SSL_ERROR_WANT_READ: Ignoring as per protocol.");

                continue;
            }

            case SSL_ERROR_WANT_X509_LOOKUP:    // The demo server ignores this error.
            {
                ICE_PROTOCOL("Error SSL_ERROR_WANT_X509_LOOKUP: Repeating as per protocol.");

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
                        ICE_DEV_DEBUG("ServerConnection::write(): Throwing ConnectionLostException... SslConnectionOpenSSLServer.cpp, 388");
	                ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = getSocketErrno();
                        throw ex;
	            }
	            else
	            {
                        ICE_DEV_DEBUG("ServerConnection::write(): Throwing SocketException... SslConnectionOpenSSLServer.cpp, 395");
		        SocketException ex(__FILE__, __LINE__);
		        ex.error = getSocketErrno();
		        throw ex;
	            }
                }
                else
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
                ICE_EXCEPTION("SSL_ERROR_ZERO_RETURN");
                ICE_DEV_DEBUG("ServerConnection::write(): Throwing ConnectionLostException... SslConnectionOpenSSLServer.cpp, 430");

                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }
    }

    ICE_METHOD_RET("OpenSSL::ServerConnection::write(Buffer&, int)");

    return totalBytesWritten;
}

//
// Protected Methods
//

void
IceSecurity::Ssl::OpenSSL::ServerConnection::showConnectionInfo()
{
    ICE_METHOD_INV("OpenSSL::ServerConnection::showConnectionInfo()");

    // Only in extreme cases do we enable this, partially because it doesn't use the Logger.
    if (ICE_SECURITY_LEVEL_PROTOCOL_DEBUG)
    {
        ICE_PROTOCOL_DEBUG("Begin Connection Information");

        BIO* bio = BIO_new_fp(stdout, BIO_NOCLOSE);

        showCertificateChain(bio);

        showPeerCertificate(bio,"Server");

        showSharedCiphers(bio);

        showSelectedCipherInfo(bio);

        showHandshakeStats(bio);

        showSessionInfo(bio);

        ICE_PROTOCOL_DEBUG("End of Connection Information");

        if (bio != 0)
        {
            BIO_free(bio);
            bio = 0;
        }
    }

    ICE_METHOD_RET("OpenSSL::ServerConnection::showConnectionInfo()");
}
