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
#include <Ice/Logger.h>
#include <Ice/Functional.h>
#include <Ice/Protocol.h>
#include <Ice/ObjectAdapterFactory.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ThreadPool* p) { p->__incRef(); }
void IceInternal::decRef(ThreadPool* p) { p->__decRef(); }

void
IceInternal::ThreadPool::_register(int fd, const EventHandlerPtr& handler)
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
    if (handler->server())
    {
	++_servers;
    }
    _adds.push_back(make_pair(fd, handler));
    setInterrupt();
}

void
IceInternal::ThreadPool::unregister(int fd)
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
    _removes.push_back(fd);
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
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);

    while (_servers > 0 && _threadNum > 0)
    {
	try
	{
	    wait();
	}
	catch (const JTCInterruptedException&)
	{
	}
    }

    if (_servers > 0)
    {
	_instance->logger()->error("can't wait for graceful server termination in thread pool\n"
				   "since all threads have vanished");
    }
}

void
IceInternal::ThreadPool::waitUntilFinished()
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
    
    while (_handlerMap.size() > 0 && _threadNum > 0) // Faster than _reapList.size() with most STLs.
    {
	try
	{
	    wait();
	}
	catch (const JTCInterruptedException&)
	{
	}
    }

    if (_handlerMap.size() > 0)
    {
	_instance->logger()->error("can't wait for graceful application termination in thread pool\n"
				   "since all threads have vanished");
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
    for (vector<JTCThreadHandle>::iterator p = _threads.begin(); p != _threads.end(); ++p)
    {
	(*p)->join();
    }
}

void
IceInternal::ThreadPool::setMaxConnections(int maxConnections)
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
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
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
    return _maxConnections;
}

