// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Functional.h>

#include <IceStorm/TopicI.h>
#include <IceStorm/SubscriberFactory.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>

#include <algorithm>
#include <list>

using namespace IceStorm;
using namespace std;


namespace IceStorm
{

//
// A list of Subscribers.
//
typedef std::list<SubscriberPtr> SubscriberList;

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from Publishers.
//
class PublisherProxyI : public Ice::Blobject
{
public:

    PublisherProxyI(const IceStorm::TopicSubscribersPtr& s) :
	_subscribers(s)
    {
    }

    ~PublisherProxyI()
    {
    }

    virtual void ice_invoke(const vector< Ice::Byte>&, vector< Ice::Byte>&, const Ice::Current&);

private:

    //
    // Set of associated subscribers
    //
    IceStorm::TopicSubscribersPtr _subscribers;
};

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from linked Topics..
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

    virtual void forward(const string&, bool, const ByteSeq&, const ContextData&, const Ice::Current&);

private:

    //
    // Set of associated subscribers
    //
    IceStorm::TopicSubscribersPtr _subscribers;
};


//
// Holder for the set of subscribers.
//
class TopicSubscribers : public IceUtil::Shared
{
public:

    TopicSubscribers(const TraceLevelsPtr& traceLevels) :
	_traceLevels(traceLevels)
    {
    }

    ~TopicSubscribers()
    {
    }

    void
    add(const SubscriberPtr& subscriber)
    {
	JTCSyncT<JTCMutex> sync(_subscribersMutex);

	//
	// Add to the set of subscribers
	//
	_subscribers.push_back(subscriber);
    }

