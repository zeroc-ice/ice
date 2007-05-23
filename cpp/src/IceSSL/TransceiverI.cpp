// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/TransceiverI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/Util.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Stats.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>

#include <openssl/err.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

SOCKET
IceSSL::TransceiverI::fd()
{
    assert(_fd != INVALID_SOCKET);
    return _fd;
}

void
IceSSL::TransceiverI::close()
{
    if(_instance->networkTraceLevel() >= 1)
    {
        Trace out(_logger, _instance->networkTraceCategory());
        out << "closing ssl connection\n" << toString();
    }

    shutdown();

    assert(_fd != INVALID_SOCKET);
    SSL_free(_ssl);
    _ssl = 0;
    _fd = INVALID_SOCKET;
}

void
IceSSL::TransceiverI::shutdownWrite()
{
    if(_instance->networkTraceLevel() >= 2)
    {
        Trace out(_logger, _instance->networkTraceCategory());
        out << "shutting down ssl connection for writing\n" << toString();
    }

    shutdown();

    assert(_fd != INVALID_SOCKET);
    IceInternal::shutdownSocketWrite(_fd);
}

void
IceSSL::TransceiverI::shutdownReadWrite()
{
    if(_instance->networkTraceLevel() >= 2)
    {
        Trace out(_logger, _instance->networkTraceCategory());
        out << "shutting down ssl connection for reading and writing\n" << toString();
    }

    shutdown();

    assert(_fd != INVALID_SOCKET);
    IceInternal::shutdownSocketReadWrite(_fd);
}