IceInternal::ThreadPool::ThreadPool(const InstancePtr& instance) :
    _instance(instance),
    _destroyed(false),
    _lastFd(-1),
    _servers(0),
    _timeout(0)
{
    int fds[2];
    createPipe(fds);
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    setBlock(_fdIntrRead, false);

    FD_ZERO(&_fdSet);
    FD_SET(_fdIntrRead, &_fdSet);
    _maxFd = _fdIntrRead;
    _minFd = _fdIntrRead;

    _timeout = atoi(_instance->properties()->getProperty("Ice.ServerIdleTime").c_str());

    try
    {
	_threadNum = 10;
	string value = _instance->properties()->getProperty("Ice.ThreadPool.Size");
	if (!value.empty())
	{
	    _threadNum = atoi(value.c_str());
	    if (_threadNum < 1)
	    {
		_threadNum = 1;
	    }
	}

	for (int i = 0 ; i < _threadNum ; ++i)
	{
	    JTCThreadHandle thread = new EventHandlerThread(this);
	    thread->start();
	    _threads.push_back(thread);
	}
    }
    catch (const JTCException&)
    {
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
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
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
	
	EventHandlerPtr handler;
	bool reap;
	
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
	
	{
	    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
	    
	    if (_destroyed)
	    {
		//
		// Don't clear the interrupt fd if destroyed, so that
		// the other threads exit as well.
		//
		return;
	    }
	    
	    bool interrupt = false;
	    if (FD_ISSET(_fdIntrRead, &fdSet))
	    {
		shutdown = clearInterrupt();
		interrupt = true;
	    }
	    
	    if (!_adds.empty())
	    {
		//
		// New handlers have been added.
		//
		for (vector<pair<int, EventHandlerPtr> >::iterator p = _adds.begin(); p != _adds.end(); ++p)
		{
		    _reapList.push_front(p->first);
		    _handlerMap[p->first] = make_pair(p->second, _reapList.begin());
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
		for (vector<int>::iterator p = _removes.begin(); p != _removes.end(); ++p)
		{
		    map<int, pair<EventHandlerPtr, list<int>::iterator> >::iterator q =	_handlerMap.find(*p);
		    assert(q != _handlerMap.end());
#ifdef WIN32
		    FD_CLR(static_cast<u_int>(*p), &_fdSet);
#else
		    FD_CLR(*p, &_fdSet);
#endif
		    q->second.first->finished();
		    if (q->second.first->server())
		    {
			--_servers;
		    }

		    _reapList.erase(q->second.second);
		    _handlerMap.erase(q);
		}
		_removes.clear();
		_maxFd = _fdIntrRead;
		_minFd = _fdIntrRead;
		if (!_handlerMap.empty())
		{
		    _maxFd = max(_maxFd, (--_handlerMap.end())->first);
		    _minFd = min(_minFd, (--_handlerMap.end())->first);
		}
		if (_handlerMap.empty() || _servers == 0)
		{
		    notifyAll(); // For waitUntil...Finished() methods.
		}
	    }
	    
	    if (interrupt)
	    {
		goto repeatSelect;
	    }

	    //
	    // Check if there are connections to reap.
	    //
	    reap = false;
            // _handlerMap.size() is faster than _reapList() with most STLs.
	    if (_maxConnections > 0 && _handlerMap.size() > static_cast<list<int>::size_type>(_maxConnections))
	    {
		for (list<int>::reverse_iterator p = _reapList.rbegin(); p != _reapList.rend(); ++p)
		{
		    int fd = *p;
		    if (fd != -1)
		    {
			_reapList.pop_back();
			_reapList.push_front(-1);
			map<int, pair<EventHandlerPtr, list<int>::iterator> >::iterator q = _handlerMap.find(fd);
			q->second.second = _reapList.begin();
			handler = q->second.first;
			reap = true;
			break;
		    }
		}
	    }

	    if (!reap)
	    {
		//
		// Round robin for the filedescriptors.
		//
		if (_lastFd < _minFd - 1)
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
		    ostringstream s;
		    s << "select() in thread pool returned " << ret << " but no filedescriptor is readable";
		    _instance->logger()->error(s.str());
		    goto repeatSelect;
		}
		
		map<int, pair<EventHandlerPtr, list<int>::iterator> >::iterator p = _handlerMap.find(_lastFd);
		if(p == _handlerMap.end())
		{
		    ostringstream s;
		    s << "filedescriptor " << _lastFd << " not registered with the thread pool";
		    _instance->logger()->error(s.str());
		    goto repeatSelect;
		}
		
		//
		// Make the fd for the handler the most recently used one
		// by moving it to the beginning of the the reap list.
		//
		if (p->second.second != _reapList.begin())
		{
		    _reapList.erase(p->second.second);
		    _reapList.push_front(p->first);
		    p->second.second = _reapList.begin();
		}
		
		handler = p->second.first;
	    }
	}

	if (reap)
	{
	    //
	    // Reap the handler.
	    //
	    try
	    {
		if (!handler->tryDestroy())
		{
		    goto repeatSelect;
		}
	    }
	    catch (const LocalException&)
	    {
		// Ignore exeptions.
	    }
	}
	else
	{
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
}

void
IceInternal::ThreadPool::read(const EventHandlerPtr& handler)
{
    BasicStream& stream = handler->_stream;
    
    if (stream.b.size() < static_cast<BasicStream::Container::size_type>(headerSize)) // Read header?
    {
	if (stream.b.size() == 0)
	{
	    stream.b.resize(headerSize);
	    stream.i = stream.b.begin();
	}
	
	handler->read(stream);
	if (stream.i != stream.b.end())
	{
	    return;
	}
    }
    
    if (stream.b.size() >= static_cast<BasicStream::Container::size_type>(headerSize)) // Interpret header?
    {
	int pos = stream.i - stream.b.begin();
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
	if (size > 1024 * 1024) // TODO: configurable
	{
	    throw MemoryLimitException(__FILE__, __LINE__);
	}
	stream.b.resize(size);
	stream.i = stream.b.begin() + pos;
    }
    
    if (stream.b.size() > static_cast<BasicStream::Container::size_type>(headerSize) && stream.i != stream.b.end())
    {
	handler->read(stream);
    }
}

void
IceInternal::ThreadPool::reapConnections()
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
	ostringstream s;
	s << "exception in thread pool:\n" << ex;
	_pool->_instance->logger()->error(s.str());
    }
    catch (const JTCException& ex)
    {
	ostringstream s;
	s << "JThreads/C++ exception in thread pool:\n" << ex;
	_pool->_instance->logger()->error(s.str());
    }
    catch (...)
    {
	_pool->_instance->logger()->error("unknown exception in thread pool");
    }

    {
	JTCSyncT<JTCMonitorT<JTCMutex> > sync(*_pool.get());
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
