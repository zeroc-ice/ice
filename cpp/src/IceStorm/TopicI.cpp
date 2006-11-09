// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/TopicI.h>
#include <IceStorm/Instance.h>
#include <IceStorm/Subscribers.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Event.h>
#include <IceStorm/KeepAliveThread.h>
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
	EventPtr event = new Event(
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
	
	EventSeq v;
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
    forward(const EventSeq& v, const Ice::Current& current)
    {
	_topic->publish(true, v);
    }

private:

    const TopicIPtr _topic;
};

class TopicUpstreamLinkI : public TopicUpstreamLink
{
public:

    TopicUpstreamLinkI(const SubscriberPtr& subscriber) :
	_subscriber(subscriber)
    {
    }

    virtual void
    keepAlive(const Ice::Current&)
    {
	_subscriber->reachable();
    }

private:

    const SubscriberPtr _subscriber;
};

}

TopicI::TopicI(
    const InstancePtr& instance,
    const string& name,
    const LinkRecordDict& topicRecord,
    const string& envName,
    const string& dbName) :
    _instance(instance),
    _name(name),
    _connection(Freeze::createConnection(instance->communicator(), envName)),
    _topics(_connection, dbName, false),
    _topicRecord(topicRecord),
    _upstream(_connection, "upstream", false),
    _destroyed(false)
{
    //
    // Create a servant per topic to receive event data. The servant's
    // identity is category=<topicname>, name=publish. Activate the
    // object and save a reference to give to publishers.
    //
    Ice::Identity id;
    id.category = _name;
    id.name = "publish";
    _publisherPrx = _instance->objectAdapter()->add(new PublisherI(this), id);

    //
    // Create a servant per topic to receive linked event data. The
    // servant's identity is category=<topicname>, name=link. Activate
    // the object and save a reference to give to linked topics.
    //
    id.name = "link";
    _linkPrx = TopicLinkPrx::uncheckedCast(_instance->objectAdapter()->add(new TopicLinkI(this), id));

    //
    // Re-establish linked subscribers.
    //
    for(LinkRecordDict::const_iterator p = _topicRecord.begin(); p != _topicRecord.end(); ++p)
    {
	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(traceLevels->topic > 0)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	    out << _name << " relink " << p->first;
	}

	//
	// Create the subscriber object and the upstream servant and
	// add it to the set of subscribers.
	//
	SubscriberPtr subscriber = Subscriber::create(_instance, p->second.obj, p->second.cost);
	TopicUpstreamLinkPrx upstream = TopicUpstreamLinkPrx::uncheckedCast(
	    _instance->objectAdapter()->add(
		new TopicUpstreamLinkI(subscriber), p->second.upstream->ice_getIdentity()));
	_subscribers.push_back(subscriber);
	_instance->subscriberPool()->add(subscriber);
    }

    PersistentUpstreamMap::const_iterator upI = _upstream.find(_name);
    if(upI != _upstream.end())
    {
	//
	// This record should really be there, but its possible for it
	// not to be in the event of a crash in between the add of the
	// topic record and the add of the upstream record.
	//
	_upstreamRecord = upI->second;
    }

    for(TopicUpstreamLinkPrxSeq::const_iterator q = _upstreamRecord.begin(); q != _upstreamRecord.end(); ++q)
    {
	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(traceLevels->topic > 0)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	    out << _name << " upstream " << _instance->communicator()->identityToString((*q)->ice_getIdentity());
	}
	_instance->keepAlive()->add(*q);
    }
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
TopicI::subscribe(const QoS& qos, const Ice::ObjectPrx& obj, const Ice::Current& current)
{
    Ice::Identity ident = obj->ice_getIdentity();
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	out << "Subscribe: " << _instance->communicator()->identityToString(ident);
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
    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), ident);
    if(p != _subscribers.end())
    {
	(*p)->destroy();
	_instance->subscriberPool()->remove(*p);
	_subscribers.erase(p);
    }

    SubscriberPtr subscriber = Subscriber::create(_instance, obj, qos);
    _subscribers.push_back(subscriber);
    _instance->subscriberPool()->add(subscriber);
}

