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
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/Logger.h>
#include <Ice/Functional.h>
#include <sstream>

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
IceInternal::ThreadPool::waitUntilServerFinished()
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);

    while (_servers > 0)
    {
	try
	{
	    wait();
	}
	catch (const JTCInterruptedException&)
	{
	}
    }
}

void
IceInternal::ThreadPool::waitUntilFinished()
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);

    while (_handlers.size() > 0)
    {
	try
	{
	    wait();
	}
	catch (const JTCInterruptedException&)
	{
	}
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

IceInternal::ThreadPool::ThreadPool(const InstancePtr& instance) :
    _instance(instance),
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

    try
    {
	int threadNum = 10;
	string value = _instance->properties()->getProperty("Ice.ThreadPool.Size");
	if (!value.empty())
	{
	    threadNum = atoi(value.c_str());
	    if (threadNum < 1)
	    {
		threadNum = 1;
	    }
	}

	for (int i = 0 ; i < threadNum ; ++i)
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

    _timeout = atoi(_instance->properties()->getProperty("Ice.ServerIdleTime").c_str());
}

IceInternal::ThreadPool::~ThreadPool()
{
    assert(!_instance);

    closeSocket(_fdIntrWrite);
    closeSocket(_fdIntrRead);
}

void
IceInternal::ThreadPool::destroy()
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
    _instance = 0;
    setInterrupt();
}

void
IceInternal::ThreadPool::clearInterrupt()
{
    char s[32]; // Clear up to 32 interrupts at once
#ifdef WIN32
    while (::recv(_fdIntrRead, s, 32, 0) == 32)
	;
#else
    while (::read(_fdIntrRead, s, 32) == 32)
	;
#endif
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
    while (true)
    {
	_threadMutex.lock();
	
	EventHandlerPtr handler;
	InstancePtr instance;
	
    repeatSelect:
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

	if (ret == 0) // Timeout
	{
	    assert(_timeout);
	    _timeout = 0;
	    _instance->communicator()->shutdown();
	    goto repeatSelect;
	}
	
	if (ret == SOCKET_ERROR)
	{
	    if (interrupted())
	    {
		goto repeatSelect;
	    }
	    
	    _threadMutex.unlock();
	    throw SocketException(__FILE__, __LINE__);
	}

	{
	    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);

	    instance = _instance;
	    
	    if (!instance) // Destroyed?
	    {
		//
		// Don't clear the interrupt fd if destroyed, so that
		// the other threads exit as well
		//
		_threadMutex.unlock();
		return;
	    }
		
	    if (FD_ISSET(_fdIntrRead, &fdSet))
	    {
		clearInterrupt();
#ifdef WIN32
		FD_CLR(static_cast<u_int>(_fdIntrRead), &fdSet);
#else
		FD_CLR(_fdIntrRead, &fdSet);
#endif
	    }
	    
	    bool again = false;
	    
	    if (!_adds.empty())
	    {
		//
		// New handlers have been added
		//
		for (vector<pair<int, EventHandlerPtr> >::iterator p = _adds.begin(); p != _adds.end(); ++p)
		{
		    _handlers.insert(*p);
		    FD_SET(p->first, &_fdSet);
		    _maxFd = max(_maxFd, p->first);
		}
		_adds.clear();
		again = true;
	    }
	    
	    if (!_removes.empty())
	    {
		//
		// Handlers are permanently removed
		//
		for (vector<int>::iterator p = _removes.begin(); p != _removes.end(); ++p)
		{
		    std::map<int, EventHandlerPtr>::iterator q = _handlers.find(*p);
		    assert(q != _handlers.end());
#ifdef WIN32
		    FD_CLR(static_cast<u_int>(*p), &_fdSet);
#else
		    FD_CLR(*p, &_fdSet);
#endif
		    q->second->finished();
		    if (q->second->server())
		    {
			--_servers;
		    }
		    _handlers.erase(q);
		}
		_removes.clear();
		_maxFd = _fdIntrRead;
		if (!_handlers.empty())
		{
		    _maxFd = max(_maxFd, (--_handlers.end())->first);
		}
		again = true;
		if (_handlers.empty() || _servers == 0)
		{
		    notifyAll(); // For waitUntil...Finished() methods
		}
	    }
	    
	    if (again)
	    {
		goto repeatSelect;
	    }
	    
	    //
	    // Round robin for the filedescriptors
	    //
	    do
	    {
		if (++_lastFd > _maxFd)
		{
		    _lastFd = 0;
		}
	    }
	    while (!FD_ISSET(_lastFd, &fdSet));
	    
	    std::map<int, EventHandlerPtr>::iterator p = _handlers.find(_lastFd);
	    assert(p != _handlers.end());
	    handler = p->second;
	}
	
	//
	// If the handler is "readable", try to read a message
	//
	Stream stream(instance);
	if (handler->readable())
	{
	    try
	    {
		read(handler);
	    }
	    catch (const TimeoutException&) // Expected
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
    Stream& stream = handler->_stream;
    
    if (stream.b.size() < 8) // Read header?
    {
	if (stream.b.size() == 0)
	{
	    stream.b.resize(8);
	    stream.i = stream.b.begin();
	}
	
	handler->read(stream);
	if (stream.i != stream.b.end())
	{
	    return;
	}
    }
    
    if (stream.b.size() >= 8) // Interpret header?
    {
	int pos = stream.i - stream.b.begin();
	stream.i = stream.b.begin();
	bool peerBigendian;
	stream.read(peerBigendian);
	stream.pushBigendian(peerBigendian);
	Byte protVer;
	stream.read(protVer);
	if (protVer != 0)
	{
	    throw UnsupportedProtocolException(__FILE__, __LINE__);
	}
	Byte encVer;
	stream.read(encVer);
	if (encVer != 0)
	{
	    throw UnsupportedEncodingException(__FILE__, __LINE__);
	}
	Byte messageType;
	stream.read(messageType);
	Int size;
	stream.read(size);
	if (size > 1024 * 1024) // TODO: configurable
	{
	    throw ::Ice::MemoryLimitException(__FILE__, __LINE__);
	}
	stream.b.resize(size);
	stream.i = stream.b.begin() + pos;
    }
    
    if (stream.b.size() > 8 && stream.i != stream.b.end())
    {
	handler->read(stream);
    }
}

void
IceInternal::ThreadPool::EventHandlerThread::run()
{
    try
    {
	_pool->run();
    }
    catch (const LocalException& ex)
    {
	ostringstream s;
	s << "exception in thread pool:\n" << ex;
	_pool->_instance->logger()->error(s.str());
    }
    catch (const JTCException& ex)
    {
	ostringstream s;
	s << "exception in thread pool:\n" << ex;
	_pool->_instance->logger()->error(s.str());
    }
    catch (...)
    {
	_pool->_instance->logger()->error("unknown exception in thread pool");
    }

    _pool = 0; // Break cyclic dependency
}