void
IceSSL::TransceiverI::write(IceInternal::Buffer& buf, int timeout)
{
    // Its impossible for the packetSize to be more than an Int.
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    
#ifdef _WIN32
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if(packetSize > _maxPacketSize)
    { 
        packetSize = _maxPacketSize;
    }
#endif

    while(buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
        assert(_fd != INVALID_SOCKET);
        int ret = SSL_write(_ssl, reinterpret_cast<const void*>(&*buf.i), packetSize);

        if(ret <= 0)
        {
            switch(SSL_get_error(_ssl, ret))
            {
            case SSL_ERROR_NONE:
                assert(false);
                break;
            case SSL_ERROR_ZERO_RETURN:
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            case SSL_ERROR_WANT_READ:
            {
                if(!selectRead(_fd, timeout))
                {
                    throw TimeoutException(__FILE__, __LINE__);
                }
                continue;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                if(!selectWrite(_fd, timeout))
                {
                    throw TimeoutException(__FILE__, __LINE__);
                }
                continue;
            }
            case SSL_ERROR_SYSCALL:
            {
                if(ret == -1)
                {
                    if(IceInternal::interrupted())
                    {
                        continue;
                    }

                    if(IceInternal::noBuffers() && packetSize > 1024)
                    {
                        packetSize /= 2;
                        continue;
                    }

                    if(IceInternal::wouldBlock())
                    {
                        if(SSL_want_read(_ssl))
                        {
                            if(!selectRead(_fd, timeout))
                            {
                                throw TimeoutException(__FILE__, __LINE__);
                            }
                        }
                        else if(SSL_want_write(_ssl))
                        {
                            if(!selectWrite(_fd, timeout))
                            {
                                throw TimeoutException(__FILE__, __LINE__);
                            }
                        }

                        continue;
                    }

                    if(IceInternal::connectionLost())
                    {
                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = IceInternal::getSocketErrno();
                        throw ex;
                    }
                }

                if(ret == 0)
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }

                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            case SSL_ERROR_SSL:
            {
                ProtocolException ex(__FILE__, __LINE__);
                ex.reason = "SSL protocol error during write:\n" + _instance->sslErrors();
                throw ex;
            }
            }
        }

        if(_instance->networkTraceLevel() >= 3)
        {
            Trace out(_logger, _instance->networkTraceCategory());
            out << "sent " << ret << " of " << packetSize << " bytes via ssl\n" << toString();
        }

        if(_stats)
        {
            _stats->bytesSent(type(), static_cast<Int>(ret));
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }
}

void
IceSSL::TransceiverI::read(IceInternal::Buffer& buf, int timeout)
{
    // Its impossible for the packetSize to be more than an Int.
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    
    while(buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
        assert(_fd != INVALID_SOCKET);
        int ret = SSL_read(_ssl, reinterpret_cast<void*>(&*buf.i), packetSize);

        if(ret <= 0)
        {
            switch(SSL_get_error(_ssl, ret))
            {
            case SSL_ERROR_NONE:
                assert(false);
                break;
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
                //shutdownSocketReadWrite(_fd);

                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = 0;
                throw ex;
            }
            case SSL_ERROR_WANT_READ:
            {
                if(!selectRead(_fd, timeout))
                {
                    throw TimeoutException(__FILE__, __LINE__);
                }
                continue;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                if(!selectWrite(_fd, timeout))
                {
                    throw TimeoutException(__FILE__, __LINE__);
                }
                continue;
            }
            case SSL_ERROR_SYSCALL:
            {
                if(ret == -1)
                {
                    if(IceInternal::interrupted())
                    {
                        continue;
                    }

                    if(IceInternal::noBuffers() && packetSize > 1024)
                    {
                        packetSize /= 2;
                        continue;
                    }

                    if(IceInternal::wouldBlock())
                    {
                        if(SSL_want_read(_ssl))
                        {
                            if(!selectRead(_fd, timeout))
                            {
                                throw TimeoutException(__FILE__, __LINE__);
                            }
                        }
                        else if(SSL_want_write(_ssl))
                        {
                            if(!selectWrite(_fd, timeout))
                            {
                                throw TimeoutException(__FILE__, __LINE__);
                            }
                        }

                        continue;
                    }

                    if(IceInternal::connectionLost())
                    {
                        //
                        // See the commment above about shutting down the
                        // socket if the connection is lost while reading
                        // data.
                        //
                        //assert(_fd != INVALID_SOCKET);
                        //shutdownSocketReadWrite(_fd);

                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = IceInternal::getSocketErrno();
                        throw ex;
                    }
                }

                if(ret == 0)
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }

                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            case SSL_ERROR_SSL:
            {
                //
                // Forcefully closing a connection can result in SSL_read reporting
                // "decryption failed or bad record mac". We trap that error and
                // treat it as the loss of a connection.
                //
                // NOTE: We have to compare the reason string instead
                // of the error codes because the error code values
                // changed between OpenSSL 0.9.7i and 0.9.7j and
                // between OpenSSL 0.9.8a and 0.9.8b...
                //
                //unsigned long e = ERR_peek_error();
                //if(ERR_GET_LIB(e) == ERR_LIB_SSL && ERR_GET_REASON(e) == SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC)
                //
                unsigned long e = ERR_peek_error();
                if(ERR_GET_LIB(e) == ERR_LIB_SSL && 
                   strcmp(ERR_reason_error_string(e), "decryption failed or bad record mac") == 0)
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }
                else
                {
                    ProtocolException ex(__FILE__, __LINE__);
                    ex.reason = "SSL protocol error during read:\n" + _instance->sslErrors();
                    throw ex;
                }
            }
            }
        }

        if(_instance->networkTraceLevel() >= 3)
        {
            Trace out(_logger, _instance->networkTraceCategory());
            out << "received " << ret << " of " << packetSize << " bytes via ssl\n" << toString();
        }

        if(_stats)
        {
            _stats->bytesReceived(type(), static_cast<Int>(ret));
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }
}

string
IceSSL::TransceiverI::type() const
{
    return "ssl";
}

string
IceSSL::TransceiverI::toString() const
{
    return _desc;
}