    //
    // Unsubscribe the Subscriber with the given identity. Note that
    // this doesn't remove the Subscriber from the list of subscribers
    // - it marks the Subscriber as Unsubscribed, and it's removed on
    // the next event publish.
    //
    void
    unsubscribe(const Ice::Identity& id)
    {
	JTCSyncT<JTCMutex> sync(_subscribersMutex);

	SubscriberList::iterator i;
	for (i = _subscribers.begin() ; i != _subscribers.end(); ++i)
	{
	    if ((*i)->id() == id)
	    {
		//
		// This marks the subscriber as invalid. It will be
		// removed on the next event publish.
		//
		(*i)->unsubscribe();
		break;
	    }
	}

	//
	// If the subscriber was not found then display a diagnostic
	//
	if (i == _subscribers.end())
	{
	    if (_traceLevels->topic > 0)
	    {
		ostringstream s;
		s << id << ": not subscribed.";
		_traceLevels->logger->trace(_traceLevels->topicCat, s.str());
	    }
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
    publish(const Event& event)
    {
	JTCSyncT<JTCMutex> sync(_subscribersMutex);

	//
	// Using standard algorithms I don't think there is a way to
	// do this in one pass. For instance, I thought about using
	// remove_if - but the predicate needs to be a pure function
	// (see Meyers for details). If this is fixed then fix Flusher
	// also.
	//
	// remove_if doesn't work with handle types. remove_if also
	// isn't present in the STLport implementation
	//
        // _subscribers.remove_if(IceUtil::constMemFun(&Subscriber::inactive));
        //

	//
	// Erase the inactive subscribers from the _subscribers
	// list. Copy the subscribers in error to the error list.
	//
	SubscriberList::iterator p = remove_if(_subscribers.begin(), _subscribers.end(),
					       IceUtil::constMemFun(&Subscriber::inactive));

	if (p != _subscribers.end())
	{
	    JTCSyncT<JTCMutex> errorSync(errorMutex_);
	    //
	    // Copy each of the invalid subscribers that was not
	    // unsubscribed. Note that there is no copy_if algorithm.
	    //
	    // Note that this could also be written in terms of splice
	    // & remove_if.
	    //
	    while (p != _subscribers.end())
	    {
		if ((*p)->state() == Subscriber::StateError)
		{
		    _error.push_front(*p);
		}
		_subscribers.erase(p++);
	    }
	}

	for (SubscriberList::iterator i = _subscribers.begin(); i != _subscribers.end(); ++i)
	{
	    (*i)->publish(event);
	}
    }

    //
    // Clear & return the set of subscribers that are in error.
    //
    SubscriberList
    clearErrorList() const
    {
	//
	// Uses splice for efficiency
	//
	JTCSyncT<JTCMutex> errorSync(errorMutex_);
	SubscriberList c;
	c.splice(c.begin(), _error);
	return c;
    }

private:
    
    TraceLevelsPtr _traceLevels;

    JTCMutex    _subscribersMutex;
    SubscriberList _subscribers;

    //
    // Set of subscribers that encountered an error.
    //
    JTCMutex errorMutex_;
    mutable SubscriberList _error;
};

} // End namespace IceStorm

//
// Incoming events from publishers.
//
void
PublisherProxyI::ice_invoke(const vector< Ice::Byte>& inParams, vector< Ice::Byte>& outParam,
                            const Ice::Current& current)
{
    const Ice::Context& context = current.context;

    Event event;
    event.forwarded = false;
    Ice::Context::const_iterator p = context.find("cost");
    if (p != context.end())
    {
	event.cost = atoi(p->second.c_str());
    }
    else
    {
	event.cost = 0; // TODO: Default comes from property?
    }
    event.op = current.operation;
    event.nonmutating = current.nonmutating;
    event.data = inParams;
    event.context = context;

    _subscribers->publish(event);
}

//
// Incoming events from linked topics.
//
void
TopicLinkI::forward(const string& op, bool nonmutating, const ByteSeq& data, const ContextData& context,
                    const Ice::Current& current)
{
    Event event;
    event.forwarded = true;
    event.cost = 0;
    event.op = op;
    event.nonmutating = nonmutating;
    event.data = data;
    event.context = context;

    _subscribers->publish(event);
}

TopicI::TopicI(const Ice::ObjectAdapterPtr& adapter, const TraceLevelsPtr& traceLevels, const string& name,
	       const SubscriberFactoryPtr& factory, const Freeze::DBPtr& db) :
    _adapter(adapter),
    _traceLevels(traceLevels),
    _name(name),
    _factory(factory),
    _destroyed(false),
    _links(db),
    _linksDb(db)
{
    _subscribers = new TopicSubscribers(_traceLevels);

    //
    // Create a servant per Topic to receive event data. The servants
    // Identity is category=<topicname>, name=publish. Activate the
    // object and save a reference to give to publishers.
    //
    _publisher = new PublisherProxyI(_subscribers);
    
    Ice::Identity id;
    id.category = _name;
    id.name = "publish";
    _publisherPrx = _adapter->add(_publisher, id);

    //
    // Create a servant per Topic to receive linked event data. The
    // servants Identity is category=<topicname>, name=link. Activate
    // the object and save a reference to give to linked topics..
    //
    _link = new TopicLinkI(_subscribers);

    id.name = "link";
    _linkPrx = TopicLinkPrx::uncheckedCast(_adapter->add(_link, id));

    //
    // Run through link database - re-establishing linked subscribers
    //
    for (IdentityLinkDict::iterator p = _links.begin(); p != _links.end(); ++p)
    {
	if (_traceLevels->topic > 0)
	{
	    ostringstream s;
	    s << _name << " relink " << p->second.obj->ice_getIdentity();
	    _traceLevels->logger->trace(_traceLevels->topicCat, s.str());
	}
	
	SubscriberPtr subscriber = _factory->createLinkSubscriber(p->second.obj, p->second.info.cost);
	_subscribers->add(subscriber);
    }
}

TopicI::~TopicI()
{
}

string
TopicI::getName(const Ice::Current&)
{
    // Immutable
    return _name;
}

Ice::ObjectPrx
TopicI::getPublisher(const Ice::Current&)
{
    // Immutable
    return _publisherPrx;
}

void
TopicI::destroy(const Ice::Current&)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    _destroyed = true;

    //
    // See the comment in the constructor for the derevation of the
    // Identity.
    //
    Ice::Identity id;
    id.category = _name;
    id.name = "publish";

    if (_traceLevels->topic > 0)
    {
	ostringstream s;
	s << "destroying " << id;
	_traceLevels->logger->trace(_traceLevels->topicCat, s.str());
    }

    _adapter->remove(id);

    _linksDb->remove();
    _linksDb = 0;
}

