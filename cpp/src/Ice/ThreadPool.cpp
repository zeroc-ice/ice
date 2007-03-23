// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
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

IceUtil::Shared* IceInternal::upCast(ThreadPool* p) { return p; }

IceInternal::ThreadPool::ThreadPool(const InstancePtr& instance, const string& prefix, int timeout) :
    _instance(instance),
    _destroyed(false),
    _prefix(prefix),
    _lastFd(INVALID_SOCKET),
    _timeout(timeout),
    _size(0),
    _sizeMax(0),
    _sizeWarn(0),
    _stackSize(0),
    _running(0),
    _inUse(0),
    _load(1.0),
    _promote(true),
    _warnUdp(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Datagrams") > 0)
{
    SOCKET fds[2];
    createPipe(fds);
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    setBlock(_fdIntrRead, false);
    _maxFd = _fdIntrRead;
    _minFd = _fdIntrRead;

#if defined(_WIN32)
    _fdsInUse = 1; // _fdIntrRead is always in use.
    FD_ZERO(&_fdSet);
    FD_SET(_fdIntrRead, &_fdSet);
#elif defined(ICE_USE_EPOLL)
    _epollFd = epoll_create(1);
    if(_epollFd < 0)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    _events.resize(1);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = _fdIntrRead;
    if(epoll_ctl(_epollFd, EPOLL_CTL_ADD, _fdIntrRead, &event) != 0)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
#elif defined(__APPLE__)
    _kqueueFd = kqueue();
    if(_kqueueFd < 0)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    _events.resize(1);
    struct kevent event;
    EV_SET(&event, _fdIntrRead, EVFILT_READ, EV_ADD, 0, 0, 0);
    if(kevent(_kqueueFd, &event, 1, 0, 0, 0) < 0)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
#else
    _pollFdSet.resize(1);
    _pollFdSet[0].fd = _fdIntrRead;
    _pollFdSet[0].events = POLLIN;
#endif

    //
    // We use just one thread as the default. This is the fastest
    // possible setting, still allows one level of nesting, and
    // doesn't require to make the servants thread safe.
    //
    int size = _instance->initializationData().properties->getPropertyAsIntWithDefault(_prefix + ".Size", 1);
    if(size < 1)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".Size < 1; Size adjusted to 1";
        size = 1;
    }
    
    int sizeMax = 
        _instance->initializationData().properties->getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
    if(sizeMax < size)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".SizeMax < " << _prefix << ".Size; SizeMax adjusted to Size (" << size << ")";
        sizeMax = size;
    }           
    
    int sizeWarn = _instance->initializationData().properties->
                        getPropertyAsIntWithDefault(_prefix + ".SizeWarn", sizeMax * 80 / 100);
    if(sizeWarn > sizeMax)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".SizeWarn > " << _prefix << ".SizeMax; adjusted SizeWarn to SizeMax (" << sizeMax << ")";
        sizeWarn = sizeMax;
    }

    const_cast<int&>(_size) = size;
    const_cast<int&>(_sizeMax) = sizeMax;
    const_cast<int&>(_sizeWarn) = sizeWarn;

    int stackSize = _instance->initializationData().properties->getPropertyAsInt(_prefix + ".StackSize");
    if(stackSize < 0)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".StackSize < 0; Size adjusted to OS default";
        stackSize = 0;
    }
    const_cast<size_t&>(_stackSize) = static_cast<size_t>(stackSize);

    __setNoDelete(true);
    try
    {
        for(int i = 0 ; i < _size ; ++i)
        {
            IceUtil::ThreadPtr thread = new EventHandlerThread(this);
            thread->start(_stackSize);
            _threads.push_back(thread);
            ++_running;
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        {
            Error out(_instance->initializationData().logger);
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

    try
    {
        closeSocket(_fdIntrWrite);
    }
    catch(const LocalException& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "exception in `" << _prefix << "' while calling closeSocket():\n" << ex;
    }

    try
    {
        closeSocket(_fdIntrRead);
    }
    catch(const LocalException& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "exception in `" << _prefix << "' while calling closeSocket():\n" << ex;
    }

#if defined(ICE_USE_EPOLL)
    try
    {
        closeSocket(_epollFd);
    }
    catch(const LocalException& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "exception in `" << _prefix << "' while calling closeSocket():\n" << ex;
    }
#elif defined(__APPLE__)
    try
    {
	closeSocket(_kqueueFd);
    }
    catch(const LocalException& ex)
    {
	Error out(_instance->initializationData().logger);
	out << "exception in `" << _prefix << "' while calling closeSocket():\n" << ex;
    }
#endif
}

void
IceInternal::ThreadPool::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    assert(_handlerMap.empty());
    assert(_changes.empty());
    _destroyed = true;
    setInterrupt();
}

void
IceInternal::ThreadPool::incFdsInUse()
{
    // This is windows specific since every other platform uses an API
    // that doesn't have a specific FD limit.
#ifdef _WIN32
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    if(_fdsInUse + 1 > FD_SETSIZE)
    {
        Warning warn(_instance->initializationData().logger);
        warn << "maximum number of connections exceeded";

        //
        // No appropriate errno.
        //
        SocketException ex(__FILE__, __LINE__);
        ex.error = 0;
        throw ex;
    }
    ++_fdsInUse;
#endif
}

void
IceInternal::ThreadPool::decFdsInUse()
{
    // This is windows specific since every other platform uses an API
    // that doesn't have a specific FD limit.
#ifdef _WIN32
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    if(_fdsInUse <= 1)
    {
        Trace trace(_instance->initializationData().logger, "ThreadPool");
        trace << _prefix << ": about to assert";
    }
    assert(_fdsInUse > 1); // _fdIntrRead is always in use.
    --_fdsInUse;
#endif
}

void
IceInternal::ThreadPool::_register(SOCKET fd, const EventHandlerPtr& handler)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    _changes.push_back(make_pair(fd, handler));
    setInterrupt();
}

