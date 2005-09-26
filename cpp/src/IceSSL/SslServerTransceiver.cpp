// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerUtil.h>
#include <Ice/Stats.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/OpenSSLPluginI.h>
#include <IceSSL/TraceLevels.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/Exception.h>
#include <IceSSL/OpenSSLJanitors.h>
#include <IceSSL/SslServerTransceiver.h>

#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

//
// Public Methods
//

void
IceSSL::SslServerTransceiver::write(Buffer& buf, int timeout)
{
    assert(_fd != INVALID_SOCKET);

    _plugin->registerThread();

    int totalBytesWritten = 0;
    int bytesWritten = 0;

    Buffer::Container::difference_type packetSize = 
        static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);

#ifdef _WIN32
    //
    // Limit packet size to avoid performance problems on WIN32.
    //
    if(packetSize > 64 * 1024)
    {
        packetSize = 64 * 1024;
    }
#endif

    // We keep writing until we're done.
    while(buf.i != buf.b.end())
    {
        // Ensure we're initialized.
        if(initialize(timeout) <= 0)
        {
            // Retry the initialize call
            continue;
        }

        // initialize() must have returned > 0, so we're okay to try a write.

        // Perform a select on the socket.
        if(!writeSelect(timeout))
        {
            // We're done here.
            break;
        }

        bytesWritten = sslWrite(&*buf.i, static_cast<Int>(packetSize));

        switch(getLastError())
        {
            case SSL_ERROR_NONE:
            {
	        if(_traceLevels->network >= 3)
	        {
                    Trace out(_logger, _traceLevels->networkCat);
                    out << "sent " << bytesWritten << " of " << packetSize;
                    out << " bytes via ssl\n" << fdToString(SSL_get_fd(_sslConnection));
                }

		if(_stats)
		{
		    _stats->bytesSent(type(), bytesWritten);
		}

                totalBytesWritten += bytesWritten;

                buf.i += bytesWritten;

                if(packetSize > buf.b.end() - buf.i)
                {
                    packetSize = static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);
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
                if(bytesWritten == -1)
                {
                    // IO Error in underlying BIO

	            if(interrupted())
	            {
		        break;
	            }

                    if(wouldBlock())
	            {
                        break;
                    }

                    if(connectionLost())
	            {
	                ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = getSocketErrno();
                        throw ex;
	            }

                    SocketException ex(__FILE__, __LINE__);
		    ex.error = getSocketErrno();
		    throw ex;
                }
                else
                {
                    ProtocolException protocolEx(__FILE__, __LINE__);

                    // Protocol Error: Unexpected EOF.
                    protocolEx.message = "encountered an EOF that violates the ssl protocol\n";
                    protocolEx.message += IceSSL::sslGetErrors();

                    throw protocolEx;
                }
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx.message = "encountered a violation of the ssl protocol\n";
                protocolEx.message += IceSSL::sslGetErrors();

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
}

int
IceSSL::SslServerTransceiver::handshake(int timeout)
{
    assert(_sslConnection != 0);

    int retCode = SSL_is_init_finished(_sslConnection);
    
    while(!retCode)
    {
        if(_initWantWrite)
        {
            int i = writeSelect(timeout);

            if(i == 0)
            {
                return 0;
            }

            _initWantWrite = 0;
        }
        else
        {
            int i = readSelect(timeout);

            if(i == 0)
            {
                return 0;
            }
        }

        if(_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
        {
 	    Trace out(_logger, _traceLevels->securityCat);
            out << "Performing handshake.\n";
            out << fdToString(SSL_get_fd(_sslConnection));
        }

        int result = accept();

        // We're doing an Accept and we don't get a retry on the socket.
        if((result <= 0) && (BIO_sock_should_retry(result) == 0))
        {
            // Socket can't retry - bad scene, find out why.
            long verifyError = SSL_get_verify_result(_sslConnection);

            if(verifyError != X509_V_OK)
            {
                // Flag the connection for shutdown, let the
                // usual initialization take care of it.

                _phase = Shutdown;

                return 0;
            }
            else
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx.message = "encountered an ssl protocol violation during handshake\n";
                protocolEx.message += IceSSL::sslGetErrors();

                throw protocolEx;
            }
        }

        // Find out what the error was (if any).
        switch(getLastError())
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
                    if(interrupted())
                    {
                        break;
                    }

                    if(wouldBlock())
                    {
                        readSelect(timeout);
                        break;
                    }

                    if(connectionLost())
                    {
                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = getSocketErrno();
                        throw ex;
                    }

                    SocketException ex(__FILE__, __LINE__);
                    ex.error = getSocketErrno();
                    throw ex;
                }
                else
                {
                    //
                    // NOTE: Should this be ConnectionRefusedException like in the Client?
                    //

                    ProtocolException protocolEx(__FILE__, __LINE__);

                    // Protocol Error: Unexpected EOF
                    protocolEx.message = "encountered an eof during handshake that violates the ssl protocol\n";
                    protocolEx.message += IceSSL::sslGetErrors();

                    throw protocolEx;
                }
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException protocolEx(__FILE__, __LINE__);

                protocolEx.message = "encountered a violation of the ssl protocol during handshake\n";
                protocolEx.message += IceSSL::sslGetErrors();

                throw protocolEx;
            }

            case SSL_ERROR_ZERO_RETURN:
            {
		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
            }
        }

        retCode = SSL_is_init_finished(_sslConnection);

        if(retCode > 0)
        {
            _phase = Connected;

            // Init finished, look at the connection information.
            showConnectionInfo();
        }
    }

    return retCode;
}

//
// Protected Methods
//

void
IceSSL::SslServerTransceiver::showConnectionInfo()
{
    if((_traceLevels->security >= IceSSL::SECURITY_PROTOCOL_DEBUG) && 0)
    {
#ifdef ICE_SSL_EXTRA_TRACING
	//
        // Only in extreme cases do we enable this, partially because it doesn't use the Logger.
	//
        BIOJanitor bioJanitor(BIO_new_fp(stdout, BIO_NOCLOSE));
        BIO* bio = bioJanitor.get();

        showCertificateChain(bio);

        showPeerCertificate(bio,"Server");

        showSharedCiphers(bio);

        showSelectedCipherInfo(bio);

        showHandshakeStats(bio);

        showSessionInfo(bio);
#endif
    }
}

// Note: I would use a using directive of the form:
//       using IceSSL::CertificateVerifierPtr;
//       but unfortunately, it appears that this is not properly picked up.
//

IceSSL::SslServerTransceiver::SslServerTransceiver(const OpenSSLPluginIPtr& plugin,
                                                   SOCKET fd,
                                                   const IceSSL::CertificateVerifierPtr& certVerifier,
                                                   SSL* sslConnection,
						   int timeout) :
    SslTransceiver(plugin, fd, certVerifier, sslConnection, timeout)
{
    // Set the Accept Connection state for this connection.
    SSL_set_accept_state(sslConnection);
}


