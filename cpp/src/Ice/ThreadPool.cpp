// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ThreadPool.h>
#include <Ice/EventHandler.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Functional.h>
#include <Ice/Protocol.h>
#include <Ice/ObjectAdapterFactory.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ThreadPool* p) { p->__incRef(); }
void IceInternal::decRef(ThreadPool* p) { p->__decRef(); }

void
IceInternal::ThreadPool::_register(SOCKET fd, const EventHandlerPtr& handler)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if (handler->server())
    {
	++_servers;
    }
    _adds.push_back(make_pair(fd, handler));
    setInterrupt();
}

void
IceInternal::ThreadPool::unregister(SOCKET fd, bool callFinished)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _removes.push_back(make_pair(fd, callFinished));
    setInterrupt();
}

void
IceInternal::ThreadPool::promoteFollower()
{
    _threadMutex.unlock();
}

void
IceInternal::ThreadPool::initiateServerShutdown()
{
    char c = 1;
#ifdef WIN32
    ::send(_fdIntrWrite, &c, 1, 0);
#else
    ::write(_fdIntrWrite, &c, 1);
#endif
}

void
IceInternal::ThreadPool::waitUntilServerFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    while (_servers != 0 && _threadNum != 0)
    {
	wait();
    }

    if (_servers != 0)
    {
	Error out(_instance->logger());
	out << "can't wait for graceful server termination in thread pool\n"
	    << "since all threads have vanished";
    }
}

void
IceInternal::ThreadPool::waitUntilFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    while (!_handlerMap.empty() && _threadNum != 0)
    {
	wait();
    }

    if (!_handlerMap.empty())
    {
	Error out(_instance->logger());
	out << "can't wait for graceful application termination in thread pool\n"
	    << "since all threads have vanished";
    }
}

void
IceInternal::ThreadPool::joinWithAllThreads()
{
    //
    // _threads is immutable after the initial creation in the
    // constructor, therefore no synchronization is
    // needed. (Synchronization wouldn't be possible here anyway,
    // because otherwise the other threads would never terminate.)
    //
    for (vector<IceUtil::ThreadControl>::iterator p = _threads.begin(); p != _threads.end(); ++p)
    {
	(*p).join();
    }
}

void
IceInternal::ThreadPool::setMaxConnections(int maxConnections)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if (maxConnections < _threadNum + 1 && maxConnections != 0)
    {
	_maxConnections = _threadNum + 1;
    }
    else
    {
	_maxConnections = maxConnections;
    }
}

int
IceInternal::ThreadPool::getMaxConnections()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _maxConnections;
}

IceInternal::ThreadPool::ThreadPool(const InstancePtr& instance) :
    _instance(instance),
    _destroyed(false),
    _lastFd(INVALID_SOCKET),
    _servers(0),
    _timeout(0)
{
    SOCKET fds[2];
    createPipe(fds);
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    setBlock(_fdIntrRead, false);

    FD_ZERO(&_fdSet);
    FD_SET(_fdIntrRead, &_fdSet);
    _maxFd = _fdIntrRead;
    _minFd = _fdIntrRead;

    _timeout = atoi(_instance->properties()->getProperty("Ice.ServerIdleTime").c_str());
    _threadNum = atoi(_instance->properties()->getPropertyWithDefault("Ice.ThreadPool.Size", "10").c_str());
    if (_threadNum < 1)
    {
	_threadNum = 1;
    }

    try
    {
	for (int i = 0 ; i < _threadNum ; ++i)
	{
	    IceUtil::ThreadPtr thread = new EventHandlerThread(this);
	    _threads.push_back(thread->start());
	}
    }
    catch (const IceUtil::Exception&)
    {
	//
	// TODO: This doesn't look correct to me -- where are the
	// started threads joined with in the event of a failure?
	//
	destroy();
	throw;
    }

    // Must be called after _threadNum is set.
    setMaxConnections(atoi(_instance->properties()->getProperty("Ice.ThreadPool.MaxConnections").c_str()));
}

IceInternal::ThreadPool::~ThreadPool()
{
    assert(_destroyed);
    closeSocket(_fdIntrWrite);
    closeSocket(_fdIntrRead);
}

void
IceInternal::ThreadPool::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    _destroyed = true;
    setInterrupt();
}

bool
IceInternal::ThreadPool::clearInterrupt()
{
    bool shutdown = false;
    char c;
#ifdef WIN32
    while (::recv(_fdIntrRead, &c, 1, 0) == 1)
#else
    while (::read(_fdIntrRead, &c, 1) == 1)
#endif
    {
	if (c == 1) // Shutdown initiated?
	{
	    shutdown = true;
	}
    }

    return shutdown;
}

