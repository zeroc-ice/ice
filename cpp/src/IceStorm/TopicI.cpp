// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceStorm/TopicI.h>
#include <IceStorm/Instance.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/SubscriberPool.h>

#include <Ice/LoggerUtil.h>

#include <Freeze/Initialize.h>

#include <algorithm>

using namespace IceStorm;
using namespace std;

namespace
{

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from Publishers.
//
class PublisherI : public Ice::BlobjectArray
{
public:

    PublisherI(const TopicIPtr& topic) :
        _topic(topic)
    {
    }

    virtual bool
    ice_invoke(const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
               Ice::ByteSeq&,
               const Ice::Current& current)
    {
        EventDataPtr event = new EventData(
            current.operation,
            current.mode,
            Ice::ByteSeq(),
            current.ctx);

        //
        // COMPILERBUG: gcc 4.0.1 doesn't like this.
        //
        //event->data.swap(Ice::ByteSeq(inParams.first, inParams.second));
        Ice::ByteSeq data(inParams.first, inParams.second);
        event->data.swap(data);
        
        EventDataSeq v;
        v.push_back(event);
        _topic->publish(false, v);

        return true;
    }

private:
    
    const TopicIPtr _topic;
};

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from linked Topics.
//
class TopicLinkI : public TopicLink
{
public:

    TopicLinkI(const TopicIPtr& topic) :
        _topic(topic)
    {
    }

    virtual void
    forward(const EventDataSeq& v, const Ice::Current& current)
    {
        _topic->publish(true, v);
    }

private:

    const TopicIPtr _topic;
};

}
namespace IceStorm
{
extern string identityToTopicName(const Ice::Identity& id);
}

TopicI::TopicI(
    const InstancePtr& instance,
    const string& name,
    const Ice::Identity& id,
    const LinkRecordSeq& topicRecord,
    const string& envName,
    const string& dbName) :
    _instance(instance),
    _name(name),
    _id(id),
    _connection(Freeze::createConnection(instance->communicator(), envName)),
    _topics(_connection, dbName, false),
    _topicRecord(topicRecord),
    _destroyed(false)
{
    //
    // Create a servant per topic to receive event data. If the
    // category is empty then we are in backwards compatibility
    // mode. In this case the servant's identity is
    // category=<topicname>, name=publish, otherwise the name is
    // <instancename>/publisher.<topicname>. The same applies to the
    // link proxy.
    //
    // Activate the object and save a reference to give to publishers.
    //
    Ice::Identity pubid;
    Ice::Identity linkid;
    if(id.category.empty())
    {
        pubid.category = _name;
        pubid.name = "publish";
        linkid.category = _name;
        linkid.name = "link";
    }
    else
    {
        pubid.category = id.category;
        pubid.name = _name + ".publish";
        linkid.category = id.category;
        linkid.name = _name + ".link";
    }

    _publisherPrx = _instance->objectAdapter()->add(new PublisherI(this), pubid);
    _linkPrx = TopicLinkPrx::uncheckedCast(_instance->objectAdapter()->add(new TopicLinkI(this), linkid));

    //
    // Re-establish linked subscribers.
    //
    for(LinkRecordSeq::const_iterator p = _topicRecord.begin(); p != _topicRecord.end(); ++p)
    {
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << " relink " << _instance->communicator()->identityToString(p->theTopic->ice_getIdentity());
        }

        //
        // Create the subscriber object add it to the set of
        // subscribers.
        //
        SubscriberPtr subscriber = Subscriber::create(_instance, p->obj, p->cost);
        _subscribers.push_back(subscriber);
        _instance->subscriberPool()->add(subscriber);
    }
}

TopicI::~TopicI()
{
}

string
TopicI::getName(const Ice::Current&) const
{
    // Immutable
    return _name;
}

Ice::ObjectPrx
TopicI::getPublisher(const Ice::Current&) const
{
    // Immutable
    return _publisherPrx;
}

