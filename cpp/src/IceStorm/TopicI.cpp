// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/TopicI.h>
#include <IceStorm/SubscriberFactory.h>
#include <IceStorm/OnewaySubscriber.h>
#include <IceStorm/LinkSubscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Event.h>

#include <Freeze/Initialize.h>
#include <IceStorm/KeepAliveThread.h>
#include <algorithm>

#include <Ice/LoggerUtil.h>

using namespace IceStorm;
using namespace std;

namespace IceStorm
{

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from Publishers.
//
class PublisherProxyI : public Ice::BlobjectArray
{
public:

    PublisherProxyI(const IceStorm::TopicSubscribersPtr& s) :
	_subscribers(s)
    {
    }

    ~PublisherProxyI()
    {
    }

    virtual bool ice_invoke(const pair<const Ice::Byte*, const Ice::Byte*>&, vector< Ice::Byte>&, const Ice::Current&);

private:

    //
    // Set of associated subscribers
    //
    const IceStorm::TopicSubscribersPtr _subscribers;
};

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from linked Topics.
//
class TopicLinkI : public TopicLink
{
public:

    TopicLinkI(const IceStorm::TopicSubscribersPtr& s) :
	_subscribers(s)
    {
    }

    ~TopicLinkI()
    {
    }

    virtual void forward(const vector<EventData>&, const Ice::Current&);

private:

    //
    // Set of associated subscribers
    //
    const IceStorm::TopicSubscribersPtr _subscribers;
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

} // End namespace IceStorm

IceStorm::TopicSubscribers::TopicSubscribers(const Ice::CommunicatorPtr& communicator, 
					     const TraceLevelsPtr& traceLevels) :
    _communicator(communicator),
    _traceLevels(traceLevels)
{
}

IceStorm::TopicSubscribers::~TopicSubscribers()
{
}

void
IceStorm::TopicSubscribers::add(const SubscriberPtr& subscriber)
{
    Ice::Identity id = subscriber->id();
    
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    
    //
    // If a subscriber with this identity is already subscribed
    // then mark the subscriber as replaced.
    //
    // Note that this doesn't actually remove the subscriber from
    // the list of subscribers - it marks the subscriber as
    // replaced, and it's removed on the next event publish.
    //
    for(SubscriberList::iterator i = _subscribers.begin() ; i != _subscribers.end(); ++i)
    {
	if(!(*i)->inactive() && (*i)->id() == id)
	{
	    //
	    // This marks an active subscriber as replaced. It will be
	    // removed on the next event publish.
	    //
	    (*i)->replace();
	    break;
	}
    }

    //
    // Activate and add to the set of subscribers.
    //
    subscriber->activate();
    _subscribers.push_back(subscriber);
}

//
// Unsubscribe the subscriber with the given identity. Note that
// this doesn't remove the subscriber from the list of subscribers
// - it marks the subscriber as unsubscribed, and it's removed on
// the next event publish.
//
void
IceStorm::TopicSubscribers::remove(const Ice::ObjectPrx& obj)
{
    Ice::Identity id = obj->ice_getIdentity();
    
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    
    for(SubscriberList::iterator i = _subscribers.begin() ; i != _subscribers.end(); ++i)
    {
	if(!(*i)->inactive() && (*i)->id() == id)
	{
	    //
	    // This marks an active subscriber as unsubscribed. It will be
	    // removed on the next event publish.
	    //
	    (*i)->unsubscribe();
	    return;
	}
    }
    
    //
    // If the subscriber was not found then display a diagnostic.
    //
    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << _communicator->identityToString(id) << ": not subscribed.";
    }
}

//
// TODO: Optimize
//
// It's not strictly necessary to clear the error'd subscribers on
// every publish iteration (if the subscriber validates the state
// before attempting publishing the event). This means more mutex
// locks (due to the state check in the subscriber) - but with the
// advantage that publishes can occur in parallel and less
// subscriber list iterations.
//
void
IceStorm::TopicSubscribers::publish(const EventPtr& event)
{
    //
    // Copy of the subscriber list so that event publishing can
    // occur in parallel.
    //
    // TODO: Find out whether this is a false optimization - how
    // expensive is the cost of copying vs. lack of parallelism?
    //
    SubscriberList copy;
    
    {
	IceUtil::Mutex::Lock sync(_subscribersMutex);
	
	//
	// Copy of the subscribers that are in error.
	//
	SubscriberList e;

	//
	// Erase the inactive subscribers from the _subscribers
	// list. Copy the subscribers in error to the error list.
	//
	SubscriberList::iterator p = _subscribers.begin();
	while(p != _subscribers.end())
	{
	    if((*p)->inactive())
	    {
		//
		// NOTE: only persistent subscribers need to be reaped
		// and copied in the error list. Transient subscribers
		// can be removed right away, the topic doesn't keep
		// any reference on them.
		//
		if((*p)->error() && (*p)->persistent())
		{
		    e.push_back(*p);
		}
		
		SubscriberList::iterator tmp = p;
		++p;
		_subscribers.erase(tmp);
	    }
	    else
	    {
		copy.push_back(*p);
		++p;
	    }
	}
	
	if(!e.empty())
	{
	    IceUtil::Mutex::Lock errorSync(_errorMutex);
	    _error.splice(_error.begin(), e);
	}
    }
    
    for(SubscriberList::iterator p = copy.begin(); p != copy.end(); ++p)
    {
	(*p)->publish(event);
    }
}

