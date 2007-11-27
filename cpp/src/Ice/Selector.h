// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SELECTOR_H
#define ICE_SELECTOR_H

#include <Ice/Config.h>
#include <Ice/InstanceF.h>

#if defined(_WIN32)
#   include <winsock2.h>
#else
#   define SOCKET int
#   if defined(ICE_USE_EPOLL)
#       include <sys/epoll.h>
#   elif defined(__APPLE__)
#       include <sys/event.h>
#   else
#       include <sys/poll.h>
#   endif
#endif


namespace IceInternal
{

enum SocketStatus
{
    Finished,
    NeedConnect,
    NeedRead,
    NeedWrite
};

class Selector
{
public:

    Selector(const InstancePtr&, int = 0);
    ~Selector();

    void add(SOCKET, SocketStatus);
    void remove(SOCKET, SocketStatus);

    int select();
    SOCKET getNextSelected();
    
    bool isInterrupted()
    {
        assert(_nSelected > 0);
#if defined(_WIN32)
        return FD_ISSET(_fdIntrRead, &_selectedReadFdSet);
#elif defined(ICE_USE_EPOLL)
        for(unsigned int i = 0; i < _nSelected; ++i)
        {
            if(_events[i].data.fd == _fdIntrRead)
            {
                return true;
            }
        }
        return false;
#elif defined(__APPLE__)
        for(unsigned int i = 0; i < _nSelected; ++i)
        {
            if(_events[i].ident == static_cast<unsigned int>(_fdIntrRead))
            {
                return true;
            }
        }
        return false;
#else
        assert(_pollFdSet[0]->fd == _fdIntrRead);
        return _pollFdSet[0]->revents != 0;
#endif
    }    
    void setInterrupt();
    void clearInterrupt();

    void updateMinMax(SOCKET minFd, SOCKET maxFd);
    void clearMinMax();

#if defined(_WIN32)
    void incFdsInUse();
    void decFdsInUse();
#endif

private:

    InstancePtr _instance;
    int _timeout;
    SOCKET _maxFd;
    SOCKET _lastFd;
    unsigned int _nSelected;
    unsigned int _nSelectedReturned;
    SOCKET _fdIntrRead;
    SOCKET _fdIntrWrite;
#if defined(_WIN32)
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
    
#elif defined(ICE_USE_EPOLL)
    int _epollFd;
    std::vector<struct epoll_event> _events;
#elif defined(__APPLE__)
    int _kqueueFd;
    std::vector<struct kevent> _events;
#else
    std::vector<struct pollfd> _pollFdSet;
#endif
};

}

#endif