void
IceInternal::ThreadPool::setInterrupt()
{
    char c = 0;
#ifdef WIN32
    ::send(_fdIntrWrite, &c, 1, 0);
#else
    ::write(_fdIntrWrite, &c, 1);
#endif
}

void
IceInternal::ThreadPool::run()
{
    bool shutdown = false;

    while (true)
    {
	_threadMutex.lock();
	
    repeatSelect:
	
	if (shutdown) // Shutdown has been initiated.
	{
	    shutdown = false;
	    _instance->objectAdapterFactory()->shutdown();
	}
	
	fd_set fdSet;
	memcpy(&fdSet, &_fdSet, sizeof(fd_set));
	int ret;
	if (_timeout)
	{
	    struct timeval tv;
	    tv.tv_sec = _timeout;
	    tv.tv_usec = 0;
	    ret = ::select(_maxFd + 1, &fdSet, 0, 0, &tv);
	}
	else
	{
	    ret = ::select(_maxFd + 1, &fdSet, 0, 0, 0);
	}
	
	if (ret == 0) // Timeout.
	{
	    assert(_timeout);
	    _timeout = 0;
	    shutdown = true;
	    goto repeatSelect;
	}
	
	if (ret == SOCKET_ERROR)
	{
	    if (interrupted())
	    {
		goto repeatSelect;
	    }
	    
	    SocketException ex(__FILE__, __LINE__);
	    ex.error = getSocketErrno();
	    throw ex;
	}
	
	EventHandlerPtr handler;

	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    
	    if (_destroyed)
	    {
		//
		// Don't clear the interrupt fd if destroyed, so that
		// the other threads exit as well.
		//
		return;
	    }

	    if (!_adds.empty())
	    {
		//
		// New handlers have been added.
		//
		for (vector<pair<SOCKET, EventHandlerPtr> >::iterator p = _adds.begin(); p != _adds.end(); ++p)
		{
		    _handlerMap.insert(*p);
		    FD_SET(p->first, &_fdSet);
		    _maxFd = max(_maxFd, p->first);
		    _minFd = min(_minFd, p->first);
		}
		_adds.clear();
	    }
	    
	    if (!_removes.empty())
	    {
		//
		// Handlers are permanently removed.
		//
		for (vector<pair<SOCKET, bool> >::iterator p = _removes.begin(); p != _removes.end(); ++p)
		{
		    map<SOCKET, EventHandlerPtr>::iterator q = _handlerMap.find(p->first);
		    assert(q != _handlerMap.end());
		    FD_CLR(p->first, &_fdSet);
		    if (p->second) // Call finished() on the handler?
		    {
			q->second->finished();
		    }
		    if (q->second->server())
		    {
			--_servers;
		    }
		    _handlerMap.erase(q);
		}
		_removes.clear();
		_maxFd = _fdIntrRead;
		_minFd = _fdIntrRead;
		if (!_handlerMap.empty())
		{
		    _maxFd = max(_maxFd, (--_handlerMap.end())->first);
		    _minFd = min(_minFd, _handlerMap.begin()->first);
		}
		if (_handlerMap.empty() || _servers == 0)
		{
		    notifyAll(); // For waitUntil...Finished() methods.
		}

		//
                // Selected filedescriptors may have changed, I
                // therefore need to repeat the select().
		//
		shutdown = clearInterrupt();
		goto repeatSelect;
	    }
	
//
// Optimization for WIN32 specific version of fd_set. Looping with a
// FD_ISSET test like for Unix is very unefficient for WIN32.
//
#ifdef WIN32
	    //
	    // Round robin for the filedescriptors.
	    //
	    if (fdSet.fd_count == 0)
	    {
		Error out(_instance->logger());
		out << "select() in thread pool returned " << ret << " but no filedescriptor is readable";
		goto repeatSelect;
	    }

	    SOCKET largerFd = _maxFd + 1;
	    SOCKET smallestFd = _maxFd + 1;
	    for (u_short i = 0; i < fdSet.fd_count; ++i)
	    {
		SOCKET fd = fdSet.fd_array[i];
		assert(fd != INVALID_SOCKET);

		if (fd > _lastFd || _lastFd == INVALID_SOCKET)
		{
		    largerFd = min(largerFd, fd);
		}

		smallestFd = min(smallestFd, fd);
	    }

	    if (largerFd <= _maxFd)
	    {
		assert(largerFd >= _minFd);
		_lastFd = largerFd;
	    }
	    else
	    {
		assert(smallestFd >= _minFd && smallestFd <= _maxFd);
		_lastFd = smallestFd;
	    }
#else
	    //
	    // Round robin for the filedescriptors.
	    //
	    if (_lastFd < _minFd - 1 || _lastFd == INVALID_SOCKET)
	    {
		_lastFd = _minFd - 1;
	    }

	    int loops = 0;
	    do
	    {
		if (++_lastFd > _maxFd)
		{
		    ++loops;
		    _lastFd = _minFd;
		}
	    }
	    while (!FD_ISSET(_lastFd, &fdSet) && loops <= 1);

	    if (loops > 1)
	    {
		Error out(_instance->logger());
		out << "select() in thread pool returned " << ret << " but no filedescriptor is readable";
		goto repeatSelect;
	    }
#endif
	    
	    if (_lastFd == _fdIntrRead)
	    {
		shutdown = clearInterrupt();
		goto repeatSelect;
	    }

	    map<SOCKET, EventHandlerPtr>::iterator p = _handlerMap.find(_lastFd);
	    if(p == _handlerMap.end())
	    {
		Error out(_instance->logger());
		out << "filedescriptor " << _lastFd << " not registered with the thread pool";
		goto repeatSelect;
	    }
	    
	    handler = p->second;
	}

	//
	// If the handler is "readable", try to read a message.
	//
	BasicStream stream(_instance);
	if (handler->readable())
	{
	    try
	    {
		read(handler);
	    }
	    catch (const TimeoutException&) // Expected.
	    {
		goto repeatSelect;
	    }
	    catch (const LocalException& ex)
	    {
		handler->exception(ex);
		goto repeatSelect;
	    }
	    
	    stream.swap(handler->_stream);
	    assert(stream.i == stream.b.end());
	}
	
	handler->message(stream);
    }
}

