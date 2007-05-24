// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Transceiver.h>
#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Buffer.h>
#include <IceE/Network.h>
#include <IceE/LocalException.h>
#include <IceE/SafeStdio.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(Transceiver* p) { return p; }

void
IceInternal::Transceiver::setTimeouts(int readTimeout, int writeTimeout)
{
    _readTimeout = readTimeout;
#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS    
    setTimeout(_fd, true, _readTimeout);
#endif

    _writeTimeout = writeTimeout;
#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS    
    setTimeout(_fd, false, _writeTimeout);
#endif
}

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

#ifdef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
#ifdef _WIN32
    assert(_event != 0);
    WSACloseEvent(_event);
    WSACloseEvent(_readEvent);
    WSACloseEvent(_writeEvent);
    _event = 0;
    _readEvent = 0;
    _writeEvent = 0;
#endif
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
IceInternal::Transceiver::writeWithTimeout(Buffer& buf, int timeout)
{
    Buffer::Container::difference_type packetSize = 
        static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);
    
#ifdef _WIN32
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if(packetSize > _maxPacketSize)
    {
	packetSize = _maxPacketSize;
    }
#endif

#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
    if(timeout > 0 && timeout != _writeTimeout)
    {
	setTimeout(_fd, false, timeout);
    }

    try
    {	
#endif
	while(buf.i != buf.b.end())
	{
	repeatSend:
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
		if(interrupted())
		{
		    goto repeatSend;
		}

		if(noBuffers() && packetSize > 1024)
		{
		    packetSize /= 2;
		    goto repeatSend;
		}
	    
#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
		if(timedout())
		{
		    throw TimeoutException(__FILE__, __LINE__);
		}
#else
		if(wouldBlock())
		{
		    doSelect(false, timeout > 0 ? timeout : _writeTimeout);
 		    continue;
		}
#endif

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
#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
    }
    catch(const Ice::LocalException&)
    {
	if(timeout > 0 && timeout != _writeTimeout)
	{
	    try
	    {
		setTimeout(_fd, false, _writeTimeout);
	    }
	    catch(const Ice::LocalException&)
	    {
		// IGNORE
	    }
	}
	throw;
    }

    if(timeout > 0 && timeout != _writeTimeout)
    {
	try
	{
	    setTimeout(_fd, false, _writeTimeout);
	}
	catch(const Ice::LocalException&)
	{
	    // IGNORE
	}
    }
#endif
}

