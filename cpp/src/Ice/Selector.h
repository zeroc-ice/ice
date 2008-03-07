// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SELECTOR_H
#define ICE_SELECTOR_H

#include <IceUtil/StringUtil.h>

#include <Ice/Config.h>
#include <Ice/Network.h>
#include <Ice/SelectorF.h>
#include <Ice/InstanceF.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>

#if defined(__linux) && !defined(ICE_NO_EPOLL)
#   define ICE_USE_EPOLL 1
#elif defined(__APPLE__) && !defined(ICE_NO_KQUEUE)
#   define ICE_USE_KQUEUE 1
#elif defined(_WIN32)
#   define ICE_USE_SELECT 1
#endif

#if defined(ICE_USE_EPOLL)
#   include <sys/epoll.h>
#elif defined(ICE_USE_KQUEUE)
#   include <sys/event.h>
#elif !defined(ICE_USE_SELECT)
#   include <sys/poll.h>
#endif

namespace IceInternal
{

template<class T> class Selector
{
public:

    Selector(const InstancePtr& instance, int timeout = 0) :
        _instance(instance),
        _timeout(timeout),
        _interruptCount(0)
    {
        SOCKET fds[2];
        createPipe(fds);
        _fdIntrRead = fds[0];
        _fdIntrWrite = fds[1];
        _lastFd = _fdIntrRead;

#if defined(ICE_USE_EPOLL) || defined(ICE_USE_KQUEUE)
        _count = 0;
        _events.resize(32);
#if defined(ICE_USE_EPOLL)
        _queueFd = epoll_create(1);
        if(_queueFd < 0)
        {
            Ice::SocketException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
        epoll_event event;
        event.events = EPOLLIN;
        event.data.ptr = 0;
        if(epoll_ctl(_queueFd, EPOLL_CTL_ADD, _fdIntrRead, &event) != 0)
        {
            Ice::SocketException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
#else
        _queueFd = kqueue();
        if(_queueFd < 0)
        {
            Ice::SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        struct kevent event;
        EV_SET(&event, _fdIntrRead, EVFILT_READ, EV_ADD, 0, 0, 0);
        if(kevent(_queueFd, &event, 1, 0, 0, 0) < 0)
        {
            Ice::SocketException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
#endif
#elif defined(ICE_USE_SELECT)
        _fdsInUse = 1;
        FD_ZERO(&_readFdSet);
        FD_ZERO(&_writeFdSet);
        FD_ZERO(&_errorFdSet);
        FD_SET(_fdIntrRead, &_readFdSet);
#else
        struct pollfd pollFd;
        pollFd.fd = _fdIntrRead;
        pollFd.events = POLLIN;
        _pollFdSet.push_back(pollFd);
#endif
    }

    ~Selector()
    {
#if defined(ICE_USE_EPOLL) || defined(ICE_USE_KQUEUE)
        try
        {
            closeSocket(_queueFd);
        }
        catch(const Ice::LocalException& ex)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "exception in selector while calling closeSocket():\n" << ex;
        }
#endif

        try
        {
            closeSocket(_fdIntrWrite);
        }
        catch(const Ice::LocalException& ex)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "exception in selector while calling closeSocket():\n" << ex;
        }

        try
        {
            closeSocket(_fdIntrRead);
        }
        catch(const Ice::LocalException& ex)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "exception in selector while calling closeSocket():\n" << ex;
        }
    }

    void add(T* handler, SocketStatus status, bool noInterrupt = false)
    {
        assert(status != Finished);
#if defined(ICE_USE_EPOLL) || defined(ICE_USE_KQUEUE)
#if defined(ICE_USE_EPOLL)
        epoll_event event;
        event.events = status == NeedRead ? EPOLLIN : EPOLLOUT;
        event.data.ptr = handler;
        if(epoll_ctl(_queueFd, EPOLL_CTL_ADD, handler->_fd, &event) != 0)
#else // ICE_USE_KQUEUE
        struct kevent event;
        if(status == NeedRead)
        {
            EV_SET(&event, handler->_fd, EVFILT_READ, EV_ADD, 0, 0, handler);
        }
        else
        {
            EV_SET(&event, handler->_fd, EVFILT_WRITE, EV_ADD, 0, 0, handler);
        }
        if(kevent(_queueFd, &event, 1, 0, 0, 0) < 0)
#endif
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "error while adding filedescriptor to selector:\n";
            out << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
        }
        ++_count;
#else // ICE_USE_SELECT or ICE_USE_POLL
        if(noInterrupt)
        {
            processInterrupt();
            addImpl(handler, status);
        }
        else
        {
            _changes.push_back(ChangeInfo(handler, status, false));
            setInterrupt();
        }
#endif
    }

    void update(T* handler, SocketStatus oldStatus, SocketStatus newStatus)
    {
        // Note: can only be called from the select() thread (remove/add don't use interrupts)
        assert(newStatus != Finished);
#if defined(ICE_USE_EPOLL)
        epoll_event event;
        event.events = newStatus == NeedRead ? EPOLLIN : EPOLLOUT;
        event.data.ptr = handler;
        if(epoll_ctl(_queueFd, EPOLL_CTL_MOD, handler->_fd, &event) != 0)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "error while updating filedescriptor from selector:\n";
            out << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
        }
#else
        remove(handler, oldStatus, true);
        add(handler, newStatus, true);
#endif
    }