//
// COMPILERFIX: For some reason with VC6 find reports an error.
//
#if defined(_MSC_VER) && (_MSC_VER < 1300)
vector<SubscriberPtr>::iterator
find(vector<SubscriberPtr>::iterator start, vector<SubscriberPtr>::iterator end, const Ice::Identity& ident)
{
    while(start != end)
    {
        if(*start == ident)
        {
            return start;
        }
        ++start;
    }
    return end;
}
#endif

void
TopicI::subscribe(const QoS& origQoS, const Ice::ObjectPrx& obj, const Ice::Current&)
{
    Ice::Identity id = obj->ice_getIdentity();
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    QoS qos = origQoS;
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << "Subscribe: " << _instance->communicator()->identityToString(id);
        if(traceLevels->topic > 1)
        {
            out << " QoS: ";
            for(QoS::const_iterator p = qos.begin(); p != qos.end() ; ++p)
            {
                if(p != qos.begin())
                {
                    out << ',';
                }
                out << '[' << p->first << "," << p->second << ']';
            }
        }
    }


    string reliability = "oneway";
    {
        QoS::iterator p = qos.find("reliability");
        if(p != qos.end())
        {
            reliability = p->second;
            qos.erase(p);
        }
    }

    Ice::ObjectPrx newObj = obj;
    if(reliability == "batch")
    {
        if(newObj->ice_isDatagram())
        {
            newObj = newObj->ice_batchDatagram();
        }
        else
        {
            newObj = newObj->ice_batchOneway();
        }
    }
    else if(reliability == "twoway")
    {
        newObj = newObj->ice_twoway();
    }
    else if(reliability == "twoway ordered")
    {
        qos["reliability"] = "ordered";
        newObj = newObj->ice_twoway();
    }
    else // reliability == "oneway"
    {
        if(reliability != "oneway" && traceLevels->subscriber > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
            out << reliability <<" mode not understood.";
        }
        if(!newObj->ice_isDatagram())
        {
            newObj = newObj->ice_oneway();
        }
    }

    IceUtil::Mutex::Lock sync(_subscribersMutex);
    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p != _subscribers.end())
    {
        (*p)->destroy();
        _instance->subscriberPool()->remove(*p);
        _subscribers.erase(p);
    }

    SubscriberPtr subscriber = Subscriber::create(_instance, newObj, qos);
    _subscribers.push_back(subscriber);
    _instance->subscriberPool()->add(subscriber);
}

Ice::ObjectPrx
TopicI::subscribeAndGetPublisher(const QoS& qos, const Ice::ObjectPrx& obj, const Ice::Current&)
{
    Ice::Identity id = obj->ice_getIdentity();
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << "Subscribe: " << _instance->communicator()->identityToString(id);
        if(traceLevels->topic > 1)
        {
            out << " QoS: ";
            for(QoS::const_iterator p = qos.begin(); p != qos.end() ; ++p)
            {
                if(p != qos.begin())
                {
                    out << ',';
                }
                out << '[' << p->first << "," << p->second << ']';
            }
        }
    }

    IceUtil::Mutex::Lock sync(_subscribersMutex);
    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p != _subscribers.end())
    {
        throw AlreadySubscribed();
    }

    SubscriberPtr subscriber = Subscriber::create(_instance, obj, qos);
    _subscribers.push_back(subscriber);
    _instance->subscriberPool()->add(subscriber);

    return subscriber->proxy();
}

void
TopicI::unsubscribe(const Ice::ObjectPrx& subscriber, const Ice::Current&)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(!subscriber)
    {
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << "unsubscribe with null subscriber.";
        }
        return;
    }

    Ice::Identity id = subscriber->ice_getIdentity();

    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << "Unsubscribe: " << _instance->communicator()->identityToString(id);
    }

    //
    // Unsubscribe the subscriber with this identity.
    //
    removeSubscriber(subscriber);
}

TopicLinkPrx
TopicI::getLinkProxy(const Ice::Current&)
{
    // immutable
    return _linkPrx;
}