void
IceInternal::ThreadPool::unregister(SOCKET fd)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    _changes.push_back(make_pair(fd, EventHandlerPtr(0)));
    setInterrupt();
}

void
IceInternal::ThreadPool::promoteFollower()
{
    if(_sizeMax > 1)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        assert(!_promote);
        _promote = true;
        notify();

        if(!_destroyed)
        {
            assert(_inUse >= 0);
            ++_inUse;
            
            if(_inUse == _sizeWarn)
            {
                Warning out(_instance->initializationData().logger);
                out << "thread pool `" << _prefix << "' is running low on threads\n"
                    << "Size=" << _size << ", " << "SizeMax=" << _sizeMax << ", " << "SizeWarn=" << _sizeWarn;
            }
            
            assert(_inUse <= _running);
            if(_inUse < _sizeMax && _inUse == _running)
            {
                try
                {
                    IceUtil::ThreadPtr thread = new EventHandlerThread(this);
                    thread->start(_stackSize);
                    _threads.push_back(thread);
                    ++_running;
                }
                catch(const IceUtil::Exception& ex)
                {
                    Error out(_instance->initializationData().logger);
                    out << "cannot create thread for `" << _prefix << "':\n" << ex;
                }
            }
        }
    }
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
    for(vector<IceUtil::ThreadPtr>::iterator p = _threads.begin(); p != _threads.end(); ++p)
    {
        (*p)->getThreadControl().join();
    }
}

string
IceInternal::ThreadPool::prefix() const
{
    return _prefix;
}

void
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
}

