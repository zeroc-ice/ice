// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Network.h>
#include <Ice/Logger.h>
#include <Ice/LocalException.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/Exception.h>
#include <IceSSL/OpenSSLJanitors.h>
#include <IceSSL/SslConnectionOpenSSLClient.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

////////////////////////////////////////////////
////////// SslConnectionOpenSSLClient //////////
////////////////////////////////////////////////

//
// Public Methods
//

// Note: I would use a using directive of the form:
//       using IceSSL::CertificateVerifierPtr;
//       but unfortunately, it appears that this is not properly picked up.
//

IceSSL::OpenSSL::ClientConnection::ClientConnection(const IceSSL::CertificateVerifierPtr& certificateVerifier,
                                                    SSL* connection,
                                                    const PluginBaseIPtr& plugin) :
    Connection(certificateVerifier, connection, plugin)
{
    assert(_sslConnection != 0);

    // Set the Connect Connection state for this connection.
    SSL_set_connect_state(_sslConnection);
}

IceSSL::OpenSSL::ClientConnection::~ClientConnection()
{
}

void
IceSSL::OpenSSL::ClientConnection::shutdown()
{
    Connection::shutdown();
}

int
IceSSL::OpenSSL::ClientConnection::init(int timeout)
{
    assert(_sslConnection != 0);

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

        int result = connect();

        switch (getLastError())
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
                // Retry connect.
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
                    // IO Error in underlying BIO

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
                else    // result == 0
                {
                    //
                    // The OpenSSL docs say that a result code of 0 indicates
                    // a graceful shutdown. In order to cause a retry in the
                    // Ice core, we raise ConnectFailedException. However,
                    // errno isn't set in this situation, so we always use
                    // ECONNREFUSED.
                    //
                    ConnectFailedException ex(__FILE__, __LINE__);
#ifdef _WIN32
                    ex.error = WSAECONNREFUSED;
#else
                    ex.error = ECONNREFUSED;
#endif
                    throw ex;
                }
            }

            case SSL_ERROR_SSL:
            {
                int verifyError = SSL_get_verify_result(_sslConnection);

                if (verifyError != X509_V_OK && verifyError != 1)
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

                    protocolEx._message = "encountered a violation of the ssl protocol during handshake\n";
                    protocolEx._message += sslGetErrors();

                    throw protocolEx;
                }
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
IceSSL::OpenSSL::ClientConnection::write(Buffer& buf, int timeout)
{
    int totalBytesWritten = 0;
    int bytesWritten = 0;

    int packetSize = buf.b.end() - buf.i;

#ifdef _WIN32
    //
    // Limit packet size to avoid performance problems on WIN32.
    // (blatantly ripped off from Marc Laukien)
    //
    if (packetSize > 64 * 1024)
    {
        packetSize = 64 * 1024;
    }
#endif

    // We keep reading until we're done
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
                if (bytesWritten > 0)
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
                }
                continue;
            }

            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_X509_LOOKUP:
            {
                // Perform another read.  The read should take care of this.
                continue;
            }

            case SSL_ERROR_SYSCALL:
            {
                // NOTE:  The OpenSSL demo client only raises and error condition if there were
                //        actually bytes written.  This is considered to be an error status
                //        requiring shutdown.
                //        If nothing was written, the demo client stops writing - we continue.
                //        This is potentially something wierd to watch out for.
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
                else if (bytesWritten > 0)
                {
                    ProtocolException protocolEx(__FILE__, __LINE__);

                    // Protocol Error: Unexpected EOF
                    protocolEx._message = "encountered an EOF that violates the ssl protocol\n";
                    protocolEx._message += sslGetErrors();

                    throw protocolEx;
                }
                else // bytesWritten == 0
                {
                    // Didn't write anything, continue, should be fine.
                    break;
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
IceSSL::OpenSSL::ClientConnection::showConnectionInfo()
{
    // Only in extreme cases do we enable this, partially because it doesn't use the Logger.
    if ((_traceLevels->security >= IceSSL::SECURITY_PROTOCOL_DEBUG) && 0)
    {
        BIOJanitor bioJanitor(BIO_new_fp(stdout, BIO_NOCLOSE));
        BIO* bio = bioJanitor.get();

        showCertificateChain(bio);

        showPeerCertificate(bio,"Client");

        showClientCAList(bio, "Client");

        showSharedCiphers(bio);

        showSelectedCipherInfo(bio);

        showHandshakeStats(bio);

        showSessionInfo(bio);
    }
}