void
TopicI::link(const TopicPrx& topic, Ice::Int cost, const Ice::Current&)
{
    TopicInternalPrx internal = TopicInternalPrx::uncheckedCast(topic);
    TopicLinkPrx link = internal->getLinkProxy();

    IceUtil::RecMutex::Lock topicSync(_topicRecordMutex);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    Ice::Identity id = topic->ice_getIdentity();
    string name = identityToTopicName(id);

    // Validate that this topic doesn't already have an established
    // link.
    for(LinkRecordSeq::const_iterator p = _topicRecord.begin(); p != _topicRecord.end(); ++p)
    {
        if(p->theTopic->ice_getIdentity() == topic->ice_getIdentity())
        {
            LinkExists ex;
            ex.name = name;
            throw ex;
        }
    }
    
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << " link " << _instance->communicator()->identityToString(id)
            << " cost " << cost;
    }

    SubscriberPtr subscriber = Subscriber::create(_instance, link, cost);

    //
    // Create the LinkRecord
    //
    LinkRecord record;
    record.obj = link;
    record.cost = cost;
    record.theTopic = topic;
    
    //
    // Save
    //
    _topicRecord.push_back(record);
    _topics.put(PersistentTopicMap::value_type(_id, _topicRecord));
    
    IceUtil::Mutex::Lock subscriberSync(_subscribersMutex);
    _subscribers.push_back(subscriber);
    _instance->subscriberPool()->add(subscriber);
}

void
TopicI::unlink(const TopicPrx& topic, const Ice::Current& current)
{
    IceUtil::RecMutex::Lock topicSync(_topicRecordMutex);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    Ice::Identity id = topic->ice_getIdentity();
    string name = identityToTopicName(id);
    
    LinkRecordSeq::iterator p = _topicRecord.begin();
    while(p != _topicRecord.end())
    {
        if(p->theTopic->ice_getIdentity() == topic->ice_getIdentity())
        {
            break;
        }
        ++p;
    }
    if(p == _topicRecord.end())
    {
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << " unlink " << name << " failed - not linked";
        }

        NoSuchLink ex;
        ex.name = name;
        throw ex;
    }

    Ice::ObjectPrx subscriber = p->obj;
    _topicRecord.erase(p);

    //
    // Save
    //
    _topics.put(PersistentTopicMap::value_type(_id, _topicRecord));

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << " unlink " << _instance->communicator()->identityToString(id);
    }
    removeSubscriber(subscriber);
}

LinkInfoSeq
TopicI::getLinkInfoSeq(const Ice::Current&) const
{
    IceUtil::RecMutex::Lock topicSync(_topicRecordMutex);
    TopicI* This = const_cast<TopicI*>(this);
    This->reap();
    
    LinkInfoSeq seq;
    
    for(LinkRecordSeq::const_iterator q = _topicRecord.begin(); q != _topicRecord.end(); ++q)
    {
        LinkInfo info;
        info.name = identityToTopicName(q->theTopic->ice_getIdentity());
        info.cost = q->cost;
        info.theTopic = q->theTopic;
        seq.push_back(info);
    }
    
    return seq;
}

void
TopicI::destroy(const Ice::Current&)
{
    IceUtil::RecMutex::Lock sync(_topicRecordMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    _destroyed = true;

    try
    {
        _instance->objectAdapter()->remove(_linkPrx->ice_getIdentity());
        _instance->objectAdapter()->remove(_publisherPrx->ice_getIdentity());
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // Ignore -- this could occur on shutdown.
    }
}

bool
TopicI::destroyed() const
{
    IceUtil::RecMutex::Lock sync(_topicRecordMutex);
    return _destroyed;
}

Ice::Identity
TopicI::id() const
{
    // immutable
    return _id;
}

void
TopicI::reap()
{
    IceUtil::RecMutex::Lock topicSync(_topicRecordMutex);
    if(_destroyed)
    {
        return;
    }
    bool updated = false;

    //
    // Run through all invalid subscribers and remove them from the
    // database.
    //
    list<SubscriberPtr> error;
    {
        IceUtil::Mutex::Lock errorSync(_errorMutex);
        _error.swap(error);
    }

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    for(list<SubscriberPtr>::const_iterator p = error.begin(); p != error.end(); ++p)
    {
        SubscriberPtr subscriber = *p;
        assert(subscriber->persistent()); // Only persistent subscribers need to be reaped.

        bool found = false;
        //
        // If this turns out to be a performance problem then we
        // can create an in memory map cache.
        //
        LinkRecordSeq::iterator q = _topicRecord.begin();
        while(q != _topicRecord.end())
        {
            if(q->obj->ice_getIdentity() == subscriber->id())
            {
                _topicRecord.erase(q);
                updated = true;
                found = true;
                break;
            }
            ++q;
        }
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << "reaping " << _instance->communicator()->identityToString(subscriber->id());
            if(!found)
            {
                out << ": failed - not in database";
            }
        }
    }
    if(updated)
    {
        _topics.put(PersistentTopicMap::value_type(_id, _topicRecord));
    }
}

