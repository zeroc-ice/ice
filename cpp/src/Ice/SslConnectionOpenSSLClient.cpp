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
#include <Ice/SslException.h>
#include <Ice/SslConnectionOpenSSLClient.h>

using IceSecurity::Ssl::ShutdownException;
using namespace IceInternal;
using namespace std;

////////////////////////////////////////////////
////////// SslConnectionOpenSSLClient //////////
////////////////////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::OpenSSL::ClientConnection::ClientConnection(SSL* connection, string& systemID) :
                                            Connection(connection, systemID)
{
}

IceSecurity::Ssl::OpenSSL::ClientConnection::~ClientConnection()
{
    METHOD_INV("OpenSSL::ClientConnection::~ClientConnection()");

    shutdown();

    METHOD_RET("OpenSSL::ClientConnection::~ClientConnection()");
}

void
IceSecurity::Ssl::OpenSSL::ClientConnection::shutdown()
{
    METHOD_INV("OpenSSL::ClientConnection::shutdown()");

    if (_sslConnection != 0)
    {
        int shutdown = 0;
        int retries = 100;

        do
        {
            shutdown = SSL_shutdown(_sslConnection);
            retries--;
        }
        while ((shutdown == 0) && (retries > 0));

        if (shutdown <= 0)
        {
            ostringstream s;
            s << "SSL shutdown failure encountered: code[" << shutdown << "] retries[";
            s << retries << "]\n" << fdToString(SSL_get_fd(_sslConnection));
            PROTOCOL_DEBUG(s.str());
        }
    }

    Connection::shutdown();

    METHOD_RET("OpenSSL::ClientConnection::shutdown()");
}

int
IceSecurity::Ssl::OpenSSL::ClientConnection::init(int timeout)
{
    JTCSyncT<JTCMutex> sync(_initMutex);

    int retCode = 0;

    METHOD_INV("OpenSSL::ClientConnection::init()");

    if (!SSL_is_init_finished(_sslConnection))
    {
        int i = 0;

        if (initWantRead)
        {
            i = readSelect(timeout);
        }
        else if (initWantWrite)
        {
            i = writeSelect(timeout);
        }

        if (initWantRead && i == 0)
        {
            return 0;
        }

        if (initWantWrite && i == 0)
        {
            return 0;
        }

        initWantRead = 0;
        initWantWrite = 0;

        int result = connect();

        // Find out what the error was (if any).
        int code = getLastError();

        printGetError(code);

        switch (code)
        {
            case SSL_ERROR_WANT_READ:
            {
                initWantRead = 1;
                break;
            }

            case SSL_ERROR_WANT_WRITE:
            {
                initWantWrite = 1;
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
                    if (interrupted())
                    {
                        break;
                    }

                    if (wouldBlock())
                    {
                        readSelect(timeout);
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
                    string errorString = "SSL_ERROR_SYSCALL: During init()";

                    EXCEPTION(errorString);

                    throw IceSecurity::Ssl::InitException(errorString.c_str(), __FILE__, __LINE__);
                }
            }

            case SSL_ERROR_SSL:
            {
                string errorString = "SSL_ERROR_SSL: During init()";

                EXCEPTION(errorString);

                throw IceSecurity::Ssl::InitException(errorString.c_str(), __FILE__, __LINE__);
            }
        }

        retCode = SSL_is_init_finished(_sslConnection);

        if (retCode > 0)
        {
            // Init finished, look at the connection information.
            showConnectionInfo();
        }
    }
    else
    {
        retCode = 1;
    }

    METHOD_RET("OpenSSL::ClientConnection::init()");

    return retCode;
}

int
IceSecurity::Ssl::OpenSSL::ClientConnection::read(Buffer& buf, int timeout)
{
    METHOD_INV("OpenSSL::ClientConnection::read(Buffer&,int)");

    int totalBytesRead = 0;

    // Initialization to 1 is a cheap trick to ensure we enter the loop.
    int bytesRead = 1;

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

        // Keep track of the total number of bytes read.
        totalBytesRead += bytesRead;
    }

    METHOD_RET("OpenSSL::ClientConnection::read(Buffer&,int)");

    return totalBytesRead;
}