void
IceInternal::ThreadPool::read(const EventHandlerPtr& handler)
{
    BasicStream& stream = handler->_stream;
    
    if (stream.b.size() == 0)
    {
	stream.b.resize(headerSize);
	stream.i = stream.b.begin();
    }

    if (stream.i != stream.b.end())
    {
	handler->read(stream);
	assert(stream.i == stream.b.end());
    }
    
    int pos = stream.i - stream.b.begin();
    assert(pos >= headerSize);
    stream.i = stream.b.begin();
    Byte protVer;
    stream.read(protVer);
    if (protVer != protocolVersion)
    {
	throw UnsupportedProtocolException(__FILE__, __LINE__);
    }
    Byte encVer;
    stream.read(encVer);
    if (encVer != encodingVersion)
    {
	throw UnsupportedEncodingException(__FILE__, __LINE__);
    }
    Byte messageType;
    stream.read(messageType);
    Int size;
    stream.read(size);
    if (size < headerSize)
    {
	throw IllegalMessageSizeException(__FILE__, __LINE__);
    }
    if (size > 1024 * 1024) // TODO: configurable
    {
	throw MemoryLimitException(__FILE__, __LINE__);
    }
    if (size > static_cast<Int>(stream.b.size()))
    {
	stream.b.resize(size);
    }
    stream.i = stream.b.begin() + pos;
    
    if (stream.i != stream.b.end())
    {
	handler->read(stream);
	assert(stream.i == stream.b.end());
    }
}

IceInternal::ThreadPool::EventHandlerThread::EventHandlerThread(const ThreadPoolPtr& pool) :
    _pool(pool)
{
}

void
IceInternal::ThreadPool::EventHandlerThread::run()
{
    try
    {
	_pool->run();
    }
    catch (const Exception& ex)
    {	
	Error out(_pool->_instance->logger());
	out << "exception in thread pool:\n" << ex;
    }
    catch (...)
    {
	Error out(_pool->_instance->logger());
	out << "unknown exception in thread pool";
    }

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*_pool.get());
	--_pool->_threadNum;
	assert(_pool->_threadNum >= 0);

	//
	// The notifyAll() shouldn't be needed, *except* if one of the
	// threads exits because of an exception. (Which is an error
	// condition in Ice and if it happens needs to be debugged.)
	// However, I call notifyAll() anyway, in all cases, using a
	// "defensive" programming approach when it comes to
	// multithreading.
	//
	if (_pool->_threadNum == 0)
	{
	    _pool->notifyAll(); // For waitUntil...Finished() methods.
	}
    }

    _pool->promoteFollower();
    _pool = 0; // Break cyclic dependency.
}
