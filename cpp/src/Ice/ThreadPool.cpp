// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/ThreadPool.h>
#include <Ice/EventHandler.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Functional.h>
#include <Ice/Protocol.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ThreadPool* p) { p->__incRef(); }
void IceInternal::decRef(ThreadPool* p) { p->__decRef(); }

IceInternal::ThreadPool::ThreadPool(const InstancePtr& instance, const string& prefix, int timeout) :
    _instance(instance),
    _destroyed(false),
    _prefix(prefix),
    _lastFd(INVALID_SOCKET),
    _timeout(timeout),
    _size(0),
    _sizeMax(0),
    _sizeWarn(0),
    _running(0),
    _inUse(0),
    _load(0)
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

    int size = _instance->properties()->getPropertyAsIntWithDefault(_prefix + ".Size", 5);
    if(size < 1)
    {
	size = 1;
    }
    const_cast<int&>(_size) = size;
    
    int sizeMax = _instance->properties()->getPropertyAsIntWithDefault(_prefix + ".SizeMax", _size * 10);
    if(sizeMax < _size)
    {
	sizeMax = _size;
    }
    const_cast<int&>(_sizeMax) = sizeMax;

    int sizeWarn = _instance->properties()->getPropertyAsIntWithDefault(_prefix + ".SizeWarn", _sizeMax * 80 / 100);
    const_cast<int&>(_sizeWarn) = sizeWarn;

    __setNoDelete(true);
    try
    {
	for(int i = 0 ; i < _size ; ++i)
	{
	    IceUtil::ThreadPtr thread = new EventHandlerThread(this);
	    _threads.push_back(thread->start());
	    ++_running;
	}
    }
    catch(const IceUtil::Exception& ex)
    {
	{
	    Error out(_instance->logger());
	    out << "cannot create thread for `" << _prefix << "':\n" << ex;
	}

	destroy();
	joinWithAllThreads();
	__setNoDelete(false);
	throw;
    }
    catch(...)
    {
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
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
    IceUtil::Mutex::Lock sync(*this);
    assert(!_destroyed);
    assert(_handlerMap.empty());
    assert(_changes.empty());
    _destroyed = true;
    setInterrupt(0);
}

void
IceInternal::ThreadPool::_register(SOCKET fd, const EventHandlerPtr& handler)
{
    IceUtil::Mutex::Lock sync(*this);
    assert(!_destroyed);
    _changes.push_back(make_pair(fd, handler));
    setInterrupt(0);
}

void
IceInternal::ThreadPool::unregister(SOCKET fd)
{
    IceUtil::Mutex::Lock sync(*this);
    assert(!_destroyed);
    _changes.push_back(make_pair(fd, EventHandlerPtr(0)));
    setInterrupt(0);
}

void
IceInternal::ThreadPool::promoteFollower()
{
    if(_sizeMax > 1)
    {
	_promoteMutex.unlock();

	{
	    IceUtil::Mutex::Lock sync(*this);

	    if(!_destroyed)
	    {
		assert(_inUse >= 0);
		++_inUse;
		
		if(_inUse == _sizeWarn)
		{
		    Warning out(_instance->logger());
		    out << "thread pool `" << _prefix << "' is running low on threads\n"
			<< "Size=" << _size << ", " << "SizeMax=" << _sizeMax << ", " << "SizeWarn=" << _sizeWarn;
		}
		
		assert(_inUse <= _running);
		if(_inUse < _sizeMax && _inUse == _running)
		{
		    try
		    {
			IceUtil::ThreadPtr thread = new EventHandlerThread(this);
			_threads.push_back(thread->start());
			++_running;
		    }
		    catch(const IceUtil::Exception& ex)
		    {
			Error out(_instance->logger());
			out << "cannot create thread for `" << _prefix << "':\n" << ex;
		    }
		}
	    }
	}
    }
}

void
IceInternal::ThreadPool::initiateShutdown()
{
    //
    // This operation must be signal safe, so all we can do is to set
    // an interrupt.
    //
    setInterrupt(1);
}

