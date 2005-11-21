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
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/TraceLevels.h>
#include <IceSSL/Exception.h>
#include <IceSSL/OpenSSLPluginI.h>
#include <IceSSL/OpenSSLUtils.h>

#include <openssl/err.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

//
// Static Member Initialization
//
IceSSL::SslTransceiverMap IceSSL::SslTransceiver::_transceiverMap;
IceUtil::StaticMutex IceSSL::SslTransceiver::_transceiverRepositoryMutex = ICE_STATIC_MUTEX_INITIALIZER;

void IceInternal::incRef(IceSSL::SslTransceiver* p) { p->__incRef(); }
void IceInternal::decRef(IceSSL::SslTransceiver* p) { p->__decRef(); }

SOCKET
IceSSL::SslTransceiver::fd()
{
    assert(_fd != INVALID_SOCKET);
    return _fd;
}

void
IceSSL::SslTransceiver::close()
{
    if(_fd == INVALID_SOCKET)
    {
        // Ignore - the connection was never set up.
        return;
    }

    if(_traceLevels->network >= 1)
    {
   	Trace out(_logger, _traceLevels->networkCat);
	out << "closing ssl connection\n" << toString();
    }

    try
    {
	internalShutdownWrite(10 * 1000);
    }
    catch(const Ice::Exception& e)
    {
	Warning warn(_logger);
	warn << "error in shutting down ssl connection " << e;
    }

    assert(_fd != INVALID_SOCKET);
    try
    {
	closeSocket(_fd);
	_fd = INVALID_SOCKET;
    }
    catch(const SocketException&)
    {
	_fd = INVALID_SOCKET;
	throw;
    }
}

void
IceSSL::SslTransceiver::shutdownWrite()
{
    if(_traceLevels->network >= 2)
    {
 	Trace out(_logger, _traceLevels->networkCat);
	out << "shutting down ssl connection for writing\n" << toString();
    }

    try
    {
	internalShutdownWrite(10 * 1000);
    }
    catch(const Ice::Exception& e)
    {
	Warning warn(_logger);
	warn << "error in shutting down ssl connection " << e;
    }

    assert(_fd != INVALID_SOCKET);
    shutdownSocketWrite(_fd);
}

void
IceSSL::SslTransceiver::shutdownReadWrite()
{
    if(_traceLevels->network >= 2)
    {
 	Trace out(_logger, _traceLevels->networkCat);
	out << "shutting down ssl connection for reading and writing\n" << toString();
    }

    assert(_fd != INVALID_SOCKET);
    shutdownSocketReadWrite(_fd);
}

void
IceSSL::SslTransceiver::write(Buffer& buf, int timeout)
{
    assert(_fd != INVALID_SOCKET);

    _plugin->registerThread();


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
	ERR_clear_error();
	int ret = SSL_write(_sslConnection, &*buf.i, static_cast<int>(packetSize));
        switch(SSL_get_error(_sslConnection, ret))
        {
	    case SSL_ERROR_NONE:
		break;
	    
            case SSL_ERROR_WANT_WRITE:
	    {
		writeSelect(timeout);
		continue;
	    }

	    //
	    // If session renegotiation is ever enabled this could
	    // occur.
	    //
            //case SSL_ERROR_WANT_READ:
	    //{
	    //	  readSelect(timeout);
	    //	  continue;
	    //}

            case SSL_ERROR_SYSCALL:
            {
		if(ret == -1)
		{
		    // IO Error in underlying BIO

		    if(interrupted())
		    {
			continue;
		    }

		    if(noBuffers() && packetSize > 1024)
		    {
			packetSize /= 2;
			continue;
		    }

		    //
		    // Its not clear whether this can occur, isn't
		    // this the same as SSL_ERROR_WANT_WRITE?
		    //
		    if(wouldBlock())
		    {
			writeSelect(timeout);
			continue;
		    }

		    if(connectionLost())
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
		// fall through
            }

            case SSL_ERROR_ZERO_RETURN:
            {
		assert(ret == 0);
		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = 0;
		throw ex;
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException ex(__FILE__, __LINE__);
                ex.message = "encountered a violation of the ssl protocol\n";
                ex.message += IceSSL::sslGetErrors();
                throw ex;
            }

	    default:
	    {
		Warning warn(_logger);
		warn << "IceSSL: unexpected result from SSL_write: " << SSL_get_error(_sslConnection, ret);
	    }
        }

	if(_traceLevels->network >= 3)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "sent " << ret << " of " << packetSize; out << " bytes via ssl\n"
		<< fdToString(SSL_get_fd(_sslConnection));
	}

	if(_stats)
	{
	    _stats->bytesSent(type(), ret);
	}

	buf.i += ret;

	if(packetSize > buf.b.end() - buf.i)
	{
	    packetSize = static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);
	}
    }
}

