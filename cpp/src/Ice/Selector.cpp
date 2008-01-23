// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/StringUtil.h>
#include <Ice/Selector.h>
#include <Ice/Network.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::Selector::Selector(const InstancePtr& instance, int timeout) : 
    _instance(instance),
    _timeout(timeout)
{
#if defined(_WIN32)
    _fdsInUse = 0;
    FD_ZERO(&_readFdSet);
    FD_ZERO(&_writeFdSet);
    FD_ZERO(&_errorFdSet);
#elif defined(ICE_USE_EPOLL)
    _epollFd = epoll_create(1);
    if(_epollFd < 0)
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
#endif

    SOCKET fds[2];
    createPipe(fds);
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    setBlock(_fdIntrRead, false);
    _maxFd = _fdIntrRead;
    add(_fdIntrRead, NeedRead);
#if defined(_WIN32)
    ++_fdsInUse;
#endif

    _lastFd = _fdIntrRead;
}

IceInternal::Selector::~Selector()
{
#if defined(ICE_USE_EPOLL)
    try
    {
        closeSocket(_epollFd);
    }
    catch(const LocalException& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }
#elif defined(__APPLE__)
    try
    {
	closeSocket(_kqueueFd);
    }
    catch(const LocalException& ex)
    {
	Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }
#endif


    try
    {
        closeSocket(_fdIntrWrite);
    }
    catch(const LocalException& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }

    try
    {
        closeSocket(_fdIntrRead);
    }
    catch(const LocalException& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }
}

void
IceInternal::Selector::add(SOCKET fd, SocketStatus status)
{
    if(fd > _maxFd)
    {
        _maxFd = fd;
    }
#if defined(_WIN32)
    switch(status)
    {
    case NeedRead:
        FD_SET(fd, &_readFdSet);
        break;
    case NeedWrite:
        FD_SET(fd, &_writeFdSet);
	break;
    case NeedConnect:
        FD_SET(fd, &_writeFdSet);
        FD_SET(fd, &_errorFdSet);
        break;
    case Finished:
        assert(false);
    }
#elif defined(ICE_USE_EPOLL)
    epoll_event event;
    switch(status)
    {
    case NeedRead:
        event.events = EPOLLIN;
        break;
    case NeedWrite:
    case NeedConnect:
        event.events = EPOLLOUT;
        break;
    case Finished:
        assert(false);
    }
    event.data.fd = fd;
    if(epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) != 0)
    {
        Error out(_instance->initializationData().logger);
        out << "error while adding filedescriptor to epoll set:\n";
        out << IceUtilInternal::errorToString(getSocketErrno());
    }
    _events.resize(_events.size() + 1);
#elif defined(__APPLE__)
    struct kevent event;
    switch(status)
    {
    case NeedRead:
        EV_SET(&event, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
        break;
    case NeedWrite:
    case NeedConnect:
        EV_SET(&event, fd, EVFILT_WRITE, EV_ADD, 0, 0, 0);
        break;
    case Finished:
        assert(false);
    }
    if(kevent(_kqueueFd, &event, 1, 0, 0, 0) < 0)
    {
        Error out(_instance->initializationData().logger);
        out << "error while adding filedescriptor to kqueue:\n";
        out << IceUtilInternal::errorToString(getSocketErrno());
    }
    _events.resize(_events.size() + 1);
#else
    struct pollfd pollFd;
    pollFd.fd = fd;
    switch(status)
    {
    case NeedRead:
        pollFd.events = POLLIN;
        break;
    case NeedWrite:
    case NeedConnect:
        pollFd.events = POLLOUT;
        break;
    case Finished:
        assert(false);
    }
    _pollFdSet.push_back(pollFd);
#endif
}

void
Selector::remove(SOCKET fd, SocketStatus status)
{
#if defined(_WIN32)
    switch(status)
    {
    case NeedRead:
        FD_CLR(fd, &_readFdSet);
        break;
    case NeedWrite:
        FD_CLR(fd, &_writeFdSet);
	break;
    case NeedConnect:
        FD_CLR(fd, &_writeFdSet);
        FD_CLR(fd, &_errorFdSet);
        break;
    case Finished:
        assert(false);
    }
#elif defined(ICE_USE_EPOLL)
    epoll_event event;
    event.events = 0;
    int rs = epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, &event);
    if(rs < 0)
    {
        //
        // It's possible for the socket to already be closed at this point.
        //
//         Error out(_instance->initializationData().logger);
//         out << "error while removing filedescriptor from epoll set:\n";
//         out << IceUtilInternal::errorToString(getSocketErrno());
    }
    _events.resize(_events.size() - 1);
#elif defined(__APPLE__)
    struct kevent event;
    switch(status)
    {
    case NeedRead:
        EV_SET(&event, fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
        break;
    case NeedWrite:
    case NeedConnect:
        EV_SET(&event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
        break;
    case Finished:
        assert(false);
    }
    int rs = kevent(_kqueueFd, &event, 1, 0, 0, 0);
    if(rs < 0)
    {
        //
        // It's possible for the socket to already be closed at this point.
        //
//         Error out(_instance->initializationData().logger);
//         out << "error while removing filedescriptor from kqueue:\n";
//         out << IceUtilInternal::errorToString(getSocketErrno());
    }
    _events.resize(_events.size() - 1);
#else
    for(vector<struct pollfd>::iterator p = _pollFdSet.begin(); p != _pollFdSet.end(); ++p)
    {
        if(p->fd == fd)
        {
            _pollFdSet.erase(p);
            break;
        }
    }
#endif
}

SOCKET
IceInternal::Selector::getNextSelected()
{
    assert(_nSelected > 0);
    if(_nSelectedReturned == _nSelected)
    {
        return INVALID_SOCKET;
    }

    //
    // Round robin for the filedescriptors.
    //
    SOCKET largerFd = _maxFd + 1;
    SOCKET smallestFd = _maxFd + 1;
#if defined(_WIN32)
    if(_selectedReadFdSet.fd_count == 0 && _selectedWriteFdSet.fd_count == 0 && _selectedErrorFdSet.fd_count == 0)
    {
        Error out(_instance->initializationData().logger);
        out << "select() in selector returned " << _nSelected << " but no filedescriptor is ready";
	return INVALID_SOCKET;
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
#elif defined(ICE_USE_EPOLL)
    for(unsigned int i = 0; i < _nSelected; ++i)
    {
        SOCKET fd = _events[i].data.fd;
#elif defined(__APPLE__)
    for(unsigned int i = 0; i < _nSelected; ++i)
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
        if(fd > _lastFd)
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
        _lastFd = largerFd;
    }
    else
    {
	assert(smallestFd <= _maxFd);
        _lastFd = smallestFd;
    }
    ++_nSelectedReturned;
    return _lastFd;
}

int
IceInternal::Selector::select()
{
    while(true)
    {
        int ret;
        _nSelectedReturned = 0;
        _nSelected = 0;
#if defined(_WIN32)
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
#elif defined(ICE_USE_EPOLL)
        ret = epoll_wait(_epollFd, &_events[0], _events.size(), _timeout > 0 ? _timeout * 1000 : -1);
#elif defined(__APPLE__)
        assert(!_events.empty());
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

void
IceInternal::Selector::setInterrupt()
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
            
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        break;
    }
}

void
IceInternal::Selector::clearInterrupt()
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
            
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        break;
    }
}

#ifdef _WIN32
void
IceInternal::Selector::incFdsInUse()
{
    // This is windows specific since every other platform uses an API
    // that doesn't have a specific FD limit.
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
}
#endif

#ifdef _WIN32
void
IceInternal::Selector::decFdsInUse()
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