void
IceInternal::ThreadPool::joinWithAllThreads()
{
    //
    // _threads is immutable after destroy() has been called,
    // therefore no synchronization is needed. (Synchronization
    // wouldn't be possible here anyway, because otherwise the other
    // threads would never terminate.)
    //
    assert(_destroyed);
#if defined(_MSC_VER) && _MSC_VER <= 1200 // The mem_fun_ref below does not work with VC++ 6.0
    for(vector<IceUtil::ThreadControl>::iterator p = _threads.begin(); p != _threads.end(); ++p)
    {
	p->join();
    }
#else
    for_each(_threads.begin(), _threads.end(), mem_fun_ref(&IceUtil::ThreadControl::join));
#endif
}

bool
IceInternal::ThreadPool::clearInterrupt()
{
    char c;

repeat:

#ifdef _WIN32
    if(::recv(_fdIntrRead, &c, 1, 0) == SOCKET_ERROR)
    {
	if(interrupted())
	{
	    goto repeat;
	}

	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
#else
    if(::read(_fdIntrRead, &c, 1) == -1)
    {
	if(interrupted())
	{
	    goto repeat;
	}

	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
#endif
    
    return c == 1; // Return true if shutdown has been initiated.
}

void
IceInternal::ThreadPool::setInterrupt(char c)
{
repeat:

#ifdef _WIN32
    if(::send(_fdIntrWrite, &c, 1, 0) == SOCKET_ERROR)
    {
	if(interrupted())
	{
	    goto repeat;
	}

	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
#else
    if(::write(_fdIntrWrite, &c, 1) == -1)
    {
	if(interrupted())
	{
	    goto repeat;
	}

	SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
#endif
}

bool
IceInternal::ThreadPool::run()
{
    ThreadPoolPtr self = this;

    if(_sizeMax > 1)
    {
	_promoteMutex.lock();	
    }

    while(true)
    {
	fd_set fdSet;
	memcpy(&fdSet, &_fdSet, sizeof(fd_set));
	int ret;
	if(_timeout > 0)
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
	
	if(ret == 0) // We initiate a shutdown if there is a thread pool timeout.
	{
	    assert(_timeout > 0);
	    _timeout = 0;
	    initiateShutdown();
	    continue;
	}
	
	if(ret == SOCKET_ERROR)
	{
	    if(interrupted())
	    {
		continue;
	    }
	    
	    SocketException ex(__FILE__, __LINE__);
	    ex.error = getSocketErrno();
	    throw ex;
	}
	
	EventHandlerPtr handler;
	bool finished = false;
	bool shutdown = false;
	
	{
	    IceUtil::Mutex::Lock sync(*this);

	    if(FD_ISSET(_fdIntrRead, &fdSet))
	    {
		//
		// There are three possiblities for an interrupt:
		//
		// - The thread pool has been destroyed.
		//
		// - Server shutdown has been initiated.
		//
		// - An event handler was registered or unregistered.
		//
		
		//
		// Thread pool destroyed?
		//
		if(_destroyed)
		{
		    //
		    // Don't clear the interrupt if destroyed, so that
		    // the other threads exit as well.
		    //
		    return true;
		}
		
		shutdown = clearInterrupt();

		if(!shutdown)
		{
		    //
		    // An event handler must have been registered or
		    // unregistered.
		    //
		    assert(!_changes.empty());
		    pair<SOCKET, EventHandlerPtr> change = _changes.front();
		    _changes.pop_front();
		    
		    if(change.second) // Addition if handler is set.
		    {
			_handlerMap.insert(change);
			FD_SET(change.first, &_fdSet);
			_maxFd = max(_maxFd, change.first);
			_minFd = min(_minFd, change.first);
			continue;
		    }
		    else // Removal if handler is not set.
		    {
			map<SOCKET, EventHandlerPtr>::iterator p = _handlerMap.find(change.first);
			assert(p != _handlerMap.end());
			handler = p->second;
			finished = true;
			_handlerMap.erase(p);
			FD_CLR(change.first, &_fdSet);
			_maxFd = _fdIntrRead;
			_minFd = _fdIntrRead;
			if(!_handlerMap.empty())
			{
			    _maxFd = max(_maxFd, (--_handlerMap.end())->first);
			    _minFd = min(_minFd, _handlerMap.begin()->first);
			}
			// Don't continue; we have to call
			// finished() on the event handler below, outside
			// the thread synchronization.
		    }
		}
	    }
	    else
	    {
//
// Optimization for WIN32 specific version of fd_set. Looping with a
// FD_ISSET test like for Unix is very inefficient for WIN32.
//
#ifdef _WIN32
		//
		// Round robin for the filedescriptors.
		//
		if(fdSet.fd_count == 0)
		{
		    Error out(_instance->logger());
		    out << "select() in `" << _prefix << "' returned " << ret << " but no filedescriptor is readable";
		    continue;
		}
		
		SOCKET largerFd = _maxFd + 1;
		SOCKET smallestFd = _maxFd + 1;
		for(u_short i = 0; i < fdSet.fd_count; ++i)
		{
		    SOCKET fd = fdSet.fd_array[i];
		    assert(fd != INVALID_SOCKET);
		    
		    if(fd > _lastFd || _lastFd == INVALID_SOCKET)
		    {
			largerFd = min(largerFd, fd);
		    }
		    
		    smallestFd = min(smallestFd, fd);
		}
		
		if(largerFd <= _maxFd)
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
		if(_lastFd < _minFd - 1 || _lastFd == INVALID_SOCKET)
		{
		    _lastFd = _minFd - 1;
		}
		
		int loops = 0;
		do
		{
		    if(++_lastFd > _maxFd)
		    {
			++loops;
			_lastFd = _minFd;
		    }
		}
		while(!FD_ISSET(_lastFd, &fdSet) && loops <= 1);
		
		if(loops > 1)
		{
		    Error out(_instance->logger());
		    out << "select() in `" << _prefix << "' returned " << ret << " but no filedescriptor is readable";
		    continue;
		}
#endif
		
		assert(_lastFd != _fdIntrRead);
		
		map<SOCKET, EventHandlerPtr>::iterator p = _handlerMap.find(_lastFd);
		if(p == _handlerMap.end())
		{
		    Error out(_instance->logger());
		    out << "filedescriptor " << _lastFd << " not registered with `" << _prefix << "'";
		    continue;
		}
		
		handler = p->second;
	    }
	}
	
	assert(handler || shutdown);

	if(shutdown)
	{
	    //
	    // Initiate server shutdown.
	    //
	    ObjectAdapterFactoryPtr factory;
	    try
	    {
		factory = _instance->objectAdapterFactory();
	    }
	    catch(const Ice::CommunicatorDestroyedException&)
	    {
		continue;
	    }

	    promoteFollower();
	    factory->shutdown();
	}
	else
	{
	    assert(handler);
	    
	    if(finished)
	    {
		//
		// Notify a handler about it's removal from the thread
		// pool.
		//
		try
		{
		    handler->finished(self); // "self" is faster than "this", as the reference count is not modified.
		}
		catch(const LocalException& ex)
		{
		    Error out(_instance->logger());
		    out << "exception in `" << _prefix << "' while calling finished():\n"
			<< ex << '\n' << handler->toString();
		}
	    }
	    else
	    {
		//
		// If the handler is "readable", try to read a
		// message.
		//
		BasicStream stream(_instance.get());
		if(handler->readable())
		{
		    try
		    {
			read(handler);
		    }
		    catch(const TimeoutException&) // Expected.
		    {
			continue;
		    }
		    catch(const LocalException& ex)
		    {
			handler->exception(ex);
			continue;
		    }
		    
		    stream.swap(handler->_stream);
		    assert(stream.i == stream.b.end());
		}
	    
		//
		// "self" is faster than "this", as the reference
		// count is not modified.
		//
		handler->message(stream, self);
	    }
	}

	if(_sizeMax > 1)
	{
	    {
		IceUtil::Mutex::Lock sync(*this);
		
		if(!_destroyed)
		{
		    //
		    // First we reap threads that have been destroyed before.
		    //
		    int sz = static_cast<int>(_threads.size());
		    assert(_running <= sz);
		    if(_running < sz)
		    {
			vector<IceUtil::ThreadControl>::iterator start =
			    partition(_threads.begin(), _threads.end(), mem_fun_ref(&IceUtil::ThreadControl::isAlive));
#if defined(_MSC_VER) && _MSC_VER <= 1200 // The mem_fun_ref below does not work with VC++ 6.0
			for(vector<IceUtil::ThreadControl>::iterator p = start; p != _threads.end(); ++p)
			{
			    p->join();
			}
#else
			for_each(start, _threads.end(), mem_fun_ref(&IceUtil::ThreadControl::join));
#endif
			_threads.erase(start, _threads.end());
		    }
		    
		    //
		    // Now we check if this thread can be destroyed, based
		    // on a load factor.
		    //
		    const double loadFactor = 0.05; // TODO: Configurable?
		    const double oneMinusLoadFactor = 1 - loadFactor;
		    _load = _load * oneMinusLoadFactor + _inUse * loadFactor;

		    if(_running > _size)
		    {
			int load = static_cast<int>(_load + 1);
			if(load < _running)
			{
			    assert(_inUse > 0);
			    --_inUse;

			    assert(_running > 0);
			    --_running;

			    return false;
			}
		    }
			
		    assert(_inUse > 0);
		    --_inUse;
		}
	    }

	    _promoteMutex.lock();	
	}
    }
}

void
IceInternal::ThreadPool::read(const EventHandlerPtr& handler)
{
    BasicStream& stream = handler->_stream;
    
    if(stream.b.size() == 0)
    {
	stream.b.resize(headerSize);
	stream.i = stream.b.begin();
    }

    if(stream.i != stream.b.end())
    {
	handler->read(stream);
	assert(stream.i == stream.b.end());
    }
    
    ptrdiff_t pos = stream.i - stream.b.begin();
    assert(pos >= headerSize);
    stream.i = stream.b.begin();
    ByteSeq m(sizeof(magic), 0);
    stream.readBlob(m, static_cast<Int>(sizeof(magic)));
    if(!equal(m.begin(), m.end(), magic))
    {
	BadMagicException ex(__FILE__, __LINE__);
	ex.badMagic = m;
	throw ex;
    }
    Byte pMajor;
    Byte pMinor;
    stream.read(pMajor);
    stream.read(pMinor);
    if(pMajor != protocolMajor
       || static_cast<unsigned char>(pMinor) > static_cast<unsigned char>(protocolMinor))
    {
	UnsupportedProtocolException ex(__FILE__, __LINE__);
	ex.badMajor = static_cast<unsigned char>(pMajor);
	ex.badMinor = static_cast<unsigned char>(pMinor);
	ex.major = static_cast<unsigned char>(protocolMajor);
	ex.minor = static_cast<unsigned char>(protocolMinor);
	throw ex;
    }
    Byte eMajor;
    Byte eMinor;
    stream.read(eMajor);
    stream.read(eMinor);
    if(eMajor != encodingMajor
       || static_cast<unsigned char>(eMinor) > static_cast<unsigned char>(encodingMinor))
    {
	UnsupportedEncodingException ex(__FILE__, __LINE__);
	ex.badMajor = static_cast<unsigned char>(eMajor);
	ex.badMinor = static_cast<unsigned char>(eMinor);
	ex.major = static_cast<unsigned char>(encodingMajor);
	ex.minor = static_cast<unsigned char>(encodingMinor);
	throw ex;
    }
    Byte messageType;
    stream.read(messageType);
    Byte compress;
    stream.read(compress);
    Int size;
    stream.read(size);
    if(size < headerSize)
    {
	throw IllegalMessageSizeException(__FILE__, __LINE__);
    }
    if(size > 1024 * 1024) // TODO: configurable
    {
	throw MemoryLimitException(__FILE__, __LINE__);
    }
    if(size > static_cast<Int>(stream.b.size()))
    {
	stream.b.resize(size);
    }
    stream.i = stream.b.begin() + pos;
    
    if(stream.i != stream.b.end())
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
    bool promote;

    try
    {
	promote = _pool->run();
    }
    catch(const Exception& ex)
    {	
	Error out(_pool->_instance->logger());
	out << "exception in `" << _pool->_prefix << "':\n" << ex; 
	promote = true;
    }
    catch(...)
    {
	Error out(_pool->_instance->logger());
	out << "unknown exception in `" << _pool->_prefix << "'"; 
	promote = true;
   }

    if(promote && _pool->_sizeMax > 1)
    {
	//
	// Promote a follower, but w/o modifying _inUse or creating
	// new threads.
	//
	_pool->_promoteMutex.unlock();
    }

    _pool = 0; // Break cyclic dependency.
}