void
IceSSL::SslTransceiver::read(Buffer& buf, int timeout)
{
    assert(_fd != INVALID_SOCKET);

    _plugin->registerThread();

    Buffer::Container::difference_type packetSize = 
        static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);

    while(buf.i != buf.b.end())
    {
	ERR_clear_error();
	int ret = SSL_read(_sslConnection, &*buf.i, static_cast<Int>(packetSize));
        switch(SSL_get_error(_sslConnection, ret))
        {
	    case SSL_ERROR_NONE:
		break;

            case SSL_ERROR_WANT_READ:
            {
                readSelect(timeout);
                continue;
            }

	    //
	    // If session renegotiation is ever enabled this could
	    // occur.
	    //
            //case SSL_ERROR_WANT_WRITE:
	    //{
	    //	  writeSelect(timeout);
	    //	  continue;
	    //}

            case SSL_ERROR_SYSCALL:
            {
                if(ret == -1)
                {
                    // IO Error in underlying BIO

                    if(interrupted())
                    {
                        continue;
                    }

		    if(noBuffers() && packetSize > 1024)
		    {
			packetSize /= 2;
			continue;
		    }

		    //
		    // Its not clear whether this can occur, isn't
		    // this the same as SSL_ERROR_WANT_READ?
		    //
                    if(wouldBlock())
                    {
			readSelect(timeout);
                        continue;
                    }

                    if(!connectionLost())
                    {
			SocketException ex(__FILE__, __LINE__);
			ex.error = getSocketErrno();
			throw ex;
		    }
                }
		// fall throught
	    }

            case SSL_ERROR_ZERO_RETURN:
	    {

		//
		// If the connection is lost when reading data, we shut
		// down the write end of the socket. This helps to unblock
		// threads that are stuck in send() or select() while
		// sending data. Note: I don't really understand why
		// send() or select() sometimes don't detect a connection
		// loss. Therefore this helper to make them detect it.
		//
		//assert(_fd != INVALID_SOCKET);
		//shutdownSocket(_fd);

		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
            }

            case SSL_ERROR_SSL:
            {
                ProtocolException ex(__FILE__, __LINE__);
                ex.message = "encountered a violation of the ssl protocol\n";
                ex.message += sslGetErrors();
                throw ex;
            }

	    default:
	    {
		Warning warn(_logger);
		warn << "IceSSL: unexpected result from SSL_write: " << SSL_get_error(_sslConnection, ret);
	    }
        }

	if(_traceLevels->network >= 3)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "received " << ret << " of " << packetSize; out << " bytes via ssl\n" << toString();
	}

	if(_stats)
	{
	    _stats->bytesReceived(type(), ret);
	}

	buf.i += ret;

	if(packetSize > buf.b.end() - buf.i)
	{
	    packetSize = static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);
	}
    }
}

string
IceSSL::SslTransceiver::type() const
{
    return "ssl";
}

string
IceSSL::SslTransceiver::toString() const
{
    return fdToString(_fd);
}