//
// Clear & return the set of subscribers that are in error.
//
SubscriberList
IceStorm::TopicSubscribers::clearErrorList()
{
    //
    // Uses splice for efficiency
    //
    IceUtil::Mutex::Lock errorSync(_errorMutex);
    SubscriberList c;
    c.splice(c.begin(), _error);
    return c;
}

//
// Incoming events from publishers.
//
bool
PublisherProxyI::ice_invoke(const pair<const Ice::Byte*, const Ice::Byte*>& inParams, vector< Ice::Byte>& outParam,
                            const Ice::Current& current)
{
    const Ice::Context& context = current.ctx;

    EventPtr event = new Event;
    event->forwarded = false;
    Ice::Context::const_iterator p = context.find("cost");
    if(p != context.end())
    {
        event->cost = atoi(p->second.c_str());
    }
    else
    {
        event->cost = 0; // TODO: Default comes from property?
    }
    event->op = current.operation;
    event->mode = current.mode;
    vector<Ice::Byte>(inParams.first, inParams.second).swap(event->data);
    event->context = context;

    _subscribers->publish(event);

    return true;
}

//
// Incoming events from linked topics.
//
void
TopicLinkI::forward(const vector<EventData>& v, const Ice::Current& current)
{
    for(vector<EventData>::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	EventPtr event = new Event;
	event->forwarded = true;
	event->cost = 0;
	event->op = p->op;
	event->mode = p->mode;
	event->data = p->data;
	event->context = p->context;
	
	_subscribers->publish(event);
    }
}

TopicI::TopicI(const Ice::CommunicatorPtr& communicator, const Ice::ObjectAdapterPtr& adapter, 
	       const TraceLevelsPtr& traceLevels, const KeepAliveThreadPtr& keepAlive, const string& name,
	       const LinkRecordDict& topicRecord, const SubscriberFactoryPtr& factory, const string& envName,
	       const string& dbName) :
    _communicator(communicator),
    _adapter(adapter),
    _traceLevels(traceLevels),
    _keepAlive(keepAlive),
    _name(name),
    _factory(factory),
    _connection(Freeze::createConnection(_communicator, envName)),
    _topics(_connection, dbName, false),
    _topicRecord(topicRecord),
    _upstream(_connection, "upstream", false),
    _destroyed(false)
{
    _subscribers = new TopicSubscribers(_communicator, _traceLevels);

    //
    // Create a servant per topic to receive event data. The servant's
    // identity is category=<topicname>, name=publish. Activate the
    // object and save a reference to give to publishers.
    //
    _publisher = new PublisherProxyI(_subscribers);
    
    Ice::Identity id;
    id.category = _name;
    id.name = "publish";
    _publisherPrx = _adapter->add(_publisher, id);

    //
    // Create a servant per topic to receive linked event data. The
    // servant's identity is category=<topicname>, name=link. Activate
    // the object and save a reference to give to linked topics.
    //
    _link = new TopicLinkI(_subscribers);

    id.name = "link";
    _linkPrx = TopicLinkPrx::uncheckedCast(_adapter->add(_link, id));

    //
    // Re-establish linked subscribers.
    //
    for(LinkRecordDict::const_iterator p = _topicRecord.begin(); p != _topicRecord.end(); ++p)
    {
	if(_traceLevels->topic > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	    out << _name << " relink " << p->first;
	}

	//
	// Create the subscriber object and the upstream servant and
	// add it to the set of subscribers.
	//
	SubscriberPtr subscriber = _factory->createLinkSubscriber(p->second.obj, p->second.cost);
	TopicUpstreamLinkPrx upstream = TopicUpstreamLinkPrx::uncheckedCast(
	    _adapter->add(new TopicUpstreamLinkI(subscriber), p->second.upstream->ice_getIdentity()));
	_subscribers->add(subscriber);
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
	if(_traceLevels->topic > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	    out << _name << " upstream " << _communicator->identityToString((*q)->ice_getIdentity());
	}
	_keepAlive->add(*q);
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

    //
    // See the comment in the constructor for the format of the identities.
    //
    Ice::Identity id;
    id.category = _name;
    id.name = "publish";

    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << "destroying " << _communicator->identityToString(id);
    }

    _adapter->remove(id);

    id.name = "link";

    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << "destroying " << _communicator->identityToString(id);
    }

    _adapter->remove(id);
}