void
TopicI::link(const TopicPrx& topic, Ice::Int cost, const Ice::Current&)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    string name = topic->getName();
    if (_traceLevels->topic > 0)
    {
	ostringstream s;
	s << _name << " link " << name << " cost " << cost;
	_traceLevels->logger->trace(_traceLevels->topicCat, s.str());
    }

    //
    // Retrieve the TopicLink
    //
    TopicInternalPrx internal = TopicInternalPrx::checkedCast(topic);
    TopicLinkPrx link = internal->getLinkProxy();
    Ice::Identity ident = link->ice_getIdentity();

    LinkDB dbInfo;
    dbInfo.obj = link;
    dbInfo.info.topic = topic;
    dbInfo.info.name = name;
    dbInfo.info.cost = cost;

    //
    // If the link already exists then remove the original subscriber.
    //
    // Note: If events arrive before the new subscriber is added then
    // they will be lost. An alternative to this strategy would be to
    // update the subscriber information.
    //
    IdentityLinkDict::iterator p = _links.find(ident);
    if (p != _links.end())
    {
	_subscribers->unsubscribe(ident);
    }

    _links.insert(make_pair(ident, dbInfo));

    SubscriberPtr subscriber = _factory->createLinkSubscriber(dbInfo.obj, dbInfo.info.cost);
    _subscribers->add(subscriber);
}

void
TopicI::unlink(const TopicPrx& topic, const Ice::Current&)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    TopicInternalPrx internal = TopicInternalPrx::checkedCast(topic);
    Ice::ObjectPrx link = internal->getLinkProxy();

    if (_links.erase(link->ice_getIdentity()) > 0)
    {
	if (_traceLevels->topic > 0)
	{
	    ostringstream s;
	    s << _name << " unlink " << topic->getName();
	    _traceLevels->logger->trace(_traceLevels->topicCat, s.str());
	}
	_subscribers->unsubscribe(link->ice_getIdentity());
    }
    else
    {
	if (_traceLevels->topic > 0)
	{
	    ostringstream s;
	    s << _name << " unlink " << topic->getName() << " failed - not linked";
	    _traceLevels->logger->trace(_traceLevels->topicCat, s.str());
	}
    }
}

LinkInfoSeq
TopicI::getLinkInfoSeq(const Ice::Current&)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    LinkInfoSeq seq;

    for (IdentityLinkDict::const_iterator p = _links.begin(); p != _links.end(); ++p)
    {
	LinkInfo info = p->second.info;
	seq.push_back(info);
    }

    return seq;
}

TopicLinkPrx
TopicI::getLinkProxy(const Ice::Current&)
{
    // Immutable
    return _linkPrx;
}

bool
TopicI::destroyed() const
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    return _destroyed;
}

void
TopicI::subscribe(const Ice::ObjectPrx& tmpl, const string& id, const QoS& qos)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    //
    // Create the identity category=id, name=<topicname>.
    //
    Ice::Identity ident;
    ident.category = id;
    ident.name = _name;
    Ice::ObjectPrx obj = tmpl->ice_newIdentity(ident);
    
    //
    // Create the subscriber with this id and add to the set of
    // subscribers.
    //
    SubscriberPtr subscriber = _factory->createSubscriber(qos, obj);
    _subscribers->add(subscriber);
}

void
TopicI::unsubscribe(const string& id)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    //
    // Create the identity category=id, name=<topicname>.
    //
    Ice::Identity ident;
    ident.category = id;
    ident.name = _name;

    //
    // Unsubscribe the subscriber with this identity.
    //
    _subscribers->unsubscribe(ident);
}

void
TopicI::reap()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_destroyed)
    {
	return;
    }

    //
    // Run through all invalid subscribers and remove them from the
    // database.
    //
    SubscriberList error = _subscribers->clearErrorList();
    for (SubscriberList::iterator p = error.begin(); p != error.end(); ++p)
    {
	SubscriberPtr subscriber = *p;
	assert(subscriber->state() == Subscriber::StateError);
	if (subscriber->persistent())
	{
	    if (_links.erase(subscriber->id()) > 0)
	    {
		if (_traceLevels->topic > 0)
		{
		    ostringstream s;
		    s << "reaping " << subscriber->id();
		    _traceLevels->logger->trace(_traceLevels->topicCat, s.str());
		}
	    }
	    else
	    {
		if (_traceLevels->topic > 0)
		{
		    ostringstream s;
		    s << "reaping " << subscriber->id() << " failed - not in database";
		    _traceLevels->logger->trace(_traceLevels->topicCat, s.str());
		}
	    }
	}
    }
}
