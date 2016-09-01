// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/Observers.h>
#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>

using namespace std;
using namespace IceStorm;
using namespace IceStormElection;

Observers::Observers(const InstancePtr& instance) :
    _traceLevels(instance->traceLevels()),
    _majority(0)
{
}

void
Observers::setMajority(unsigned int majority)
{
    _majority = majority;
}

bool
Observers::check()
{
    Lock sync(*this);
    if(_observers.size() >= _majority)
    {
        vector<ObserverInfo>::iterator p = _observers.begin();
        while(p != _observers.end())
        {
            try
            {
                p->observer->ice_ping();
            }
            catch(const Ice::Exception& ex)
            {
                if(_traceLevels->replication > 0)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
                    out << "ice_ping failed: " << ex;
                }
                int id = p->id;
                p = _observers.erase(p);

                // COMPILERFIX: Just using following causes double unlock with C++Builder 2007
                //IceUtil::Mutex::Lock sync(_reapedMutex);
                _reapedMutex.lock();
                _reaped.push_back(id);
                _reapedMutex.unlock();
                continue;
            }
            ++p;
        }
    }
    return _majority == 0 || _observers.size() >= _majority;
}

void
Observers::clear()
{
    Lock sync(*this);
    _observers.clear();
}

void
Observers::getReapedSlaves(std::vector<int>& d)
{
    IceUtil::Mutex::Lock sync(_reapedMutex);
    d.swap(_reaped);
}

void
Observers::init(const set<GroupNodeInfo>& slaves, const LogUpdate& llu, const TopicContentSeq& content)
{
    {
        IceUtil::Mutex::Lock sync(_reapedMutex);
        _reaped.clear();
    }

    Lock sync(*this);
    _observers.clear();

    vector<ObserverInfo> observers;

    for(set<GroupNodeInfo>::const_iterator p = slaves.begin(); p != slaves.end(); ++p)
    {
        try
        {
            assert(p->observer);

            // 60s timeout for reliability in the event that a replica becomes unresponsive.
            ReplicaObserverPrx observer = ReplicaObserverPrx::uncheckedCast(p->observer->ice_timeout(60 * 1000));

            Ice::AsyncResultPtr result = observer->begin_init(llu, content);
            observers.push_back(ObserverInfo(p->id, observer, result));
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->replication > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
                out << "error calling init on " << p->id << ", exception: " << ex;
            }
            throw;
        }
    }

    for(vector<ObserverInfo>::iterator p = observers.begin(); p != observers.end(); ++p)
    {
        try
        {
            p->observer->end_init(p->result);
            p->result = 0;
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->replication > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
                out << "init on " << p->id << " failed with exception " << ex;
            }
            throw;
        }
    }

    _observers.swap(observers);
}

void
Observers::createTopic(const LogUpdate& llu, const string& name)
{
    Lock sync(*this);
    for(vector<ObserverInfo>::iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        p->result = p->observer->begin_createTopic(llu, name);
    }
    wait("createTopic");
}

void
Observers::destroyTopic(const LogUpdate& llu, const string& id)
{
    Lock sync(*this);
    for(vector<ObserverInfo>::iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        p->result = p->observer->begin_destroyTopic(llu, id);
    }
    wait("destroyTopic");
}


void
Observers::addSubscriber(const LogUpdate& llu, const string& name, const SubscriberRecord& rec)
{
    Lock sync(*this);
    for(vector<ObserverInfo>::iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        p->result = p->observer->begin_addSubscriber(llu, name, rec);
    }
    wait("addSubscriber");
}

void
Observers::removeSubscriber(const LogUpdate& llu, const string& name, const Ice::IdentitySeq& id)
{
    Lock sync(*this);
    for(vector<ObserverInfo>::iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        p->result = p->observer->begin_removeSubscriber(llu, name, id);
    }
    wait("removeSubscriber");
}

void
Observers::wait(const string& op)
{
    vector<ObserverInfo>::iterator p = _observers.begin();
    while(p != _observers.end())
    {
        try
        {
            p->result->waitForCompleted();
            p->result->throwLocalException();
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->replication > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
                out << op << ": " << ex;
            }
            int id = p->id;
            p = _observers.erase(p);

            IceUtil::Mutex::Lock sync(_reapedMutex);
            _reaped.push_back(id);
            continue;
        }
        ++p;
    }
    // If we now no longer have the majority of observers we raise.
    if(_observers.size() < _majority)
    {
        // TODO: Trace here?
        //Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
        //out << op;
        throw Ice::UnknownException(__FILE__, __LINE__);
    }
}