void
IceInternal::ThreadPool::setInterrupt()
{
    char c = 0;

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
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        while(!_promote)
        {
            wait();
        }

        _promote = false;
    }

    while(true)
    {
        int ret;
#if defined(_WIN32)
        fd_set fdSet;
        memcpy(&fdSet, &_fdSet, sizeof(fd_set));
        if(_timeout > 0)
        {
            struct timeval tv;
            tv.tv_sec = _timeout;
            tv.tv_usec = 0;
            ret = ::select(static_cast<int>(_maxFd + 1), &fdSet, 0, 0, &tv);
        }
        else
        {
            ret = ::select(static_cast<int>(_maxFd + 1), &fdSet, 0, 0, 0);
        }
#elif defined(ICE_USE_EPOLL)
        ret = epoll_wait(_epollFd, &_events[0], _events.size(), _timeout > 0 ? _timeout * 1000 : -1);
#elif defined(__APPLE__)
        if(_timeout > 0)
        {
            struct timespec ts;
            ts.tv_sec = _timeout;
            ts.tv_nsec = 0;
            ret = kevent(_kqueueFd, 0, 0, &_events[0], _events.size(), &ts);
        }
        else
        {
            ret = kevent(_kqueueFd, 0, 0, &_events[0], _events.size(), 0);
        }
#else
        ret = poll(&_pollFdSet[0], _pollFdSet.size(), _timeout > 0 ? _timeout * 1000 : -1);
#endif

        if(ret == SOCKET_ERROR)
        {
            if(interrupted())
            {
                continue;
            }
            
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            //throw ex;
            Error out(_instance->initializationData().logger);
            out << "exception in `" << _prefix << "':\n" << ex; 
            continue;
        }
        
        EventHandlerPtr handler;
        bool finished = false;
        bool shutdown = false;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
            
            if(ret == 0) // We initiate a shutdown if there is a thread pool timeout.
            {
                assert(_timeout > 0);
                _timeout = 0;
                shutdown = true;
            }
            else
            {
                bool interrupted = false;
#if defined(_WIN32)
                interrupted = FD_ISSET(_fdIntrRead, &fdSet);
#elif defined(ICE_USE_EPOLL)
                for(int i = 0; i < ret; ++i)
                {
                    if(_events[i].data.fd == _fdIntrRead)
                    {
                        interrupted = true;
                        break;
                    }
                }
#elif defined(__APPLE__)
                for(int i = 0; i < ret; ++i)
		{
		    if(_events[i].ident == static_cast<unsigned int>(_fdIntrRead))
		    {
			interrupted = true;
			break;
		    }
		}
#else
                assert(_pollFdSet[0].fd == _fdIntrRead);
                interrupted = _pollFdSet[0].revents != 0;
#endif
                if(interrupted)
                {
                    //
                    // There are two possiblities for an interrupt:
                    //
                    // 1. The thread pool has been destroyed.
                    //
                    // 2. An event handler was registered or unregistered.
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
                    
                    clearInterrupt();
                    
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
#if defined(_WIN32)
                        FD_SET(change.first, &_fdSet);
#elif defined(ICE_USE_EPOLL)
                        epoll_event event;
                        event.events = EPOLLIN;
                        event.data.fd = change.first;
                        if(epoll_ctl(_epollFd, EPOLL_CTL_ADD, change.first, &event) != 0)
                        {
                            Error out(_instance->initializationData().logger);
                            out << "error while adding filedescriptor to epoll set:\n";
                            out << errorToString(getSocketErrno());
                            continue;
                        }
                        _events.resize(_handlerMap.size() + 1);
#elif defined(__APPLE__)
                        struct kevent event;
                        EV_SET(&event, change.first, EVFILT_READ, EV_ADD, 0, 0, 0);
                        if(kevent(_kqueueFd, &event, 1, 0, 0, 0) < 0)
                        {
			    Error out(_instance->initializationData().logger);
			    out << "error while adding filedescriptor to kqueue:\n";
			    out << errorToString(getSocketErrno());
			    continue;
                        }
			_events.resize(_handlerMap.size() + 1);
#else
                        struct pollfd pollFd;
                        pollFd.fd = change.first;
                        pollFd.events = POLLIN;
                        _pollFdSet.push_back(pollFd);
#endif
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
#if defined(_WIN32)
                        FD_CLR(change.first, &_fdSet);
#elif defined(ICE_USE_EPOLL)
                        epoll_event event;
                        event.events = 0;
                        if(epoll_ctl(_epollFd, EPOLL_CTL_DEL, change.first, &event) != 0)
                        {
                            Error out(_instance->initializationData().logger);
                            out << "error while adding filedescriptor from epoll set:\n";
                            out << errorToString(getSocketErrno());
                            continue;
                        }
                        _events.resize(_handlerMap.size() + 1);
#elif defined(__APPLE__)
                        struct kevent event;
                        EV_SET(&event, change.first, EVFILT_READ, EV_DELETE, 0, 0, 0);
                        if(kevent(_kqueueFd, &event, 1, 0, 0, 0) < 0)
                        {
			    Error out(_instance->initializationData().logger);
			    out << "error while removing filedescriptor from kqueue:\n";
			    out << errorToString(getSocketErrno());
			    continue;
                        }
			_events.resize(_handlerMap.size() + 1);
#else
                        for(vector<struct pollfd>::iterator p = _pollFdSet.begin(); p != _pollFdSet.end(); ++p)
                        {
                            if(p->fd == change.first)
                            {
                                _pollFdSet.erase(p);
                                break;
                            }
                        }
#endif
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
                else
                {
                    //
                    // Round robin for the filedescriptors.
                    //
                    SOCKET largerFd = _maxFd + 1;
                    SOCKET smallestFd = _maxFd + 1;
#if defined(_WIN32)
                    if(fdSet.fd_count == 0)
                    {
                        Error out(_instance->initializationData().logger);
                        out << "select() in `" << _prefix << "' returned " << ret
                            << " but no filedescriptor is readable";
                        continue;
                    }
                    for(u_short i = 0; i < fdSet.fd_count; ++i)
                    {
                        SOCKET fd = fdSet.fd_array[i];
#elif defined(ICE_USE_EPOLL)
                    for(int i = 0; i < ret; ++i)
                    {
                        SOCKET fd = _events[i].data.fd;
#elif defined(__APPLE__)
		    for(int i = 0; i < ret; ++i)
		    {
			SOCKET fd = _events[i].ident;
#else
                    for(vector<struct pollfd>::const_iterator p = _pollFdSet.begin(); p != _pollFdSet.end(); ++p)
                    {
                        if(p->revents == 0)
                        {
                            continue;
                        }
                        SOCKET fd = p->fd;
#endif
                        assert(fd != INVALID_SOCKET);
                        if(fd > _lastFd || _lastFd == INVALID_SOCKET)
                        {
                            largerFd = min(largerFd, fd);
                        }
                        
                        smallestFd = min(smallestFd, fd);
                    }
#ifdef never // To match ICE_USE_EPOLL __APPLE
                    }}}
#endif
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
                    assert(_lastFd != _fdIntrRead);
                    map<SOCKET, EventHandlerPtr>::iterator p = _handlerMap.find(_lastFd);
                    if(p == _handlerMap.end())
                    {
                        Error out(_instance->initializationData().logger);
                        out << "filedescriptor " << _lastFd << " not registered with `" << _prefix << "'";
                        continue;
                    }
                    
                    handler = p->second;
                }
            }
        }
        
        //
        // Now we are outside the thread synchronization.
        //
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

            //
            // No "continue", because we want shutdown to be done in
            // its own thread from this pool. Therefore we called
            // promoteFollower().
            //
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
                    //
                    // "self" is faster than "this", as the reference
                    // count is not modified.
                    //
                    handler->finished(self);
                }
                catch(const LocalException& ex)
                {
                    Error out(_instance->initializationData().logger);
                    out << "exception in `" << _prefix << "' while calling finished():\n"
                        << ex << '\n' << handler->toString();
                }

                //
                // No "continue", because we want finished() to be
                // called in its own thread from this pool. Note that
                // this means that finished() must call
                // promoteFollower().
                //
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
                    catch(const DatagramLimitException&) // Expected.
                    {
                        continue;
                    }
                    catch(const SocketException& ex)
                    {
                        handler->exception(ex);
                        continue;
                    }
                    catch(const LocalException& ex)
                    {
                        if(handler->datagram())
                        {
                            if(_instance->initializationData().properties->
                                        getPropertyAsInt("Ice.Warn.Connections") > 0)
                            {
                                Warning out(_instance->initializationData().logger);
                                out << "datagram connection exception:\n" << ex << '\n' << handler->toString();
                            }
                        }
                        else
                        {
                            handler->exception(ex);
                        }
                        continue;
                    }
                    
                    stream.swap(handler->_stream);
                    assert(stream.i == stream.b.end());
                }
            
                //
                // Provide a new mesage to the handler.
                //
                try
                {
                    //
                    // "self" is faster than "this", as the reference
                    // count is not modified.
                    //
                    handler->message(stream, self);
                }
                catch(const LocalException& ex)
                {
                    Error out(_instance->initializationData().logger);
                    out << "exception in `" << _prefix << "' while calling message():\n"
                        << ex << '\n' << handler->toString();
                }
                
                //
                // No "continue", because we want message() to be
                // called in its own thread from this pool. Note that
                // this means that message() must call
                // promoteFollower().
                //
            }
        }

        if(_sizeMax > 1)
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
            
            if(!_destroyed)
            {
                //
                // First we reap threads that have been destroyed before.
                //
                int sz = static_cast<int>(_threads.size());
                assert(_running <= sz);
                if(_running < sz)
                {
                    vector<IceUtil::ThreadPtr>::iterator start =
                        partition(_threads.begin(), _threads.end(), IceUtil::constMemFun(&IceUtil::Thread::isAlive));

                    for(vector<IceUtil::ThreadPtr>::iterator p = start; p != _threads.end(); ++p)
                    {
                        (*p)->getThreadControl().join();
                    }

                    _threads.erase(start, _threads.end());
                }
                
                //
                // Now we check if this thread can be destroyed, based
                // on a load factor.
                //

                //
                // The load factor jumps immediately to the number of
                // threads that are currently in use, but decays
                // exponentially if the number of threads in use is
                // smaller than the load factor. This reflects that we
                // create threads immediately when they are needed,
                // but want the number of threads to slowly decline to
                // the configured minimum.
                //
                double inUse = static_cast<double>(_inUse);
                if(_load < inUse)
                {
                    _load = inUse;
                }
                else
                {
                    const double loadFactor = 0.05; // TODO: Configurable?
                    const double oneMinusLoadFactor = 1 - loadFactor;
                    _load = _load * oneMinusLoadFactor + inUse * loadFactor;
                }
                
                if(_running > _size)
                {
                    int load = static_cast<int>(_load + 0.5);

                    //
                    // We add one to the load factor because on
                    // additional thread is needed for select().
                    //
                    if(load + 1 < _running)
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


            while(!_promote)
            {
                wait();
            }
            
            _promote = false;
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
    if(pos < headerSize)
    {
        //
        // This situation is possible for small UDP packets.
        //
        throw IllegalMessageSizeException(__FILE__, __LINE__);
    }
    stream.i = stream.b.begin();
    const Byte* m;
    stream.readBlob(m, static_cast<Int>(sizeof(magic)));
    if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
    {
        BadMagicException ex(__FILE__, __LINE__);
        ex.badMagic = Ice::ByteSeq(&m[0], &m[0] + sizeof(magic));
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
    if(size > static_cast<Int>(_instance->messageSizeMax()))
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
        if(handler->datagram())
        {
            if(_warnUdp)
            {
                Warning out(_instance->initializationData().logger);
                out << "DatagramLimitException: maximum size of " << pos << " exceeded";
                stream.resize(0);
                stream.i = stream.b.begin();
            }
            throw DatagramLimitException(__FILE__, __LINE__);
        }
        else
        {
            handler->read(stream);
            assert(stream.i == stream.b.end());
        }
    }
}

IceInternal::ThreadPool::EventHandlerThread::EventHandlerThread(const ThreadPoolPtr& pool) :
    _pool(pool)
{
}

void
IceInternal::ThreadPool::EventHandlerThread::run()
{
    if(_pool->_instance->initializationData().threadHook)
    {
        _pool->_instance->initializationData().threadHook->start();
    }

    bool promote;

    try
    {
        promote = _pool->run();
    }
    catch(const Exception& ex)
    {   
        Error out(_pool->_instance->initializationData().logger);
        out << "exception in `" << _pool->_prefix << "':\n" << ex; 
        promote = true;
    }
    catch(const std::exception& ex)
    {
        Error out(_pool->_instance->initializationData().logger);
        out << "std::exception in `" << _pool->_prefix << "':\n" << ex.what();
        promote = true;
    }
    catch(...)
    {
        Error out(_pool->_instance->initializationData().logger);
        out << "unknown exception in `" << _pool->_prefix << "'"; 
        promote = true;
    }

    if(promote && _pool->_sizeMax > 1)
    {
        //
        // Promote a follower, but w/o modifying _inUse or creating
        // new threads.
        //
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*_pool.get());
            assert(!_pool->_promote);
            _pool->_promote = true;
            _pool->notify();
        }
    }

    if(_pool->_instance->initializationData().threadHook)
    {
        _pool->_instance->initializationData().threadHook->stop();
    }

    _pool = 0; // Break cyclic dependency.
}