void
TopicI::publish(bool forwarded, const EventDataSeq& events)
{
    //
    // Copy of the subscriber list so that event publishing can occur
    // in parallel.
    //
    vector<SubscriberPtr> copy;
    {
        IceUtil::Mutex::Lock sync(_subscribersMutex);
        copy = _subscribers;
    }

    //
    // Queue each event. This results in two lists -- one the list of
    // subscribers in error and the second a list of subscribers that
    // need to be flushed.
    //
    vector<Ice::Identity> e;
    list<SubscriberPtr> flush;
    for(vector<SubscriberPtr>::const_iterator p = copy.begin(); p != copy.end(); ++p)
    {
        Subscriber::QueueState state = (*p)->queue(forwarded, events);
        switch(state)
        {
        case Subscriber::QueueStateError:
            e.push_back((*p)->id());
            break;
        case Subscriber::QueueStateFlush:
            flush.push_back(*p);
            break;
        case Subscriber::QueueStateNoFlush:
            break;
        }
    }

    //
    // Now we add each subscriber to be flushed to the flush manager.
    //
    if(!flush.empty())
    {
        _instance->subscriberPool()->flush(flush);
    }

    //
    // Run through the error list removing those subscribers that are
    // in error from the subscriber list.
    //
    list<SubscriberPtr> reap;
    if(!e.empty())
    {
        IceUtil::Mutex::Lock sync(_subscribersMutex);
        for(vector<Ice::Identity>::const_iterator ep = e.begin(); ep != e.end(); ++ep)
        {
            //
            // Its possible for the subscriber to already have been
            // removed since the copy is iterated over outside of
            // mutex protection.
            //
            // Note that although this could be quicker if we used a
            // map, the most optimal case should be pushing around
            // events not searching for a particular subscriber.
            //
            // The subscriber is immediately destroyed & removed from
            // the _subscribers list. If the subscriber is persistent
            // its added to an list of error'd subscribers and removed
            // from the database on the next reap.
            //
            vector<SubscriberPtr>::iterator q = find(_subscribers.begin(), _subscribers.end(), *ep);
            if(q != _subscribers.end())
            {
                //
                // Destroy the subscriber in any case.
                //
                (*q)->destroy();
                if((*q)->persistent())
                {
                    reap.push_back(*q);
                }
                _instance->subscriberPool()->remove(*q);
                _subscribers.erase(q);
            }
        }
    }
    
    if(!reap.empty())
    {
        //
        // This is why _error is a list, so we can splice on the
        // reaped subscribers.
        //
        IceUtil::Mutex::Lock errorSync(_errorMutex);
        _error.splice(_error.begin(), reap);
    }
}

void
TopicI::removeSubscriber(const Ice::ObjectPrx& obj)
{
    Ice::Identity id = obj->ice_getIdentity();
    
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p != _subscribers.end())
    {
        (*p)->destroy();
        _instance->subscriberPool()->remove(*p);
        _subscribers.erase(p);
        return;
    }
    
    //
    // If the subscriber was not found then display a diagnostic.
    //
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _instance->communicator()->identityToString(id) << ": not subscribed.";
    }
}
