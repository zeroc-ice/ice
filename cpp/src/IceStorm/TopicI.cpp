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
#include <IceStorm/Flusher.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>

#include <algorithm>

using namespace IceStorm;
using namespace std;

namespace IceStorm
{

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from Publishers.
//
class BlobjectI : public Ice::Blobject
{
public:

    BlobjectI(const IceStorm::TopicSubscribersPtr& s) :
	_subscribers(s)
    {
    }

    ~BlobjectI()
    {
    }

    virtual void ice_invoke(const vector< Ice::Byte>&, vector< Ice::Byte>&, const Ice::Current&);

private:

    //
    // Set of associated subscribers
    //
    IceStorm::TopicSubscribersPtr _subscribers;
};


class TopicSubscribers : public IceUtil::Shared, public JTCMutex
{
public:

    TopicSubscribers(const TraceLevelsPtr& traceLevels, const Ice::LoggerPtr& logger, const string& topic,
		     const FlusherPtr& flusher) :
	_traceLevels(traceLevels),
	_logger(logger),
	_topic(topic),
	_flusher(flusher)
    {
    }

    ~TopicSubscribers()
    {
    }

    void
    add(const Ice::ObjectPrx& s, const string& idPrefix, const QoS& qos)
    {
	//
	// Create the full topic subscriber id (<prefix>#<topic>)
	//
	string id = idPrefix;
	id += '#';
	id += _topic;

	//
	// Change the identity of the proxy
	//
	Ice::ObjectPrx obj = s->ice_newIdentity(id);

	SubscriberPtr subscriber = new Subscriber(_logger, _traceLevels, _flusher, qos, obj);

	JTCSyncT<JTCMutex> sync(*this);

	//
	// Add to the set of subscribers
	//
	_subscribers.push_back(subscriber);
    }
    
    void
    remove(const string& idPrefix)
    {
	JTCSyncT<JTCMutex> sync(*this);

	//
	// Create the full topic subscriber id (<prefix>#<topic>)
	//
	string id = idPrefix;
	id += '#';
	id += _topic;

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
		s << _topic << ": " << id << "not subscribed.";
		_logger->trace(_traceLevels->topicCat, s.str());
	    }
	}
    }


    void
    publish(const string& op, bool nonmutating, const vector< Ice::Byte>& blob)
    {
	JTCSyncT<JTCMutex> sync(*this);

	//
	// Using standard algorithms I don't think there is a way to
	// do this in one pass. For instance, I thought about using
	// remove_if - but the predicate needs to be a pure function
	// (see Meyers for details). If this is fixed then fix Flusher
	// also.
	//
	// remove_if doesn't work with handle types. remove_if also //
	// isn't present in the STLport implementation
	//
        // _subscribers.remove_if(IceUtil::constMemFun(&Subscriber::invalid));
        //
        _subscribers.erase(remove_if(_subscribers.begin(), _subscribers.end(),
				     IceUtil::constMemFun(&Subscriber::invalid)), _subscribers.end());

	for (SubscriberList::iterator i = _subscribers.begin(); i != _subscribers.end(); ++i)
	{
	    (*i)->publish(op, nonmutating, blob);
	}
	//for_each(_subscribers.begin(), _subscribers.end(), Ice::memFun(&Subscriber::publish));
    }

private:
    
    TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;
    
    string _topic;
    FlusherPtr _flusher;
    SubscriberList _subscribers;
};

} // End namespace IceStorm

void
BlobjectI::ice_invoke(const vector< Ice::Byte>& inParams, vector< Ice::Byte>& outParam, const Ice::Current& current)
{
    _subscribers->publish(current.operation, current.nonmutating, inParams);
}

TopicI::TopicI(const Ice::ObjectAdapterPtr& adapter, const TraceLevelsPtr& traceLevels, const Ice::LoggerPtr& logger,
               const string& name, const FlusherPtr& flusher) :
    _adapter(adapter),
    _traceLevels(traceLevels),
    _logger(logger),
    _name(name),
    _flusher(flusher),
    _destroyed(false)
{
    _subscribers = new TopicSubscribers(_traceLevels, _logger, _name, _flusher);

    //
    // Create a servant per Topic to receive event data. The servants
    // object-id is <topic>#publish. Active the object and save a
    // reference to give to publishers.
    //
    _publisher = new BlobjectI(_subscribers);

    string id = name;
    id += '#';
    id += "publish";

    _adapter->add(_publisher, id);
    _obj = adapter->createProxy(id);
}

TopicI::~TopicI()
{
}

string
TopicI::getName(const Ice::Current&)
{
    return _name;
}

Ice::ObjectPrx
TopicI::getPublisher(const Ice::Current&)
{
    return _obj;
}

void
TopicI::destroy(const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(_destroyedMutex);
    if (_traceLevels->topic > 0)
    {
	ostringstream s;
	s << "Destroy " << _name;
	_logger->trace(_traceLevels->topicCat, s.str());
    }

    _adapter->remove(_name);
    _destroyed = true;
}

bool
TopicI::destroyed() const
{
    JTCSyncT<JTCMutex> sync(_destroyedMutex);
    return _destroyed;
}

void
TopicI::subscribe(const Ice::ObjectPrx& tmpl, const string& id, const QoS& qos)
{
    _subscribers->add(tmpl, id, qos);
}

void
TopicI::unsubscribe(const string& id)
{
    _subscribers->remove(id);
}