Ice::ObjectPrx
TopicI::subscribeAndGetPublisher(const QoS& qos, const Ice::ObjectPrx& obj, const Ice::Current&)
{
    Ice::Identity ident = obj->ice_getIdentity();
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	out << "Subscribe: " << _instance->communicator()->identityToString(ident);
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
    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), ident);
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

    Ice::Identity ident = subscriber->ice_getIdentity();

    if(traceLevels->topic > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	out << "Unsubscribe: " << _instance->communicator()->identityToString(ident);
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
    string name = topic->getName();
    TopicInternalPrx internal = TopicInternalPrx::checkedCast(topic);
    TopicLinkPrx link = internal->getLinkProxy();

    IceUtil::RecMutex::Lock topicSync(_topicRecordMutex);
    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    if(_topicRecord.find(name) != _topicRecord.end())
    {
	LinkExists ex;
	ex.name = name;
	throw ex;
    }
    
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	out << _name << " link " << name << " cost " << cost;
    }

    SubscriberPtr subscriber = Subscriber::create(_instance, link, cost);
    TopicUpstreamLinkPrx upstream;
    try
    {
	upstream = TopicUpstreamLinkPrx::uncheckedCast(
	    _instance->objectAdapter()->addWithUUID(new TopicUpstreamLinkI(subscriber)));
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
	subscriber->destroy();
	throw;
    }

    //
    // Notify the downstream topic that it is now linked. For linking
    // we use the "notify & save" strategy. This is important because
    // if there is a failure after the notify and before the save then
    // the downstream service will detect that the topic upstream link
    // does not exist and correctly clean up. If we saved and then
    // notified we could have a downstream linked client that does not
    // know it is in fact linked -- and this is not easily detectable.
    //
    try
    {
	internal->linkNotification(_name, upstream);
    }
    catch(const Ice::Exception&)
    {
	// Cleanup.
	_instance->objectAdapter()->remove(upstream->ice_getIdentity());
	subscriber->destroy();
	throw;
    }
    
    //
    // Create the LinkRecord
    //
    LinkRecord record;
    record.obj = link;
    record.cost = cost;
    record.theTopic = topic;
    record.upstream = upstream;
    
    //
    // Save
    //
    _topicRecord.insert(LinkRecordDict::value_type(name, record));
    _topics.put(PersistentTopicMap::value_type(_name, _topicRecord));
    
    IceUtil::Mutex::Lock subscriberSync(_subscribersMutex);
    _subscribers.push_back(subscriber);
    _instance->subscriberPool()->add(subscriber);
}

void
TopicI::unlink(const TopicPrx& topic, const Ice::Current& current)
{
    unlinkByName(topic->getName(), current);
}

void
TopicI::unlinkByName(const string& name, const Ice::Current&)
{
    IceUtil::RecMutex::Lock topicSync(_topicRecordMutex);
    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();
    
    LinkRecordDict::iterator q = _topicRecord.find(name);
    if(q == _topicRecord.end())
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

    //
    // First save and then notify.  unlinking we use the "save &
    // notify" strategy. This is important because if there is a
    // failure after the save and before the notify then the
    // downstream service will detect that the topic upstream link
    // does not exist and correctly clean up.
    //

    //
    // Copy the record first because we use it after the save.
    //
    LinkRecord rec = q->second;
    _topicRecord.erase(q);

    //
    // Save
    //
    _topics.put(PersistentTopicMap::value_type(_name, _topicRecord));

    //
    // Remove the TopicUpstreamLink servant.
    //
    try
    {
	_instance->objectAdapter()->remove(rec.upstream->ice_getIdentity());
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
	// Ignore -- this could occur on shutdown.
    }

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	out << _name << " unlink " << name;
    }
    removeSubscriber(rec.obj);

    TopicInternalPrx internal = TopicInternalPrx::checkedCast(rec.theTopic);
    try
    {
	internal->unlinkNotification(_name, rec.upstream);
    }
    catch(const Ice::Exception& e)
    {
	if(traceLevels->topic > 0)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	    out << _name << " unlinkNotification failed: " << name << ": " << e;
	}
	// Ignore. This will be detected upon a restart.
    }
}

