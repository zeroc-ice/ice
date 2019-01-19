//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_REAPER_THREAD_H
#define ICEGRID_REAPER_THREAD_H

#include <IceUtil/Thread.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

#include <Ice/Logger.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Connection.h>

#include <list>

namespace IceGrid
{

class Reapable : public Ice::LocalObject
{
public:

    virtual ~Reapable() { }

    virtual void heartbeat() const { };

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

protected:

    const Ice::LoggerPtr _logger;
    const TPtr _session;
};

template<class T>
class SessionReapableWithHeartbeat : public SessionReapable<T>
{
    typedef IceUtil::Handle<T> TPtr;

public:

    SessionReapableWithHeartbeat(const Ice::LoggerPtr& logger, const TPtr& session) :
        SessionReapable<T>(logger, session)
    {
    }

    virtual void
    heartbeat() const
    {
        try
        {
            SessionReapable<T>::_session->keepAlive(Ice::Current());
        }
        catch(Ice::Exception&)
        {
        }
    }
};

class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReapThread();

    virtual void run();
    void terminate();
    void add(const ReapablePtr&, int, const Ice::ConnectionPtr& = Ice::ConnectionPtr());

    void connectionHeartbeat(const Ice::ConnectionPtr&);
    void connectionClosed(const Ice::ConnectionPtr&);

private:

    bool calcWakeInterval();

    Ice::CloseCallbackPtr _closeCallback;
    Ice::HeartbeatCallbackPtr _heartbeatCallback;
    IceUtil::Time _wakeInterval;
    bool _terminated;
    struct ReapableItem
    {
        ReapablePtr item;
        Ice::ConnectionPtr connection;
        IceUtil::Time timeout;
    };
    std::list<ReapableItem> _sessions;

    std::map<Ice::ConnectionPtr, std::set<ReapablePtr> > _connections;
};
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

};

#endif