void
IceInternal::Transceiver::readWithTimeout(Buffer& buf, int timeout)
{
    assert(timeout != 0);

    Buffer::Container::difference_type packetSize = 
	static_cast<Buffer::Container::difference_type>(buf.b.end() - buf.i);

#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS    
    if(timeout > 0 && timeout != _readTimeout)
    {
	setTimeout(_fd, true, timeout);
    }
    try
    {
#endif
	while(buf.i != buf.b.end())
	{
	repeatRead:	
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
		if(interrupted())
		{
		    goto repeatRead;
		}
		
		if(noBuffers() && packetSize > 1024)
		{
		    packetSize /= 2;
		    goto repeatRead;
		}
		
#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
		if(timedout())
		{
		    throw TimeoutException(__FILE__, __LINE__);
		}
#else
		if(wouldBlock())
		{
		    doSelect(true, timeout > 0 ? timeout : _readTimeout);
		    continue;
		}
#endif
	    
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
#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
    }
    catch(const Ice::LocalException&)
    {
	if(timeout > 0 && timeout != _readTimeout)
	{
	    try
	    {
		setTimeout(_fd, true, _readTimeout);
	    }
	    catch(const Ice::LocalException&)
	    {
		// IGNORE
	    }
	}
	throw;
    }

    if(timeout > 0 && timeout != _readTimeout)
    {
	try
	{
	    setTimeout(_fd, true, _readTimeout);
	}
	catch(const Ice::LocalException&)
	{
	    // IGNORE
	}
    }
#endif
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
    _logger(instance->initializationData().logger),
    _fd(fd),
    _readTimeout(-1),
    _writeTimeout(-1),
    _desc(fdToString(fd))
{
#ifdef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
#ifdef _WIN32
    _event = WSACreateEvent();
    _readEvent = WSACreateEvent();
    _writeEvent = WSACreateEvent();
    if(_event == 0 || _readEvent == 0 || _writeEvent == 0)
    {
	int error = WSAGetLastError();
	if(_event != 0)
	{
	    WSACloseEvent(_event);
	}
	if(_readEvent != 0)
	{
	    WSACloseEvent(_readEvent);
	}
	if(_writeEvent != 0)
	{
	    WSACloseEvent(_writeEvent);
	}
    	closeSocket(_fd);

	SocketException ex(__FILE__, __LINE__);
	ex.error = error;
	throw ex;
    }

    //
    // Select the READ, WRITE and CLOSE for trigging.
    //
    if(WSAEventSelect(_fd, _event, FD_READ|FD_WRITE|FD_CLOSE) == SOCKET_ERROR)
    {
	int error = WSAGetLastError();

    	WSACloseEvent(_event);
    	WSACloseEvent(_readEvent);
    	WSACloseEvent(_writeEvent);
    	closeSocket(_fd);

	SocketException ex(__FILE__, __LINE__);
	ex.error = error;
	throw ex;
    }
#else
    FD_ZERO(&_wFdSet);
    FD_ZERO(&_rFdSet);
#endif
#endif

#ifdef _WIN32
    //
    // On Windows, limiting the buffer size is important to prevent
    // poor throughput performances when transfering large amount of
    // data. See Microsoft KB article KB823764.
    //
    _maxPacketSize = getSendBufferSize(_fd) / 2;
    if(_maxPacketSize < 512)
    {
        _maxPacketSize = 512; // Make sure the packet size limiter isn't too small.
    }
#endif
}

IceInternal::Transceiver::~Transceiver()
{
    assert(_fd == INVALID_SOCKET);
#ifdef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
#ifdef _WIN32
    assert(_event == 0);
    assert(_readEvent == 0);
    assert(_writeEvent == 0);
#endif
#endif
}

#ifdef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
void
IceInternal::Transceiver::doSelect(bool read, int timeout)
{
    while(true)
    {
#ifdef _WIN32
	//
	// This code is basically the same as the code in
	// ::send above. Check that for detailed comments.
	//
	WSAEVENT events[2];
	events[0] = _event;
	events[1] = read ? _readEvent : _writeEvent;
	long tout = (timeout >= 0) ? timeout : WSA_INFINITE;
	DWORD rc = WSAWaitForMultipleEvents(2, events, FALSE, tout, FALSE);
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
	    
	if(rc == WSA_WAIT_EVENT_0)
	{
	    WSANETWORKEVENTS nevents;
	    if(WSAEnumNetworkEvents(_fd, _event, &nevents) == SOCKET_ERROR)
	    {
		SocketException ex(__FILE__, __LINE__);
		ex.error = WSAGetLastError();
		throw ex;
	    }
		
	    //
	    // If we're selecting for reading and have consumed a WRITE
	    // event, set the _writeEvent event. Otherwise, if we're 
	    // selecting for writing have consumed a READ event, set the
	    // _readEvent event.
	    //
	    if(read && nevents.lNetworkEvents & FD_WRITE)
	    {
		WSASetEvent(_writeEvent);
	    }
	    else if(!read && nevents.lNetworkEvents & FD_READ)
	    {
		WSASetEvent(_readEvent);
	    }

		
	    //
	    // This checks for an error on the fd (this would
	    // be same as recv itself returning an error). In
	    // the event of an error we set the error code,
	    // and repeat the error handling.
	    //
	    if(read && nevents.lNetworkEvents & FD_READ && nevents.iErrorCode[FD_READ_BIT] != 0)
	    {
		WSASetLastError(nevents.iErrorCode[FD_READ_BIT]);
	    }
	    else if(!read && nevents.lNetworkEvents & FD_WRITE && nevents.iErrorCode[FD_WRITE_BIT] != 0)
	    {
		WSASetLastError(nevents.iErrorCode[FD_WRITE_BIT]);
	    }
	    else if(nevents.lNetworkEvents & FD_CLOSE && nevents.iErrorCode[FD_CLOSE_BIT] != 0)
	    {
		WSASetLastError(nevents.iErrorCode[FD_CLOSE_BIT]);
	    }
	    else
	    {
		return; // No errors: we're done.
	    }
	
	    if(interrupted())
	    {
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
	else
	{
	    //
	    // Otherwise the _readEvent or _writeEvent is set, reset it.
	    //
	    if(read)
	    {
		WSAResetEvent(_readEvent);
	    }
	    else 
	    {
		WSAResetEvent(_writeEvent);
	    }
	    return;
	}
#else
	int rs;
	assert(_fd != INVALID_SOCKET);
	if(read)
	{
	    FD_SET(_fd, &_rFdSet);
	}
	else
	{
	    FD_SET(_fd, &_wFdSet);
	}
	    
        struct pollfd pollFd[1];
        pollFd[0].fd = _fd;
        pollFd[0].events = read ? POLLIN : POLLOUT;
        rs = ::poll(pollFd, 1, timeout);
	if(rs == SOCKET_ERROR)
	{
	    if(interrupted())
	    {
		continue;
	    }
		
	    SocketException ex(__FILE__, __LINE__);
	    ex.error = getSocketErrno();
	    throw ex;
	}
	    
	if(rs == 0)
	{
	    throw TimeoutException(__FILE__, __LINE__);
	}
	
	return;
#endif
    }
}
#endif
