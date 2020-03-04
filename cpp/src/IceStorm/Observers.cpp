//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceStorm/Observers.h>
#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>

using namespace std;
using namespace IceStorm;
using namespace IceStormElection;

Observers::Observers(shared_ptr<TraceLevels> traceLevels) :
    _traceLevels(move(traceLevels)),
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
    lock_guard<mutex> lg(_mutex);

    if(_observers.size() >= _majority)
    {
        auto p = _observers.begin();
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

                lock_guard<mutex> reapedLock(_reapedMutex);
                _reaped.push_back(id);
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
    lock_guard<mutex> lg(_mutex);
    _observers.clear();
}

void
Observers::getReapedSlaves(std::vector<int>& d)
{
    lock_guard<mutex> reapedLock(_reapedMutex);
    d.swap(_reaped);
}

void
Observers::init(const set<GroupNodeInfo>& slaves, const LogUpdate& llu, const TopicContentSeq& content)
{
    {
        lock_guard<mutex> reapedLock(_reapedMutex);
        _reaped.clear();
    }

    lock_guard<mutex> lg(_mutex);
    _observers.clear();

    vector<ObserverInfo> observers;

    for(const auto& slave : slaves)
    {
        try
        {
            assert(slave.observer);

            auto observer = Ice::uncheckedCast<ReplicaObserverPrx>(slave.observer);

            auto future = observer->initAsync(llu, content);

            observers.push_back({ slave.id, observer, move(future) });
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->replication > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
                out << "error calling init on " << slave.id << ", exception: " << ex;
            }
            throw;
        }
    }

    for(auto& o : observers)
    {
        try
        {
            o.future.get();
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->replication > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
                out << "init on " << o.id << " failed with exception " << ex;
            }
            throw;
        }
    }
    _observers = move(observers);
}

void
Observers::createTopic(const LogUpdate& llu, const string& name)
{
    lock_guard<mutex> lg(_mutex);
    for(auto& o : _observers)
    {
        o.future = o.observer->createTopicAsync(llu, name);
    }
    wait("createTopic");
}

void
Observers::destroyTopic(const LogUpdate& llu, const string& id)
{
    lock_guard<mutex> lg(_mutex);
    for(auto& o : _observers)
    {
        o.future = o.observer->destroyTopicAsync(llu, id);
    }
    wait("destroyTopic");
}

void
Observers::addSubscriber(const LogUpdate& llu, const string& name, const SubscriberRecord& rec)
{
    lock_guard<mutex> lg(_mutex);
    for(auto& o : _observers)
    {
        o.future = o.observer->addSubscriberAsync(llu, name, rec);
    }
    wait("addSubscriber");
}

void
Observers::removeSubscriber(const LogUpdate& llu, const string& name, const Ice::IdentitySeq& id)
{
    lock_guard<mutex> lg(_mutex);
    for(auto& o : _observers)
    {
        o.future = o.observer->removeSubscriberAsync(llu, name, id);
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
            p->future.get();
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

            lock_guard<mutex> reapedLock(_mutex);
            _reaped.push_back(id);
            continue;
        }
        ++p;
    }

    // If we now no longer have the majority of observers we raise.
    if(_observers.size() < _majority)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
        out << "number of observers `" << _observers.size() << "' is less than the majority '" << _majority << "'";
        throw Ice::UnknownException(__FILE__, __LINE__);
    }
}
