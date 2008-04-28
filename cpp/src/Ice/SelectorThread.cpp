// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

ICE_DECLSPEC_EXPORT IceUtil::Shared* IceInternal::upCast(SelectorThread* p) { return p; }

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
    Lock sync(*this);
    assert(!_destroyed);
    _destroyed = true;
    _selector.setInterrupt();
}

void
IceInternal::SelectorThread::incFdsInUse()
{
#ifdef ICE_USE_SELECT
    Lock sync(*this);
    _selector.incFdsInUse();
#endif
}

void
IceInternal::SelectorThread::decFdsInUse()
{
#ifdef ICE_USE_SELECT
    Lock sync(*this);
    _selector.decFdsInUse();
#endif
}

void
IceInternal::SelectorThread::_register(SOCKET fd, const SocketReadyCallbackPtr& cb, SocketStatus status, int timeout)
{
    Lock sync(*this);
    assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
    assert(status != Finished);
    assert(cb->_status == Finished);

    cb->_fd = fd;
    cb->_status = status;
    cb->_timeout = timeout;
    if(cb->_timeout >= 0)
    {
        _timer->schedule(cb, IceUtil::Time::milliSeconds(cb->_timeout));
    }

    _selector.add(cb.get(), cb->_status);
}

void
IceInternal::SelectorThread::unregister(const SocketReadyCallbackPtr& cb)
{
    // Note: unregister should only be called from the socketReady() call-back.
    Lock sync(*this);
    assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
    assert(cb->_fd != INVALID_SOCKET);
    assert(cb->_status != Finished);

    _selector.remove(cb.get(), cb->_status, true); // No interrupt needed, it's always called from the selector thread.
    cb->_status = Finished;
}

void
IceInternal::SelectorThread::finish(const SocketReadyCallbackPtr& cb)
{
    Lock sync(*this);
    assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
    assert(cb->_fd != INVALID_SOCKET);
    assert(cb->_status != Finished);

    _selector.remove(cb.get(), cb->_status);
    cb->_status = Finished;

    _finished.push_back(cb);
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

        vector<pair<SocketReadyCallbackPtr, SocketStatus> > readyList;
        bool finished = false;
    
        {
            Lock sync(*this);
            if(_selector.isInterrupted())
            {
                if(_selector.processInterrupt())
                {
                    continue;
                }

                //
                // There are two possiblities for an interrupt:
                //
                // 1. The selector thread has been destroyed.
                // 2. A callback is being finished (closed).
                //
                
                //
                // Thread destroyed?
                //
                if(_destroyed)
                {
                    break;
                }
                
                do
                {
                    SocketReadyCallbackPtr cb = _finished.front();
                    _finished.pop_front();
                    readyList.push_back(make_pair(cb, Finished));
                }
                while(_selector.clearInterrupt()); // As long as there are interrupts.
                finished = true;
            }
            else
            {
                //
                // Examine the selection key set.
                //
                SocketReadyCallbackPtr cb;
                while(cb = _selector.getNextSelected())
                {
                    readyList.push_back(make_pair(cb, cb->_status));
                }
            }
        }

        vector<pair<SocketReadyCallbackPtr, SocketStatus> >::const_iterator p;
        for(p = readyList.begin(); p != readyList.end(); ++p)
        {
            const SocketReadyCallbackPtr cb = p->first;
            const SocketStatus previousStatus = p->second;
            SocketStatus status = Finished;
            try
            {
                if(cb->_timeout >= 0)
                {
                    _timer->cancel(cb);
                }

                if(finished)
                {
                    cb->socketFinished();
                }
                else
                {
                    status = cb->socketReady();
                }
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

            if(status != Finished)
            {
                if(status != previousStatus)
                {
                    Lock sync(*this);
                    if(cb->_status != Finished) // The callback might have been finished concurrently.
                    {
                        _selector.update(cb.get(), cb->_status, status);
                        cb->_status = status;
                    }
                }

                if(cb->_timeout >= 0)
                {
                    _timer->schedule(cb, IceUtil::Time::milliSeconds(cb->_timeout));
                }
            }
        }
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

    _selectorThread = 0; // Break cyclic dependency.
}