LinkInfoSeq
TopicI::getLinkInfoSeq(const Ice::Current&) const
{
    IceUtil::RecMutex::Lock topicSync(_topicRecordMutex);
    TopicI* This = const_cast<TopicI*>(this);
    This->reap();
    
    LinkInfoSeq seq;
    
    for(LinkRecordDict::const_iterator q = _topicRecord.begin(); q != _topicRecord.end(); ++q)
    {
	LinkInfo info;
	info.name = q->first;
	info.cost = q->second.cost;
	info.theTopic = q->second.theTopic;
	seq.push_back(info);
    }
    
    return seq;
}

void
TopicI::destroy(const Ice::Current&)
{
    //
    // This method must lock both the record mutexes otherwise we can
    // end up with the database record being re-added if
    // TopicManagerI::reap() is called concurrently with destroy/link.
    //
    IceUtil::RecMutex::Lock sync(_topicRecordMutex);
    IceUtil::RecMutex::Lock sync2(_upstreamRecordMutex);

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

void
TopicI::linkNotification(const string& name, const TopicUpstreamLinkPrx& upstream, const Ice::Current&)
{
    IceUtil::RecMutex::Lock topicSync(_upstreamRecordMutex);
    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	out << _name << " linkNotification " << name;
    }

    _upstreamRecord.push_back(upstream);
    _instance->keepAlive()->add(upstream);

    //
    // Save
    //
    _upstream.put(PersistentUpstreamMap::value_type(_name, _upstreamRecord));
}

void
TopicI::unlinkNotification(const string& name, const TopicUpstreamLinkPrx& upstream, const Ice::Current&)
{
    IceUtil::RecMutex::Lock topicSync(_upstreamRecordMutex);
    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
	out << _name << " unlinkNotification " << name;
    }

    TopicUpstreamLinkPrxSeq::iterator p = find(_upstreamRecord.begin(), _upstreamRecord.end(), upstream);
    if(p != _upstreamRecord.end())
    {
	_upstreamRecord.erase(p);
    }
    _instance->keepAlive()->remove(upstream);

    //
    // Save
    //
    _upstream.put(PersistentUpstreamMap::value_type(_name, _upstreamRecord));
}

bool
TopicI::destroyed() const
{
    IceUtil::RecMutex::Lock sync(_topicRecordMutex);
    return _destroyed;
}

void
TopicI::reap()
{
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

	    size_t erased = _topicRecord.erase(subscriber->id().category);
	    if(erased > 0)
	    {
		updated = true;
	    }
	    if(traceLevels->topic > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
		out << "reaping " << _instance->communicator()->identityToString(subscriber->id());
		if(erased == 0)
		{
		    out << ": failed - not in database";
		}
	    }
	}
	if(updated)
	{
	    _topics.put(PersistentTopicMap::value_type(_name, _topicRecord));
	}
    }

    //
    // Now reap any dead upstream topics.
    //
    {
	IceUtil::RecMutex::Lock topicSync(_upstreamRecordMutex);
	if(_destroyed)
	{
	    return;
	}

	if(_instance->keepAlive()->filter(_upstreamRecord))
	{
	    // Save.
	    _upstream.put(PersistentUpstreamMap::value_type(_name, _upstreamRecord));
	}
    }
}

void
TopicI::publish(bool forwarded, const EventSeq& events)
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
    Ice::Identity ident = obj->ice_getIdentity();
    
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), ident);
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
	out << _instance->communicator()->identityToString(ident) << ": not subscribed.";
    }
}