    void remove(T* handler, SocketStatus status, bool noInterrupt = false)
    {
#if defined(ICE_USE_EPOLL) || defined(ICE_USE_KQUEUE)
#if defined(ICE_USE_EPOLL)
        epoll_event event;
        event.events = 0;
        int rs = epoll_ctl(_queueFd, EPOLL_CTL_DEL, handler->_fd, &event);
#else // ICE_USE_KQUEUE
        struct kevent event;
        if(status == NeedRead)
        {
            EV_SET(&event, handler->_fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
        }
        else
        {
            EV_SET(&event, handler->_fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
        }
        int rs = kevent(_queueFd, &event, 1, 0, 0, 0);
#endif
        if(rs < 0)
        {
            //
            // It's possible for the socket to already be closed at this point.
            //
            //Ice::Error out(_instance->initializationData().logger);
            //out << "error while removing filedescriptor from epoll set:\n";
            //out << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
        }
        --_count;
#else
        if(noInterrupt)
        {
            processInterrupt();
            removeImpl(handler, status);
        }
        else
        {
            _changes.push_back(ChangeInfo(handler, status, true));
            setInterrupt();
        }
#endif
    }

    int select()
    {
        while(true)
        {
            int ret;
            _nSelectedReturned = 0;
            _nSelected = 0;
#if defined(ICE_USE_EPOLL)
            ret = epoll_wait(_queueFd, &_events[0], _events.size(), _timeout > 0 ? _timeout * 1000 : -1);
#elif defined(ICE_USE_KQUEUE)
            assert(!_events.empty());
            if(_timeout > 0)
            {
                struct timespec ts;
                ts.tv_sec = _timeout;
                ts.tv_nsec = 0;
                ret = kevent(_queueFd, 0, 0, &_events[0], _events.size(), &ts);
            }
            else
            {
                ret = kevent(_queueFd, 0, 0, &_events[0], _events.size(), 0);
            }
#elif defined(ICE_USE_SELECT)
            fd_set* rFdSet = fdSetCopy(_selectedReadFdSet, _readFdSet);
            fd_set* wFdSet = fdSetCopy(_selectedWriteFdSet, _writeFdSet);
            fd_set* eFdSet = fdSetCopy(_selectedErrorFdSet, _errorFdSet);
            if(_timeout > 0)
            {
                struct timeval tv;
                tv.tv_sec = _timeout;
                tv.tv_usec = 0;
                ret = ::select(0, rFdSet, wFdSet, eFdSet, &tv); // The first parameter is ignored on Windows
            }
            else
            {
                ret = ::select(0, rFdSet, wFdSet, eFdSet, 0); // The first parameter is ignored on Windows
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

                assert(false);
                Ice::SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }

            assert(ret >= 0);
            _nSelected = static_cast<unsigned int>(ret);
            if(_nSelected == 0)
            {
                assert(_timeout > 0);
                _timeout = 0;
            }
            return _nSelected;
        }
    }

    T* getNextSelected()
    {
        assert(_nSelected > 0);

#if defined(ICE_USE_EPOLL) || defined(ICE_USE_KQUEUE)
        if(_nSelectedReturned == _nSelected)
        {
            if(_count != _events.size())
            {
                _events.resize(_count);
            }
            return 0;
        }

        //
        // Round robin for the filedescriptors.
        //
        T* larger = 0;
        T* smallest = 0;
        for(unsigned int i = 0; i < _nSelected; ++i)
        {
#if defined(ICE_USE_EPOLL)
            T* handler = reinterpret_cast<T*>(_events[i].data.ptr);
#else
            T* handler = reinterpret_cast<T*>(_events[i].udata);
#endif
            if(!handler) // _fdIntrRead
            {
                assert(_nSelectedReturned > 0 && _interruptCount == 0);
                continue;
            }

            if(handler->_fd > _lastFd && (larger == 0 || handler->_fd < larger->_fd))
            {
                larger = handler;
            }

            if(smallest == 0 || handler->_fd < smallest->_fd)
            {
                smallest = handler;
            }
        }

        ++_nSelectedReturned;
        if(larger)
        {
            _lastFd = larger->_fd;
            return larger;
        }
        else
        {
            assert(smallest);
            _lastFd = smallest->_fd;
            return smallest;
        }
#else
        if(_nSelectedReturned == _nSelected)
        {
            return 0;
        }

        //
        // Round robin for the filedescriptors.
        //
        SOCKET largerFd = INVALID_SOCKET;
        SOCKET smallestFd = INVALID_SOCKET;
#if defined(ICE_USE_SELECT)
        if(_selectedReadFdSet.fd_count == 0 && _selectedWriteFdSet.fd_count == 0 && _selectedErrorFdSet.fd_count == 0)
        {
            Error out(_instance->initializationData().logger);
            out << "select() in selector returned " << _nSelected << " but no filedescriptor is ready";
            return 0;
        }

        const fd_set* fdSet;
        if(_nSelectedReturned < _selectedReadFdSet.fd_count)
        {
            fdSet = &_selectedReadFdSet;
        }
        else if(_nSelectedReturned < _selectedWriteFdSet.fd_count + _selectedReadFdSet.fd_count)
        {
            fdSet = &_selectedWriteFdSet;
        }
        else
        {
            fdSet = &_selectedErrorFdSet;
        }

        for(u_short i = 0; i < fdSet->fd_count; ++i)
        {
            SOCKET fd = fdSet->fd_array[i];
#else
        for(std::vector<struct pollfd>::const_iterator p = _pollFdSet.begin(); p != _pollFdSet.end(); ++p)
        {
            if(p->revents == 0)
            {
                continue;
            }
            SOCKET fd = p->fd;
#endif
            if(fd == _fdIntrRead)
            {
                assert(_nSelectedReturned > 0 && _interruptCount == 0);
                continue;
            }

            assert(fd != INVALID_SOCKET);
            if(fd > _lastFd && (largerFd == INVALID_SOCKET || largerFd > fd))
            {
                largerFd = fd;
            }

            if(smallestFd == INVALID_SOCKET || fd < smallestFd)
            {
                smallestFd = fd;
            }
        }

        if(largerFd != INVALID_SOCKET)
        {
            _lastFd = largerFd;
        }
        else
        {
            assert(smallestFd != INVALID_SOCKET);
            _lastFd = smallestFd;
        }

        typename std::map<SOCKET, T*>::const_iterator q = _handlerMap.find(_lastFd);
        if(q == _handlerMap.end())
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "filedescriptor " << _lastFd << " not registered with selector";
            return 0;
        }
        ++_nSelectedReturned;
        return q->second;
#endif
    }

    bool processInterrupt()
    {
#if !defined(ICE_USE_EPOLL) && !defined(ICE_USE_KQUEUE)
        assert(_changes.size() <= _interruptCount);
        while(!_changes.empty())
        {
            clearInterrupt();
            ChangeInfo& change = _changes.front();
            if(change.remove)
            {
                removeImpl(change.handler, change.status);
            }
            else
            {
                addImpl(change.handler, change.status);
            }
            _changes.pop_front();
        }
#endif
        return _interruptCount == 0; // No more interrupts to process.
    }

    bool isInterrupted()
    {
        return _interruptCount > 0;
    }

    void setInterrupt()
    {
        if(++_interruptCount == 1)
        {
            char c = 0;
            while(true)
            {
#ifdef _WIN32
                if(::send(_fdIntrWrite, &c, 1, 0) == SOCKET_ERROR)
#else
                if(::write(_fdIntrWrite, &c, 1) == SOCKET_ERROR)
#endif
                {
                    if(interrupted())
                    {
                        continue;
                    }

                    Ice::SocketException ex(__FILE__, __LINE__);
                    ex.error = IceInternal::getSocketErrno();
                    throw ex;
                }
                break;
            }
        }
    }

    bool clearInterrupt()
    {
        assert(_interruptCount > 0);
        if(--_interruptCount == 0)
        {
            char c;

            while(true)
            {
                ssize_t ret;
#ifdef _WIN32
                ret = ::recv(_fdIntrRead, &c, 1, 0);
#else
                ret = ::read(_fdIntrRead, &c, 1);
#endif
                if(ret == SOCKET_ERROR)
                {
                    if(interrupted())
                    {
                        continue;
                    }

                    Ice::SocketException ex(__FILE__, __LINE__);
                    ex.error = IceInternal::getSocketErrno();
                    throw ex;
                }
                break;
            }
            ++_nSelectedReturned;
            return false;
        }
        return true;
    }

#if defined(ICE_USE_SELECT)
    void incFdsInUse()
    {
        if(_fdsInUse + 1 > FD_SETSIZE)
        {
            Ice::Warning warn(_instance->initializationData().logger);
            warn << "maximum number of connections exceeded";

            //
            // No appropriate errno.
            //
            Ice::SocketException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }
        ++_fdsInUse;
    }

    void decFdsInUse()
    {
        // This is windows specific since every other platform uses an API
        // that doesn't have a specific FD limit.
        if(_fdsInUse <= 1)
        {
            Trace trace(_instance->initializationData().logger, "ThreadPool");
            trace << "selector: about to assert";
        }
        --_fdsInUse;
    }
#endif

private:

#if !defined(ICE_USE_EPOLL) && !defined(ICE_USE_KQUEUE)
    void addImpl(T* handler, SocketStatus status)
    {
        assert(_handlerMap.find(handler->_fd) == _handlerMap.end());
        _handlerMap.insert(make_pair(handler->_fd, handler));
#if defined(ICE_USE_SELECT)
        switch(status)
        {
        case NeedRead:
            FD_SET(handler->_fd, &_readFdSet);
            break;
        case NeedWrite:
            FD_SET(handler->_fd, &_writeFdSet);
            break;
        case NeedConnect:
            FD_SET(handler->_fd, &_writeFdSet);
            FD_SET(handler->_fd, &_errorFdSet);
            break;
        case Finished:
            assert(false);
        }
#else
        struct pollfd pollFd;
        pollFd.fd = handler->_fd;
        pollFd.events = status == NeedRead ? POLLIN : POLLOUT;
        _pollFdSet.push_back(pollFd);
#endif
    } 

    void removeImpl(T* handler, SocketStatus status)
    {
        typename std::map<SOCKET, T*>::iterator p = _handlerMap.find(handler->_fd);
        assert(p != _handlerMap.end());
        _handlerMap.erase(p);
#if defined(ICE_USE_SELECT)
        switch(status)
        {
        case NeedRead:
            FD_CLR(handler->_fd, &_readFdSet);
            break;
        case NeedWrite:
            FD_CLR(handler->_fd, &_writeFdSet);
            break;
        case NeedConnect:
            FD_CLR(handler->_fd, &_writeFdSet);
            FD_CLR(handler->_fd, &_errorFdSet);
            break;
        case Finished:
            assert(false);
        }
#else
        for(std::vector<struct pollfd>::iterator p = _pollFdSet.begin(); p != _pollFdSet.end(); ++p)
        {
            if(p->fd == handler->_fd)
            {
                _pollFdSet.erase(p);
                break;
            }
        }
#endif
    }
#endif

    InstancePtr _instance;
    int _timeout;
    SOCKET _lastFd;
    unsigned int _nSelected;
    unsigned int _nSelectedReturned;
    SOCKET _fdIntrRead;
    SOCKET _fdIntrWrite;
    unsigned int _interruptCount;

#if defined(ICE_USE_EPOLL) || defined(ICE_USE_KQUEUE)
#if defined(ICE_USE_EPOLL)
    std::vector<struct epoll_event> _events;
#else
    std::vector<struct kevent> _events;
#endif
    int _queueFd;
    unsigned int _count;
#else

    struct ChangeInfo
    {
        ChangeInfo(T* h, SocketStatus s, bool r) : handler(h), status(s), remove(r)
        {
        }
        T* handler;
        SocketStatus status;
        bool remove;
    };
    typename std::list<ChangeInfo> _changes; // handler set for addition; null for removal.
    typename std::map<SOCKET, T*> _handlerMap;

#if defined(ICE_USE_SELECT)
    fd_set _readFdSet;
    fd_set _writeFdSet;
    fd_set _errorFdSet;
    fd_set _selectedReadFdSet;
    fd_set _selectedWriteFdSet;
    fd_set _selectedErrorFdSet;
    int _fdsInUse;

    fd_set*
    fdSetCopy(fd_set& dest, fd_set& src)
    {
        if(src.fd_count > 0)
        {
            dest.fd_count = src.fd_count;
            memcpy(dest.fd_array, src.fd_array, sizeof(SOCKET) * src.fd_count);
            return &dest;
        }
        return 0;
    }
#else
    std::vector<struct pollfd> _pollFdSet;
#endif
#endif
};

}

#endif
