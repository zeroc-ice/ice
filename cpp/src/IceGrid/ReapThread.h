// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_REAPER_THREAD_H
#define ICEGRID_REAPER_THREAD_H

#include <IceUtil/Thread.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

#include <Ice/Logger.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>

#include <list>

namespace IceGrid
{

class Reapable : public IceUtil::Shared
{
public:

    virtual ~Reapable() { }

    virtual IceUtil::Time timestamp() const = 0;
    virtual void destroy(bool) = 0;

};
typedef IceUtil::Handle<Reapable> ReapablePtr;

template<class T>
class SessionReapable : public Reapable
{
    typedef IceUtil::Handle<T> TPtr;
    
public:
    
    SessionReapable(const Ice::LoggerPtr& logger, const TPtr& session) : 
        _logger(logger), _session(session)
    {
    }

    virtual ~SessionReapable()
    {
    }
        
    virtual IceUtil::Time
    timestamp() const
    {
        return _session->timestamp();
    }

    virtual void
    destroy(bool shutdown)
    {
        try
        {
            if(shutdown)
            {
                _session->shutdown();
            }
            else
            {
                _session->destroy(Ice::Current());
            }
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        catch(const Ice::LocalException& ex)
        {
            Ice::Warning out(_logger);
            out << "unexpected exception while reaping session:\n" << ex;
        }
    }

private:

    const Ice::LoggerPtr _logger;
    const TPtr _session;
};

class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReapThread();
    
    virtual void run();
    void terminate();
    void add(const ReapablePtr&, int);

private:

    bool calcWakeInterval();
    
    IceUtil::Time _wakeInterval;
    bool _terminated;
    struct ReapableItem
    {
        ReapablePtr item;
        IceUtil::Time timeout;
    };
    std::list<ReapableItem> _sessions;
};
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

};

#endif