void
IceSSL::SslTransceiver::initialize(int timeout)
{
    assert(_sslConnection != 0);

    if(_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
    {
	Trace out(_logger, _traceLevels->securityCat);
	out << "Performing handshake.\n";
	out << fdToString(SSL_get_fd(_sslConnection));
    }

    while(true)
    {
	ERR_clear_error();
	int result;
	if(_contextType == IceSSL::Client)
	{
	    result = SSL_connect(_sslConnection);
	}
	else
	{
	    result = SSL_accept(_sslConnection);
	}

	//
	// Success?
	//
	if(result == 1)
	{
	    assert(SSL_is_init_finished(_sslConnection));

	    //
	    // Init finished, look at the connection information.
	    //
#ifdef ICE_SSL_EXTRA_TRACING	
	    if(_traceLevels->security >= IceSSL::SECURITY_PROTOCOL_DEBUG)
	    {
		//
		// Only in extreme cases do we enable this, partially because it doesn't use the Logger.
		//
		BIOJanitor bioJanitor(BIO_new_fp(stdout, BIO_NOCLOSE));
		BIO* bio = bioJanitor.get();
		
		showCertificateChain(bio);
		
		showPeerCertificate(bio, _contextType == IceSSL::Server : "Server" ? "Client");
		
		showSharedCiphers(bio);
		
		showSelectedCipherInfo(bio);
		
		showHandshakeStats(bio);
		
		showSessionInfo(bio);
	    }
#endif
	    return;
	}

	switch(SSL_get_error(_sslConnection, result))
	{
	    case SSL_ERROR_WANT_READ:
	    {
		readSelect(timeout);
		continue;
	    }

	    case SSL_ERROR_WANT_WRITE:
	    {
		writeSelect(timeout);
		continue;
	    }

	    case SSL_ERROR_NONE:
	    {
		continue;
	    }

	    case SSL_ERROR_WANT_X509_LOOKUP:
	    {
		Warning warn(_logger);
		warn << "SSL_ERROR_NONE";
		continue;
	    }

	    case SSL_ERROR_SYSCALL:
	    {
		if(result == -1)
		{
		    if(interrupted())
		    {
			break;
		    }

		    assert(!wouldBlock());

		    if(connectionLost())
		    {
			ConnectionLostException ex(__FILE__, __LINE__);
			ex.error = getSocketErrno();
			throw ex;
		    }
		}

		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }

	    case SSL_ERROR_SSL:
	    {
		long err = SSL_get_verify_result(_sslConnection);
		if(err != X509_V_OK)
		{
		    //
		    // On a client we raise the CertificateVerificationException. On the
		    // server side we simply close the connection.
		    //
		    if(_contextType == IceSSL::Client)
		    {
			CertificateVerificationException ex(__FILE__, __LINE__);
			ex.message = getVerificationError(err);
			string errors = sslGetErrors();
			if(!errors.empty())
			{
			    ex.message += "\n";
			    ex.message += errors;
			}
			throw ex;
		    }
		    else
		    {
			//
			// Validation failed, close the connection.
			//
			throw ConnectionRefusedException(__FILE__, __LINE__);
		    }
		}

		//
		// This happens if the client or server silently drop
		// the connection. That can occur if the server
		// doesn't trust the client for example.
		//
		ProtocolException ex(__FILE__, __LINE__);
		ex.message = "encountered a violation of the ssl protocol during handshake\n";
		ex.message += sslGetErrors();
		throw ex;
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

IceSSL::SslTransceiverPtr
IceSSL::SslTransceiver::getTransceiver(SSL* sslPtr)
{
    IceUtil::StaticMutex::Lock sync(_transceiverRepositoryMutex);

    assert(sslPtr);

    SslTransceiver* transceiver = _transceiverMap[sslPtr];

    assert(transceiver);

    return SslTransceiverPtr(transceiver);
}

//
// Note: Do not throw exceptions from verifyCertificate - it would rip
// through the OpenSSL system, interfering with the usual handling and
// alert system of the handshake.  Exceptions should be caught here
// (if they can be generated), logged and then a fail return code (0)
// should returned.
//
int
IceSSL::SslTransceiver::verifyCertificate(int preVerifyOkay, X509_STORE_CTX* x509StoreContext)
{
    // Should NEVER be able to happen.
    assert(_certificateVerifier.get() != 0);

    // Get the verifier, make sure it is for OpenSSL connections
    CertificateVerifierOpenSSLPtr verifier;
    verifier = dynamic_cast<CertificateVerifierOpenSSL*>(_certificateVerifier.get());

    // Check to make sure we have a proper verifier for the operation.
    if(verifier)
    {
        // Use the verifier to verify the certificate
        try
        {
            preVerifyOkay = verifier->verify(preVerifyOkay, x509StoreContext, _sslConnection);
        }
        catch(const Ice::LocalException& localEx)
        {
            if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
            {
 	        Trace out(_logger, _traceLevels->networkCat);
                out << "WRN exception during certificate verification: \n";
                out << localEx;
            }

            preVerifyOkay = 0;
        }
    }
    else
    {
        // Note: This code should NEVER be able to be reached, as we check each
        //       CertificateVerifier as it is added to the System.

        if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        {
 	    Trace out(_logger, _traceLevels->networkCat);

            if(_certificateVerifier.get())
            {
                out << "WRN improper CertificateVerifier type";
            }
            else
            {
                // NOTE: This should NEVER be able to happen, but just in case.
                out << "WRN CertificateVerifier not set";
            }
        }
    }

    return preVerifyOkay;
}

//
// Protected Methods
//

void
IceSSL::SslTransceiver::internalShutdownWrite(int timeout)
{
    while(true)
    {
	ERR_clear_error();
	int result = SSL_shutdown(_sslConnection);
	if(result == 0)
	{
	    //
	    // From the documentation:
	    //
	    // The shutdown is not yet finished. Call SSL_shutdown()
	    // for a second time, if a bidirectional shutdown shall be
	    // performed. The output of SSL_get_error(3) may be
	    // misleading, as an erroneous SSL_ERROR_SYSCALL may be
	    // flagged even though no error occurred.
	    //
	    // Call it one more time. If the result is 0 then we're done.
	    //
	    result = SSL_shutdown(_sslConnection);
	    if(result == 0)
	    {
		return;
	    }
	}
	if(result == 1)
	{
	    // Shutdown successful - shut down the socket for writing.
	    shutdownSocketWrite(SSL_get_fd(_sslConnection));
	    return;
	}
	else if(result == -1)
	{
	    switch(SSL_get_error(_sslConnection, result))
	    {
		case SSL_ERROR_WANT_WRITE:
		{
		    writeSelect(timeout);
		    continue;
		}

		case SSL_ERROR_WANT_READ:
		{
		    readSelect(timeout);
		    continue;
		}

		case SSL_ERROR_NONE:
		case SSL_ERROR_WANT_X509_LOOKUP:
		{
		    continue;
		}

		case SSL_ERROR_SYSCALL:
		{
		    if(interrupted())
		    {
			continue;
		    }

		    assert(!wouldBlock());

		    if(connectionLost())
		    {
			ConnectionLostException ex(__FILE__, __LINE__);
			ex.error = getSocketErrno();
			throw ex;
		    }

		    //
		    // Non-specific socket problem.
		    //
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = getSocketErrno();
		    throw ex;
		}

		case SSL_ERROR_SSL:
		{
		    //
		    // Error in the SSL library, usually a Protocol error.
		    //

		    ProtocolException ex(__FILE__, __LINE__);
		    ex.message = "encountered a violation of the ssl protocol during shutdown\n";
		    ex.message += sslGetErrors();
		    throw ex;
		}

		case SSL_ERROR_ZERO_RETURN:
		{
		    //
		    // Indicates that the SSL connection has been
		    // closed. For SSLv3.0 and TLSv1.0, it indicates
		    // that a closure alert was received, and thus the
		    // connection has been closed cleanly.
		    //
		    throw CloseConnectionException(__FILE__, __LINE__);
		}
	    }
	}
    }
}

void
IceSSL::SslTransceiver::select(int timeout, bool write)
{
    int ret;

    assert(_sslConnection != 0);
    SOCKET fd = SSL_get_fd(_sslConnection);

    fd_set rwFdSet;
    struct timeval tv;

    if(timeout >= 0)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
    }

    do
    {
        FD_ZERO(&rwFdSet);
        FD_SET(fd, &rwFdSet);

        if(timeout >= 0)
        {
            if(write)
            {
                ret = ::select(static_cast<int>(fd + 1), 0, &rwFdSet, 0, &tv);
            }
            else
            {
                ret = ::select(static_cast<int>(fd + 1), &rwFdSet, 0, 0, &tv);
            }
        }
        else
        {
            if(write)
            {
                ret = ::select(static_cast<int>(fd + 1), 0, &rwFdSet, 0, 0);
            }
            else
            {
                ret = ::select(static_cast<int>(fd + 1), &rwFdSet, 0, 0, 0);
            }
        }
    }
    while(ret == SOCKET_ERROR && interrupted());

    if(ret == SOCKET_ERROR)
    {
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }

    if(ret == 0)
    {
        throw TimeoutException(__FILE__, __LINE__);
    }
}

void
IceSSL::SslTransceiver::readSelect(int timeout)
{
    select(timeout, false);
}

void
IceSSL::SslTransceiver::writeSelect(int timeout)
{
    select(timeout, true);
}

//
// Static Protected
//

void
IceSSL::SslTransceiver::addTransceiver(SSL* sslPtr, SslTransceiver* transceiver)
{
    assert(sslPtr);
    assert(transceiver);
    IceUtil::StaticMutex::Lock sync(_transceiverRepositoryMutex);
    _transceiverMap[sslPtr] = transceiver;
}

void
IceSSL::SslTransceiver::removeTransceiver(SSL* sslPtr)
{
    assert(sslPtr);
    IceUtil::StaticMutex::Lock sync(_transceiverRepositoryMutex);
    _transceiverMap.erase(sslPtr);
}

#ifdef ICE_SSL_EXTRA_TRACING

void
IceSSL::SslTransceiver::showCertificateChain(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

    STACK_OF(X509)* sk;

    // Big nasty buffer
    char buffer[4096];

    if((sk = SSL_get_peer_cert_chain(_sslConnection)) != 0)
    {
        BIO_printf(bio,"---\nCertificate chain\n");

        for(int i = 0; i < sk_X509_num(sk); i++)
        {
            X509_NAME_oneline(X509_get_subject_name(sk_X509_value(sk,i)), buffer, int(sizeof(buffer)));
            BIO_printf(bio, "%2d s:%s\n", i, buffer);

            X509_NAME_oneline(X509_get_issuer_name(sk_X509_value(sk,i)), buffer, int(sizeof(buffer)));
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
IceSSL::SslTransceiver::showPeerCertificate(BIO* bio, const char* connType)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

    X509* peerCert = 0;
    char buffer[4096];

    if((peerCert = SSL_get_peer_certificate(_sslConnection)) != 0)
    {
        BIO_printf(bio, "%s Certificate\n", connType);
        PEM_write_bio_X509(bio, peerCert);

        X509_NAME_oneline(X509_get_subject_name(peerCert), buffer, int(sizeof(buffer)));
        BIO_printf(bio, "subject=%s\n", buffer);

        X509_NAME_oneline(X509_get_issuer_name(peerCert), buffer, int(sizeof(buffer)));
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
IceSSL::SslTransceiver::showSharedCiphers(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

    char buffer[4096];
    char* strPointer = 0;

    if((strPointer = SSL_get_shared_ciphers(_sslConnection, buffer, int(sizeof(buffer)))) != 0)
    {
        // This works only for SSL 2.  In later protocol versions, the client does not know
        // what other ciphers (in addition to the one to be used in the current connection)
        // the server supports.

        BIO_printf(bio, "---\nShared Ciphers:\n");

        int j = 0;
        int i = 0;

        while(*strPointer)
        {
            if(*strPointer == ':')
            {
                BIO_write(bio, "                ", (15-j%25));
                i++;
                j=0;
                BIO_write(bio, ((i%3)?" ":"\n"), 1);
            }
            else
            {
                BIO_write(bio, strPointer, 1);
                j++;
            }

            strPointer++;
        }

        BIO_write(bio,"\n",1);
    }
}

void
IceSSL::SslTransceiver::showSessionInfo(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

    if(_sslConnection->hit)
    {
        BIO_printf(bio, "Reused session-id\n");
    }

    PEM_write_bio_SSL_SESSION(bio, SSL_get_session(_sslConnection));
}

void
IceSSL::SslTransceiver::showSelectedCipherInfo(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

    const char* str;
    SSL_CIPHER* cipher;

    // Show the cipher that was finally selected.
    cipher = SSL_get_current_cipher(_sslConnection);

    str = SSL_CIPHER_get_name(cipher);
    BIO_printf(bio, "Cipher Name: %s\n", ((str != 0) ? str : "(NONE)"));

    str = SSL_CIPHER_get_version(cipher);
    BIO_printf(bio, "Cipher Version:    %s\n", ((str != 0) ? str : "(NONE)"));
}

void
IceSSL::SslTransceiver::showHandshakeStats(BIO* bio)
{
    assert(_sslConnection != 0);
    assert(bio != 0);

    BIO_printf(bio, "---\nSSL handshake has read %ld bytes and written %ld bytes\n",
            BIO_number_read(SSL_get_rbio(_sslConnection)),
            BIO_number_written(SSL_get_wbio(_sslConnection)));
}

void
IceSSL::SslTransceiver::showClientCAList(BIO* bio, const char* connType)
{
    assert(_sslConnection != 0);
    assert(bio != 0);
    assert(connType != 0);

    char buffer[4096];
    STACK_OF(X509_NAME)* sk = SSL_get_client_CA_list(_sslConnection);

    if((sk != 0) && (sk_X509_NAME_num(sk) > 0))
    {
        BIO_printf(bio,"---\nAcceptable %s certificate CA names\n", connType);

        for(int i = 0; i < sk_X509_NAME_num(sk); i++)
        {
            X509_NAME_oneline(sk_X509_NAME_value(sk, i), buffer, int(sizeof(buffer)));
            BIO_write(bio, buffer, int(strlen(buffer)));
            BIO_write(bio,"\n", 1);
        }
    }
    else
    {
        BIO_printf(bio,"---\nNo %s certificate CA names sent\n", connType);
    }
}

#endif

//
// Private Methods
//

IceSSL::SslTransceiver::SslTransceiver(ContextType contextType,
				       const OpenSSLPluginIPtr& plugin,
                                       SOCKET fd,
		                       const CertificateVerifierPtr& certificateVerifier,
                                       SSL* sslConnection,
				       int timeout) :
    _contextType(contextType),
    _plugin(plugin),
    _fd(fd),
    _certificateVerifier(certificateVerifier),
    _sslConnection(sslConnection),
    _logger(plugin->getLogger()),
    _traceLevels(plugin->getTraceLevels()),
    _stats(plugin->getStats())
{
    assert(sslConnection != 0);

    SSL_set_ex_data(sslConnection, 0, static_cast<void*>(plugin.get()));

    // Set the Connect Connection state for this connection.
    if(contextType == IceSSL::Client)
    {
	SSL_set_connect_state(_sslConnection);
    }
    else
    {
	SSL_set_accept_state(_sslConnection);
    }

    // Set up the SSL to be able to refer back to our connection object.
    addTransceiver(_sslConnection, this);
}

IceSSL::SslTransceiver::~SslTransceiver()
{
    assert(_sslConnection != 0);

    removeTransceiver(_sslConnection);
    SSL_set_ex_data(_sslConnection, 0, 0);
    SSL_free(_sslConnection);
    _sslConnection = 0;
}
