// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/SelectorThread.h>
#include <Ice/Network.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(SelectorThread* p) { return p; }

IceInternal::SelectorThread::SelectorThread(const InstancePtr& instance) :
    _instance(instance),
    _destroyed(false),
    _selector(instance),
    _timer(_instance->timer())
{

    __setNoDelete(true);
    try
    {
        _thread = new HelperThread(this);
        _thread->start();
    }
    catch(const IceUtil::Exception& ex)
    {
        {
            Error out(_instance->initializationData().logger);
            out << "cannot create thread for selector thread:\n" << ex;
        }
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

IceInternal::SelectorThread::~SelectorThread()
{
    assert(_destroyed);
}

void
IceInternal::SelectorThread::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    _destroyed = true;
    _selector.setInterrupt();
}

void
IceInternal::SelectorThread::incFdsInUse()
{
    // This is windows specific since every other platform uses an API
    // that doesn't have a specific FD limit.
#ifdef _WIN32
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    _selector.incFdsInUse();
#endif
}

void
IceInternal::SelectorThread::decFdsInUse()
{
    // This is windows specific since every other platform uses an API
    // that doesn't have a specific FD limit.
#ifdef _WIN32
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    _selector.decFdsInUse();
#endif
}

void
IceInternal::SelectorThread::_register(SOCKET fd, const SocketReadyCallbackPtr& cb, SocketStatus status, int timeout)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
    assert(status != Finished);
    SocketInfo info(fd, cb, status, timeout);
    _changes.push_back(info);
    if(info.timeout >= 0)
    {
        _timer->schedule(info.cb, IceUtil::Time::milliSeconds(info.timeout));
    }
    _selector.setInterrupt();
}

void
IceInternal::SelectorThread::unregister(SOCKET fd)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
    _changes.push_back(SocketInfo(fd, 0, Finished, 0));
    _selector.setInterrupt();
}

void
IceInternal::SelectorThread::joinWithThread()
{
    assert(_destroyed);
    if(_thread)
    {
        _thread->getThreadControl().join();
    }
}

void
IceInternal::SelectorThread::run()
{
    std::map<SOCKET, SocketInfo> socketMap;
    vector<SocketInfo*> readyList;
    vector<SocketInfo*> finishedList;
    while(true)
    {
        try
        {
            _selector.select();
        }
        catch(const Ice::LocalException& ex)
        {
            Error out(_instance->initializationData().logger);
            out << "exception in selector thread:\n" << ex;
            continue;
        }

        assert(readyList.empty() && finishedList.empty());

        {
            if(_selector.isInterrupted())
            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

                //
                // There are two possiblities for an interrupt:
                //
                // 1. The selector thread has been destroyed.
                // 2. A socket was registered or unregistered.
                //

                //
                // Thread destroyed?
                //
                if(_destroyed)
                {
                    break;
                }

                _selector.clearInterrupt();

                SocketInfo& change = _changes.front();
                if(change.cb) // Registration
                {
                    _selector.add(change.fd, change.status);
                    assert(socketMap.find(change.fd) == socketMap.end());
                    socketMap.insert(make_pair(change.fd, change));
                    _maxFd = max(_maxFd, change.fd);
                    _minFd = min(_minFd, change.fd);
                }
                else // Unregistration
                {
                    map<SOCKET, SocketInfo>::iterator r = socketMap.find(change.fd);
                    if(r != socketMap.end() && r->second.status != Finished)
                    {
                        if(r->second.timeout >= 0)
                        {
                            _timer->cancel(r->second.cb);
                        }
                        assert(r->second.status != Finished);
                        _selector.remove(r->second.fd, r->second.status);
                        r->second.status = Finished;
                        readyList.push_back(&(r->second));
                    }
                }
                _changes.pop_front();
            }
            else
            {

                //
                // Examine the selection key set.
                //
                SOCKET fd;
                while((fd = _selector.getNextSelected()) != INVALID_SOCKET)
                {
                    map<SOCKET, SocketInfo>::iterator r = socketMap.find(fd);
                    if(r != socketMap.end())
                    {
                        if(r->second.timeout >= 0)
                        {
                            _timer->cancel(r->second.cb);
                        }

                        readyList.push_back(&(r->second));
                    }
                }
            }
        }

        for(vector<SocketInfo*>::iterator p = readyList.begin(); p != readyList.end(); ++p)
        {
            SocketInfo* info = *p;
            SocketStatus status;
            try
            {
                status = info->cb->socketReady(info->status == Finished);
            }
            catch(const std::exception& ex)
            {
                Error out(_instance->initializationData().logger);
                out << "exception in selector thread while calling socketReady():\n" << ex.what();
                status = Finished;
            }
            catch(...)
            {
                Error out(_instance->initializationData().logger);
                out << "unknown exception in selector thread while calling socketReady()";
                status = Finished;
            }

            if(status == Finished)
            {
                finishedList.push_back(info);
            }
            else if(status != info->status)
            {
                assert(info->status != Finished);
                _selector.remove(info->fd, info->status);
                info->status = status;
                _selector.add(info->fd, info->status);
                if(info->timeout >= 0)
                {
                    _timer->schedule(info->cb, IceUtil::Time::milliSeconds(info->timeout));
                }
            }
        }

        readyList.clear();

        if(finishedList.empty())
        {
            continue;
        }

        for(vector<SocketInfo*>::const_iterator q = finishedList.begin(); q != finishedList.end(); ++q)
        {
            if((*q)->status != Finished)
            {
                _selector.remove((*q)->fd, (*q)->status);
            }
            socketMap.erase((*q)->fd);
        }
        finishedList.clear();
    }

    assert(_destroyed);
}

IceInternal::SelectorThread::HelperThread::HelperThread(const SelectorThreadPtr& selectorThread) :
    _selectorThread(selectorThread)
{
}

void
IceInternal::SelectorThread::HelperThread::run()
{
    if(_selectorThread->_instance->initializationData().threadHook)
    {
        _selectorThread->_instance->initializationData().threadHook->start();
    }

    try
    {
        _selectorThread->run();
    }
    catch(const std::exception& ex)
    {
        Error out(_selectorThread->_instance->initializationData().logger);
        out << "exception in selector thread:\n" << ex.what();
    }
    catch(...)
    {
        Error out(_selectorThread->_instance->initializationData().logger);
        out << "unknown exception in selector thread";
    }

    if(_selectorThread->_instance->initializationData().threadHook)
    {
        _selectorThread->_instance->initializationData().threadHook->stop();
    }

    _selectorThread = 0; // Break cyclic dependency.
}
