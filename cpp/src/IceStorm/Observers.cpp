// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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

namespace
{

class AMI_ReplicaObserver_createTopicI : public AMI_ReplicaObserver_createTopic, public AMICall
{
public:

    virtual void ice_response() { response(); }
    virtual void ice_exception(const Ice::Exception& e) { exception(e); }
};
typedef IceUtil::Handle<AMI_ReplicaObserver_createTopicI> AMI_ReplicaObserver_createTopicIPtr;

class AMI_ReplicaObserver_addSubscriberI : public AMI_ReplicaObserver_addSubscriber, public AMICall
{
public:

    virtual void ice_response() { response(); }
    virtual void ice_exception(const Ice::Exception& e) { exception(e); }
};
typedef IceUtil::Handle<AMI_ReplicaObserver_addSubscriberI> AMI_ReplicaObserver_addSubscriberIPtr;

class AMI_ReplicaObserver_removeSubscriberI : public AMI_ReplicaObserver_removeSubscriber, public AMICall
{
public:

    virtual void ice_response() { response(); }
    virtual void ice_exception(const Ice::Exception& e) { exception(e); }
};
typedef IceUtil::Handle<AMI_ReplicaObserver_removeSubscriberI> AMI_ReplicaObserver_removeSubscriberIPtr;

class AMI_ReplicaObserver_destroyTopicI : public AMI_ReplicaObserver_destroyTopic, public AMICall
{
public:

    virtual void ice_response() { response(); }
    virtual void ice_exception(const Ice::Exception& e) { exception(e); }
};
typedef IceUtil::Handle<AMI_ReplicaObserver_destroyTopicI> AMI_ReplicaObserver_destroyTopicIPtr;

}

AMICall::AMICall() :
    _response(false)
{
}

void
AMICall::response()
{
    Lock sync(*this);
    _response = true;
    notify();
}
void
AMICall::exception(const IceUtil::Exception& e)
{
    Lock sync(*this);
    _response = true;
    _ex.reset(e.ice_clone());
    notify();
}

void
AMICall::waitResponse()
{
    Lock sync(*this);
    while(!_response)
    {
        wait();
    }
    if(_ex.get())
    {
        _ex->ice_throw();
    }
}

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
    for(set<GroupNodeInfo>::const_iterator p = slaves.begin(); p != slaves.end(); ++p)
    {
        try
        {
            assert(p->observer);
            //ReplicaObserverPrx observer = ReplicaObserverPrx::uncheckedCast(p->observer);

            // 60s timeout for reliability in the event that a replica
            // becomes unresponsive.
            ReplicaObserverPrx observer = ReplicaObserverPrx::uncheckedCast(p->observer->ice_timeout(60 * 1000));
            observer->init(llu, content);
            _observers.push_back(ObserverInfo(p->id, observer));
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels->replication > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicationCat);
                out << "error calling init on " << p->id << " ex: " << ex;
            }
            throw;
        }
    }
}

void
Observers::createTopic(const LogUpdate& llu, const string& name)
{
    Lock sync(*this);
    for(vector<ObserverInfo>::iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        AMI_ReplicaObserver_createTopicIPtr cb = new AMI_ReplicaObserver_createTopicI;
        p->call = cb;
        p->observer->createTopic_async(cb, llu, name);
    }
    wait("createTopic");
}

void
Observers::destroyTopic(const LogUpdate& llu, const string& id)
{
    Lock sync(*this);
    for(vector<ObserverInfo>::iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        AMI_ReplicaObserver_destroyTopicIPtr cb = new AMI_ReplicaObserver_destroyTopicI;
        p->call = cb;
        p->observer->destroyTopic_async(cb, llu, id);
    }
    wait("destroyTopic");
}


void
Observers::addSubscriber(const LogUpdate& llu, const string& name, const SubscriberRecord& rec)
{
    Lock sync(*this);
    for(vector<ObserverInfo>::iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        AMI_ReplicaObserver_addSubscriberIPtr cb = new AMI_ReplicaObserver_addSubscriberI;
        p->call = cb;
        p->observer->addSubscriber_async(cb, llu, name, rec);
    }
    wait("addSubscriber");
}

void
Observers::removeSubscriber(const LogUpdate& llu, const string& name, const Ice::IdentitySeq& id)
{
    Lock sync(*this);
    for(vector<ObserverInfo>::iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        AMI_ReplicaObserver_removeSubscriberIPtr cb = new AMI_ReplicaObserver_removeSubscriberI;
        p->call = cb;
        p->observer->removeSubscriber_async(cb, llu, name, id);
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
            p->call->waitResponse();
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

            // COMPILERFIX: Just using following causes double unlock with C++Builder 2007
            //IceUtil::Mutex::Lock sync(_reapedMutex);
            _reapedMutex.lock();
            _reaped.push_back(id);
            _reapedMutex.unlock();
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

