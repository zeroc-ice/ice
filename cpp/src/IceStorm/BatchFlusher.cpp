// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/BatchFlusher.h>
#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Connection.h>

#include <set>

using namespace IceStorm;
using namespace std;

BatchFlusher::BatchFlusher(const InstancePtr& instance) :
    _traceLevels(instance->traceLevels()),
    _flushTime(IceUtil::Time::milliSeconds(
                   max(instance->properties()->getPropertyAsIntWithDefault(
                           "IceStorm.Flush.Timeout", 1000), 100))), // Minimum of 100ms.
    _destroy(false)
{
    start();
}

BatchFlusher::~BatchFlusher()
{
}

void
BatchFlusher::add(const Ice::ObjectPrx& subscriber)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    //
    // If the set of subscribers is empty then wake up the flushing
    // thread since it will be waiting indefinitely
    //
    if(_subscribers.empty())
    {
        notify();
    }
    _subscribers.push_back(subscriber);
}

void
BatchFlusher::remove(const Ice::ObjectPrx& subscriber)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _subscribers.remove(subscriber);
}

void
BatchFlusher::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _destroy = true;
    notify();
}

void
BatchFlusher::run()
{
    for(;;)
    {
        list<Ice::ObjectPrx> subscribers;
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
            if(_destroy)
            {
                return;
            }
            if(_subscribers.empty())
            {
                wait();
            }
            else
            {
                timedWait(_flushTime);
            }
            if(_destroy)
            {
                return;
            }
            subscribers = _subscribers;
        }

        set<Ice::ConnectionPtr> flushSet;
        for(list<Ice::ObjectPrx>::const_iterator p = subscribers.begin(); p != subscribers.end(); ++p)
        {
            Ice::ConnectionPtr connection = (*p)->ice_getCachedConnection();
            if(connection)
            {
                flushSet.insert(connection);
            }
        }
        
        for(set<Ice::ConnectionPtr>::const_iterator q = flushSet.begin(); q != flushSet.end(); ++q)
        {
            try
            {
                (*q)->flushBatchRequests();
            }
            catch(const Ice::LocalException&)
            {
                // Ignore.
            }
        }
        
        //
        // Trace after the flush so that the correct number of objects
        // are displayed
        //
        if(_traceLevels->flush > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->flushCat);
            out << "connections: " << flushSet.size() << " subscribers: " << subscribers.size();
        }
    }
}
