// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Transceiver.h>
#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Buffer.h>
#include <IceE/Network.h>
#include <IceE/LocalExceptions.h>
#include <IceE/SafeStdio.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Transceiver* p) { p->__incRef(); }
void IceInternal::decRef(Transceiver* p) { p->__decRef(); }


SOCKET
IceInternal::Transceiver::fd()
{
    assert(_fd != INVALID_SOCKET);
    return _fd;
}

void
IceInternal::Transceiver::close()
{
    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "closing tcp connection\n" << toString();
    }

#ifdef _WIN32
    assert(_event != 0);
    WSACloseEvent(_event);
    _event = 0;
#endif

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
IceInternal::Transceiver::shutdownWrite()
{
    if(_traceLevels->network >= 2)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "shutting down tcp connection for writing\n" << toString();
    }

    assert(_fd != INVALID_SOCKET);
    shutdownSocketWrite(_fd);
}

void
IceInternal::Transceiver::shutdownReadWrite()
{
    if(_traceLevels->network >= 2)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "shutting down tcp connection for reading and writing\n" << toString();
    }

    assert(_fd != INVALID_SOCKET);
    shutdownSocketReadWrite(_fd);
}

void
IceInternal::Transceiver::write(Buffer& buf, int timeout)
{
    Buffer::Container::difference_type packetSize = 
        static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);
    
#ifdef _WIN32
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if(_isPeerLocal && packetSize > 64 * 1024)
    {
	packetSize = 64 * 1024;
    }
#endif

    while(buf.i != buf.b.end())
    {
	assert(_fd != INVALID_SOCKET);
	ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&*buf.i), packetSize, 0);

	if(ret == 0)
	{
	    ConnectionLostException ex(__FILE__, __LINE__);
	    ex.error = 0;
	    throw ex;
	}

	if(ret == SOCKET_ERROR)
	{
#ifdef _WIN32
	repeatError:
#endif
	if(interrupted())
	    {
		continue;
	    }

	    if(noBuffers() && packetSize > 1024)
	    {
		packetSize /= 2;
		continue;
	    }

	    if(wouldBlock())
	    {
#ifdef _WIN32
		WSAEVENT events[1];
		events[0] = _event;
    	    	long tout = (timeout >= 0) ? timeout : WSA_INFINITE;
		DWORD rc = WSAWaitForMultipleEvents(1, events, FALSE, tout, FALSE);
		if(rc == WSA_WAIT_FAILED)
		{
		    //
		    // This an error from WSAWaitForMultipleEvents
		    // itself (similar to an error from select). None
		    // of these errors are recoverable (such as
		    // EINTR).
		    //
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = WSAGetLastError();
		    throw ex;
		}

		if(rc == WSA_WAIT_TIMEOUT)
		{
    	    	    assert(timeout >= 0);
		    throw TimeoutException(__FILE__, __LINE__);
		}
		assert(rc == WSA_WAIT_EVENT_0);

		WSANETWORKEVENTS nevents;
		if(WSAEnumNetworkEvents(_fd, _event, &nevents) == SOCKET_ERROR)
		{
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = WSAGetLastError();
		    throw ex;
		}
		//
    	    	// This checks for an error on the fd (this would be
    	    	// same as recv itself returning an error).
		//
		// In the event of an error we set the error code, and
		// // repeat the error handling.
		//
		if(nevents.lNetworkEvents & FD_READ && nevents.iErrorCode[FD_READ_BIT] != 0)
		{
		    WSASetLastError(nevents.iErrorCode[FD_READ_BIT]);
		    goto repeatError;
		}
		if(nevents.lNetworkEvents & FD_CLOSE && nevents.iErrorCode[FD_CLOSE_BIT] != 0)
		{
		    WSASetLastError(nevents.iErrorCode[FD_CLOSE_BIT]);
		    goto repeatError;
		}
#else
	    repeatSelect:
		int rs;
		assert(_fd != INVALID_SOCKET);
		FD_SET(_fd, &_wFdSet);

		if(timeout >= 0)
		{
		    struct timeval tv;
		    tv.tv_sec = timeout / 1000;
		    tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
		    rs = ::select(_fd + 1, 0, &_wFdSet, 0, &tv);
		}
		else
		{
		    rs = ::select(_fd + 1, 0, &_wFdSet, 0, 0);
		}
		
		if(rs == SOCKET_ERROR)
		{
		    if(interrupted())
		    {
			goto repeatSelect;
		    }
		    
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = getSocketErrno();
		    throw ex;
		}
		
		if(rs == 0)
		{
		    throw TimeoutException(__FILE__, __LINE__);
		}
#endif
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

	if(_traceLevels->network >= 3)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << Ice::printfToString("sent %d of %d", ret, packetSize) << " bytes via tcp\n" << toString();
	}

	buf.i += ret;

	if(packetSize > buf.b.end() - buf.i)
	{
	    packetSize = static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);
	}
    }
}