void
TopicI::subscribe(const QoS& qos, const Ice::ObjectPrx& subscriber, const Ice::Current& current)
{
    subscribeAndGetPublisher(qos, subscriber, current);
}

Ice::ObjectPrx
TopicI::subscribeAndGetPublisher(const QoS& qos, const Ice::ObjectPrx& subscriber, const Ice::Current&)
{
    Ice::Identity ident = subscriber->ice_getIdentity();
    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << "Subscribe: " << _communicator->identityToString(ident);
	if(_traceLevels->topic > 1)
	{
	    out << " QoS: ";
	    for(QoS::const_iterator qi = qos.begin(); qi != qos.end() ; ++qi)
	    {
		if(qi != qos.begin())
		{
		    out << ',';
		}
		out << '[' << qi->first << "," << qi->second << ']';
	    }
	}
    }

    //
    // Add this subscriber to the set of subscribers.
    //
    OnewaySubscriberPtr sub = _factory->createSubscriber(_adapter, qos, subscriber);
    _subscribers->add(sub);
    return sub->proxy();
}

void
TopicI::unsubscribe(const Ice::ObjectPrx& subscriber, const Ice::Current&)
{
    if(subscriber == 0)
    {
	if(_traceLevels->topic > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	    out << "unsubscribe with null subscriber.";
	}
	return;
    }

    Ice::Identity ident = subscriber->ice_getIdentity();

    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);

	out << "Unsubscribe: " << _communicator->identityToString(ident);
    }

    //
    // Unsubscribe the subscriber with this identity.
    //
    _subscribers->remove(subscriber);
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
    
    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << _name << " link " << name << " cost " << cost;
    }

    SubscriberPtr subscriber = _factory->createLinkSubscriber(link, cost);
    TopicUpstreamLinkPrx upstream = TopicUpstreamLinkPrx::uncheckedCast(
	_adapter->addWithUUID(new TopicUpstreamLinkI(subscriber)));

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
	_adapter->remove(upstream->ice_getIdentity());
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
    
    //
    // Create the subscriber object and add it to the set of subscribers.
    //
    _subscribers->add(subscriber);
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
	if(_traceLevels->topic > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
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
    _adapter->remove(rec.upstream->ice_getIdentity());

    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << _name << " unlink " << name;
    }
    _subscribers->remove(rec.obj);

    TopicInternalPrx internal = TopicInternalPrx::checkedCast(rec.theTopic);
    try
    {
	internal->unlinkNotification(_name, rec.upstream);
    }
    catch(const Ice::Exception& e)
    {
	if(_traceLevels->topic > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
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

TopicLinkPrx
TopicI::getLinkProxy(const Ice::Current&)
{
    // immutable
    return _linkPrx;
}

void
TopicI::linkNotification(const string& name, const TopicUpstreamLinkPrx& upstream, const Ice::Current&)
{
    IceUtil::RecMutex::Lock topicSync(_upstreamRecordMutex);
    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << _name << " linkNotification " << name;
    }

    _upstreamRecord.push_back(upstream);
    _keepAlive->add(upstream);

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
    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << _name << " unlinkNotification " << name;
    }

    TopicUpstreamLinkPrxSeq::iterator p = find(_upstreamRecord.begin(), _upstreamRecord.end(), upstream);
    if(p != _upstreamRecord.end())
    {
	_upstreamRecord.erase(p);
    }
    _keepAlive->remove(upstream);

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
	SubscriberList error = _subscribers->clearErrorList();
	for(SubscriberList::iterator p = error.begin(); p != error.end(); ++p)
	{
	    SubscriberPtr subscriber = *p;
	    assert(subscriber->error() && subscriber->persistent()); // Only persistent subscribers need to be reaped.

	    if(_topicRecord.erase(subscriber->id().category) > 0)
	    {
		updated = true;
		if(_traceLevels->topic > 0)
		{
		    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
		    out << "reaping " << _communicator->identityToString(subscriber->id());
		}
	    }
	    else
	    {
		if(_traceLevels->topic > 0)
		{
		    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
		    out << "reaping " << _communicator->identityToString(subscriber->id())
			<< " failed - not in database";
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

	if(_keepAlive->filter(_upstreamRecord))
	{
	    // Save.
	    _upstream.put(PersistentUpstreamMap::value_type(_name, _upstreamRecord));
	}
    }
}
