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
#include <Ice/OpenSSLUtils.h>
#include <Ice/Network.h>
#include <Ice/OpenSSL.h>
#include <Ice/LocalException.h>
#include <Ice/SslException.h>
#include <Ice/OpenSSLJanitors.h>
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

IceSSL::OpenSSL::ServerConnection::ServerConnection(const IceInternal::TraceLevelsPtr& traceLevels,
                                                    const Ice::LoggerPtr& logger,
                                                    const IceSSL::CertificateVerifierPtr& certificateVerifier,
                                                    SSL* connection,
                                                    const IceSSL::SystemInternalPtr& system) :
                                  Connection(traceLevels, logger, certificateVerifier, connection, system)
{
    assert(_sslConnection != 0);

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
    assert(_sslConnection != 0);

    int retCode = SSL_is_init_finished(_sslConnection);
    
    while (!retCode)
    {
        int i = 0;

        _readTimeout = timeout > _handshakeReadTimeout ? timeout : _handshakeReadTimeout;

        if (_initWantWrite)
        {
            i = writeSelect(timeout);

            if (i == 0)
            {
                return 0;
            }

            _initWantWrite = 0;
        }
        else
        {
            i = readSelect(_readTimeout);

            if (i == 0)
            {
                return 0;
            }
        }

        int result = accept();

        // We're doing an Accept and we don't get a retry on the socket.
        if ((result <= 0) && (BIO_sock_should_retry(result) == 0))
        {
            // Socket can't retry - bad scene, find out why.
            long verifyError = SSL_get_verify_result(_sslConnection);

            if (verifyError != X509_V_OK)
            {
                CertificateVerificationException certVerEx(__FILE__, __LINE__);

                certVerEx._message = "ssl certificate verification error";

                string errors = sslGetErrors();

                if (!errors.empty())
                {
                    certVerEx._message += "\n";
                    certVerEx._message += errors;
                }

                throw certVerEx;
            }
            else
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx._message = "encountered an ssl protocol violation during handshake\n";
                protocolEx._message += sslGetErrors();

                throw protocolEx;
            }
        }

        // Find out what the error was (if any).
        switch (getLastError())
        {
            case SSL_ERROR_WANT_WRITE:
            {
                _initWantWrite = 1;
                break;
            }
            
            case SSL_ERROR_WANT_READ:
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
                    protocolEx._message = "encountered an eof during handshake that violates the ssl protocol\n";
                    protocolEx._message += sslGetErrors();

                    throw protocolEx;
                }
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx._message = "encountered a violation of the ssl protocol during handshake\n";
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
IceSSL::OpenSSL::ServerConnection::write(Buffer& buf, int timeout)
{
    int totalBytesWritten = 0;
    int bytesWritten = 0;

    int packetSize = buf.b.end() - buf.i;

#ifdef _WIN32
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
        if (initialize(timeout) <= 0)
        {
            // Retry the initialize call
            continue;
        }

        // initialize() must have returned > 0, so we're okay to try a write.

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

            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_X509_LOOKUP:
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

                    // Protocol Error: Unexpected EOF.
                    protocolEx._message = "encountered an EOF that violates the ssl protocol\n";
                    protocolEx._message += sslGetErrors();

                    throw protocolEx;
                }
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx._message = "encountered a violation of the ssl protocol\n";
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
        BIOJanitor bioJanitor(BIO_new_fp(stdout, BIO_NOCLOSE));
        BIO* bio = bioJanitor.get();

        showCertificateChain(bio);

        showPeerCertificate(bio,"Server");

        showSharedCiphers(bio);

        showSelectedCipherInfo(bio);

        showHandshakeStats(bio);

        showSessionInfo(bio);
    }
}