void
IceSSL::TransceiverI::initialize(int timeout)
{
    if(_incoming)
    {
        // TODO: The timeout is 0 when called by the thread pool.
        // Make this configurable?
        if(timeout == 0)
        {
            timeout = -1;
        }

        do
        {
            int ret = SSL_accept(_ssl);
            switch(SSL_get_error(_ssl, ret))
            {
            case SSL_ERROR_NONE:
                assert(SSL_is_init_finished(_ssl));
                break;
            case SSL_ERROR_ZERO_RETURN:
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            case SSL_ERROR_WANT_READ:
            {
                if(!selectRead(_fd, timeout))
                {
                    throw ConnectTimeoutException(__FILE__, __LINE__);
                }
                break;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                if(!selectWrite(_fd, timeout))
                {
                    throw ConnectTimeoutException(__FILE__, __LINE__);
                }
                break;
            }
            case SSL_ERROR_SYSCALL:
            {
                if(ret == -1)
                {
                    if(IceInternal::interrupted())
                    {
                        break;
                    }

                    if(IceInternal::wouldBlock())
                    {
                        if(SSL_want_read(_ssl))
                        {
                            if(!selectRead(_fd, timeout))
                            {
                                throw ConnectTimeoutException(__FILE__, __LINE__);
                            }
                        }
                        else if(SSL_want_write(_ssl))
                        {
                            if(!selectWrite(_fd, timeout))
                            {
                                throw ConnectTimeoutException(__FILE__, __LINE__);
                            }
                        }

                        break;
                    }

                    if(IceInternal::connectionLost())
                    {
                        ConnectionLostException ex(__FILE__, __LINE__);
                        ex.error = IceInternal::getSocketErrno();
                        throw ex;
                    }
                }

                if(ret == 0)
                {
                    ConnectionLostException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }

                SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            case SSL_ERROR_SSL:
            {
                struct sockaddr_in remoteAddr;
                string desc;
                if(IceInternal::fdToRemoteAddress(_fd, remoteAddr))
                {
                    desc = IceInternal::addrToString(remoteAddr);
                }
                ProtocolException ex(__FILE__, __LINE__);
                ex.reason = "SSL error occurred for new incoming connection:\nremote address = " + desc + "\n" +
                    _instance->sslErrors();
                throw ex;
            }
            }
        }
        while(!SSL_is_init_finished(_ssl));

        _instance->verifyPeer(_ssl, _fd, "", _adapterName, true);

        if(_instance->networkTraceLevel() >= 1)
        {
            Trace out(_logger, _instance->networkTraceCategory());
            out << "accepted ssl connection\n" << IceInternal::fdToString(_fd);
        }

        if(_instance->securityTraceLevel() >= 1)
        {
            _instance->traceConnection(_ssl, true);
        }
    }
}

void
IceSSL::TransceiverI::checkSendSize(const IceInternal::Buffer& buf, size_t messageSizeMax)
{
    if(buf.b.size() > messageSizeMax)
    {
        throw MemoryLimitException(__FILE__, __LINE__);
    }
}

ConnectionInfo
IceSSL::TransceiverI::getConnectionInfo() const
{
    //
    // This can only be called on a open transceiver.
    //
    assert(_fd != INVALID_SOCKET);
    return populateConnectionInfo(_ssl, _fd, _adapterName, _incoming);
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance, SSL* ssl, SOCKET fd,
                                   bool incoming, const string& adapterName) :
    _instance(instance),
    _logger(instance->communicator()->getLogger()),
    _stats(instance->communicator()->getStats()),
    _ssl(ssl),
    _fd(fd),
    _adapterName(adapterName),
    _incoming(incoming),
    _desc(IceInternal::fdToString(fd))
{
#ifdef _WIN32
    //
    // On Windows, limiting the buffer size is important to prevent
    // poor throughput performances when transfering large amount of
    // data. See Microsoft KB article KB823764.
    //
    _maxPacketSize = IceInternal::getSendBufferSize(_fd) / 2;
    if(_maxPacketSize < 512)
    {
        _maxPacketSize = 0;
    }
#endif
}

IceSSL::TransceiverI::~TransceiverI()
{
    assert(_fd == INVALID_SOCKET);
}

void
IceSSL::TransceiverI::shutdown()
{
    int err = SSL_shutdown(_ssl);

    //
    // Call it one more time if it returned 0.
    //
    if(err == 0)
    {
        SSL_shutdown(_ssl);
    }
}