int
IceSecurity::Ssl::OpenSSL::ClientConnection::write(Buffer& buf, int timeout)
{
    METHOD_INV("OpenSSL::ClientConnection::write(Buffer&,int)");

    int totalBytesWritten = 0;
    int bytesWritten = 0;

    int packetSize = buf.b.end() - buf.i;

#ifdef WIN32
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
        if (init(timeout))
        {
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
                        ostringstream s;

                        s << "sent " << bytesWritten << " of " << packetSize;
                        s << " bytes via SSL\n" << fdToString(SSL_get_fd(_sslConnection));

                        PROTOCOL(s.str());

                        totalBytesWritten += bytesWritten;

                        buf.i += bytesWritten;

                        if (packetSize > buf.b.end() - buf.i)
                        {
                            packetSize = buf.b.end() - buf.i;
                        }
                    }
                    else
                    {
                        // TODO: The client application performs a cleanup at this point,
                        //       not even shutting down SSL - it just frees the SSL
                        //       structure. I'm ignoring this, at the moment, as I'm sure
                        //       the demo is handling it in an artificial manner.

                        PROTOCOL("Error SSL_ERROR_NONE: Repeating as per protocol.");
                    }
                    continue;
                }

                case SSL_ERROR_WANT_WRITE:
                {
                    // Repeat with the same arguments! (as in the OpenSSL documentation)
                    // Whatever happened, the last write didn't actually write anything
                    // for us.  This is effectively a retry.

                    PROTOCOL("Error SSL_ERROR_WANT_WRITE: Repeating as per protocol.");

                    continue;
                }

                case SSL_ERROR_WANT_READ:
                {
                    // If we get this error here, it HAS to be because
                    // the protocol wants to do something handshake related.
                    // In the case that we might actually get some application data,
                    // we will use the base SSL read method, using the _inBuffer.

                    PROTOCOL("Error SSL_ERROR_WANT_READ.");

                    readSSL(_inBuffer, timeout);

                    continue;
                }

                case SSL_ERROR_WANT_X509_LOOKUP:
                {
                    // Perform another read.  The read should take care of this.

                    PROTOCOL("Error SSL_ERROR_WANT_X509_LOOKUP: Repeating as per protocol.");

                    continue;
                }

                case SSL_ERROR_SYSCALL:
                {
                    // NOTE:  The demo client only throws an exception if there were actually bytes
                    //        written.  This is considered to be an error status requiring shutdown.
                    //        If nothing was written, the demo client stops writing - we continue.
                    //        This is potentially something wierd to watch out for.
                    if (bytesWritten != 0)
                    {
                        string errorString = "SSL_ERROR_SYSCALL";

                        EXCEPTION(errorString);

                        throw ShutdownException(errorString.c_str(), __FILE__, __LINE__);
                    }
                    else
                    {
                        // Didn't write anything, continue, should be fine.

                        PROTOCOL("Error SSL_ERROR_SYSCALL: Repeating as per protocol.");

                        continue;
                    }
                }

                case SSL_ERROR_SSL:
                {
                    string errorString = "SSL_ERROR_SSL";

                    EXCEPTION(errorString);

                    throw ShutdownException(errorString.c_str(), __FILE__, __LINE__);
                }

                case SSL_ERROR_ZERO_RETURN:
                {
                    string errorString = "SSL_ERROR_ZERO_RETURN";

                    EXCEPTION(errorString);

                    throw ShutdownException(errorString.c_str(), __FILE__, __LINE__);
                }
            }
        }
    }

    METHOD_RET("OpenSSL::ClientConnection::write(Buffer&,int)");

    return totalBytesWritten;
}

//
// Protected Methods
//

// This code blatantly stolen from OpenSSL demos, slightly repackaged, and completely ugly...
void
IceSecurity::Ssl::OpenSSL::ClientConnection::showConnectionInfo()
{
    METHOD_INV("OpenSSL::ClientConnection::showConnectionInfo()");

    // Only in extreme cases do we enable this, partially because it doesn't use the Logger.
    if (SECURITY_LEVEL_PROTOCOL_DEBUG)
    {
        PROTOCOL_DEBUG("Begin Connection Information");

        BIO* bio = BIO_new_fp(stdout, BIO_NOCLOSE);

        showCertificateChain(bio);

        showPeerCertificate(bio,"Client");

        // Something extra for the client
        showClientCAList(bio, "Client");

        showSharedCiphers(bio);

        showSelectedCipherInfo(bio);

        showHandshakeStats(bio);

        showSessionInfo(bio);

        PROTOCOL_DEBUG("End of Connection Information");

        if (bio != 0)
        {
            BIO_free(bio);
            bio = 0;
        }
    }

    METHOD_RET("OpenSSL::ClientConnection::showConnectionInfo()");
}
