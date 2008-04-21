// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

    {
        IceUtil::Mutex::Lock sync(_sslMutex);
        int err = SSL_shutdown(_ssl);
        
        //
        // Call it one more time if it returned 0.
        //
        if(err == 0)
        {
            SSL_shutdown(_ssl);
        }
    }

    //
    // Only one thread calls close(), so synchronization is not necessary here.
    //
    SSL_free(_ssl);
    _ssl = 0;

    assert(_fd != INVALID_SOCKET);
    _fd = INVALID_SOCKET;
}

bool
IceSSL::TransceiverI::write(IceInternal::Buffer& buf)
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
        int ret, err;
        bool wantWrite;
        {
            IceUtil::Mutex::Lock sync(_sslMutex);
            ret = SSL_write(_ssl, reinterpret_cast<const void*>(&*buf.i), packetSize);
            err = SSL_get_error(_ssl, ret);
            wantWrite = SSL_want_write(_ssl);
        }

        if(ret <= 0)
        {
            switch(err)
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
                assert(false);
                break;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                return false;
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
                        if(wantWrite)
                        {
                            return false;
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

    return true;
}

bool
IceSSL::TransceiverI::read(IceInternal::Buffer& buf)
{
    // It's impossible for the packetSize to be more than an Int.
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    
    while(buf.i != buf.b.end())
    {
        ERR_clear_error(); // Clear any spurious errors.
        assert(_fd != INVALID_SOCKET);
        int ret, err;
        bool wantRead;
        {
            IceUtil::Mutex::Lock sync(_sslMutex);
            ret = SSL_read(_ssl, reinterpret_cast<void*>(&*buf.i), packetSize);
            err = SSL_get_error(_ssl, ret);
            wantRead = SSL_want_read(_ssl);
        }

        if(ret <= 0)
        {
            switch(err)
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
                return false;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                assert(false);
                break;
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
                        if(wantRead)
                        {
                            return false;
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
                const char* estr = ERR_GET_LIB(e) == ERR_LIB_SSL ? ERR_reason_error_string(e) : 0;
                if(estr && strcmp(estr, "decryption failed or bad record mac") == 0)
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

    return true;
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

IceInternal::SocketStatus
IceSSL::TransceiverI::initialize()
{
    try
    {
        if(_state == StateNeedConnect)
        {
            _state = StateConnectPending;
            return IceInternal::NeedConnect; 
        }
        else if(_state <= StateConnectPending)
        {
            IceInternal::doFinishConnect(_fd);
            _state = StateConnected;
            _desc = IceInternal::fdToString(_fd);
        }
        assert(_state == StateConnected);

        do
        {
            //
            // Only one thread calls initialize(), so synchronization is not necessary here.
            //
            int ret = _incoming ? SSL_accept(_ssl) : SSL_connect(_ssl);
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
                return IceInternal::NeedRead;
            }
            case SSL_ERROR_WANT_WRITE:
            {
                return IceInternal::NeedWrite;
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
                            return IceInternal::NeedRead;
                        }
                        else if(SSL_want_write(_ssl))
                        {
                            return IceInternal::NeedWrite;
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
                struct sockaddr_storage remoteAddr;
                string desc = "<not available>";
                if(IceInternal::fdToRemoteAddress(_fd, remoteAddr))
                {
                    desc = IceInternal::addrToString(remoteAddr);
                }
                ostringstream ostr;
                ostr << "SSL error occurred for new " << (_incoming ? "incoming" : "outgoing")
                     << " connection:\nremote address = " << desc << "\n" << _instance->sslErrors();
                ProtocolException ex(__FILE__, __LINE__);
                ex.reason = ostr.str();
                throw ex;
            }
            }
        }
        while(!SSL_is_init_finished(_ssl));
    
        _instance->verifyPeer(_ssl, _fd, "", _adapterName, _incoming);
    }
    catch(const Ice::LocalException& ex)
    {
        if(_instance->networkTraceLevel() >= 2)
        {
            Trace out(_logger, _instance->networkTraceCategory());
            out << "failed to establish ssl connection\n" << _desc << "\n" << ex;
        }
        throw;
    }

    if(_instance->networkTraceLevel() >= 1)
    {
        Trace out(_logger, _instance->networkTraceCategory());
        if(_incoming)
        {
            out << "accepted ssl connection\n" << _desc;
        }
        else
        {
            out << "ssl connection established\n" << _desc;
        }
    }

    if(_instance->securityTraceLevel() >= 1)
    {
        _instance->traceConnection(_ssl, _incoming);
    }

    return IceInternal::Finished;
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
    // This can only be called on an open transceiver.
    //
    assert(_fd != INVALID_SOCKET);
    return populateConnectionInfo(_ssl, _fd, _adapterName, _incoming);
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance, SSL* ssl, SOCKET fd, bool connected,
                                   bool incoming, const string& adapterName) :
    _instance(instance),
    _logger(instance->communicator()->getLogger()),
    _stats(instance->communicator()->getStats()),
    _ssl(ssl),
    _fd(fd),
    _adapterName(adapterName),
    _incoming(incoming),
    _state(connected ? StateConnected : StateNeedConnect),
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