void
IceInternal::Transceiver::read(Buffer& buf, int timeout)
{
    Buffer::Container::difference_type packetSize = 
        static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);
    
    while(buf.i != buf.b.end())
    {
	assert(_fd != INVALID_SOCKET);
	ssize_t ret = ::recv(_fd, reinterpret_cast<char*>(&*buf.i), packetSize, 0);

	if(ret == 0)
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

	if(ret == SOCKET_ERROR)
	{
#ifdef _WIN32
	repeatError:
#endif
	    if(interrupted())
	    {
		continue;
	    }
	    
	    if(noBuffers() && packetSize > 1024)
	    {
		packetSize /= 2;
		continue;
	    }

	    if(wouldBlock())
	    {
#ifdef _WIN32
    	    	//
		// This code is basically the same as the code in
		// ::send above. Check that for detailed comments.
		//
		WSAEVENT events[1];
		events[0] = _event;
    	    	long tout = (timeout >= 0) ? timeout : WSA_INFINITE;
		DWORD rc = WSAWaitForMultipleEvents(1, events, FALSE, tout, FALSE);
		if(rc == WSA_WAIT_FAILED)
		{
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = WSAGetLastError();
		    throw ex;
		}
		if(rc == WSA_WAIT_TIMEOUT)
		{
    	    	    assert(timeout >= 0);
		    throw TimeoutException(__FILE__, __LINE__);
		}
		assert(rc == WSA_WAIT_EVENT_0);

		WSANETWORKEVENTS nevents;
		if(WSAEnumNetworkEvents(_fd, _event, &nevents) == SOCKET_ERROR)
		{
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = WSAGetLastError();
		    throw ex;
		}

		if(nevents.lNetworkEvents & FD_READ && nevents.iErrorCode[FD_READ_BIT] != 0)
		{
		    WSASetLastError(nevents.iErrorCode[FD_READ_BIT]);
		    goto repeatError;
		}
		if(nevents.lNetworkEvents & FD_CLOSE && nevents.iErrorCode[FD_CLOSE_BIT] != 0)
		{
		    WSASetLastError(nevents.iErrorCode[FD_CLOSE_BIT]);
		    goto repeatError;
		}
#else
	    repeatSelect:

		int rs;
		assert(_fd != INVALID_SOCKET);
		FD_SET(_fd, &_rFdSet);

		if(timeout >= 0)
		{
		    struct timeval tv;
		    tv.tv_sec = timeout / 1000;
		    tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
		    rs = ::select(_fd + 1, &_rFdSet, 0, 0, &tv);
		}
		else
		{
		    rs = ::select(_fd + 1, &_rFdSet, 0, 0, 0);
		}
		
		if(rs == SOCKET_ERROR)
		{
		    if(interrupted())
		    {
			goto repeatSelect;
		    }
		    
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = getSocketErrno();
		    throw ex;
		}
		
		if(rs == 0)
		{
		    throw TimeoutException(__FILE__, __LINE__);
		}
#endif
		continue;
	    }
	    
	    if(connectionLost())
	    {
		//
		// See the commment above about shutting down the
		// socket if the connection is lost while reading
		// data.
		//
		//assert(_fd != INVALID_SOCKET);
		//shutdownSocketReadWrite(_fd);

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

	if(_traceLevels->network >= 3)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << Ice::printfToString("received %d of %d", ret, packetSize) << " bytes via tcp\n" << toString();
	}

	buf.i += ret;

	if(packetSize > buf.b.end() - buf.i)
	{
	    packetSize = static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);
	}
    }
}

string
IceInternal::Transceiver::type() const
{
    return "tcp";
}

string
IceInternal::Transceiver::toString() const
{
    return _desc;
}

IceInternal::Transceiver::Transceiver(const InstancePtr& instance, SOCKET fd) :
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _fd(fd),
    _desc(fdToString(fd))
#ifdef _WIN32
    , _isPeerLocal(isPeerLocal(fd))
#endif
{
#ifdef _WIN32
    _event = WSACreateEvent();
    if(_event == 0)
    {
    	closeSocket(_fd);

	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }

    //
    // Create a WSAEVENT which selects read/write and close for
    // trigging.
    //
    if(WSAEventSelect(_fd, _event, FD_READ|FD_WRITE|FD_CLOSE) == SOCKET_ERROR)
    {
	int error = WSAGetLastError();

    	WSACloseEvent(_event);
    	closeSocket(_fd);

	SocketException ex(__FILE__, __LINE__);
	ex.error = error;
	throw ex;
    }
#else
    FD_ZERO(&_wFdSet);
    FD_ZERO(&_rFdSet);
#endif
}

IceInternal::Transceiver::~Transceiver()
{
    assert(_fd == INVALID_SOCKET);
#ifdef _WIN32
    assert(_event == 0);
#endif
}
