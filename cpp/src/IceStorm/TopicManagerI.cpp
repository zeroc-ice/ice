// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TopicI.h>
#include <IceStorm/Flusher.h>
#include <IceStorm/TraceLevels.h>

#include <functional>

using namespace IceStorm;
using namespace std;

TopicManagerI::TopicManagerI(const Ice::CommunicatorPtr& communicator, const Ice::ObjectAdapterPtr& adapter,
			     const TraceLevelsPtr& traceLevels, const Freeze::DBEnvironmentPtr& dbEnv,
			     const Freeze::DBPtr& db) :
    _communicator(communicator),
    _adapter(adapter),
    _traceLevels(traceLevels),
    _dbEnv(dbEnv),
    _topics(db)
{
    _flusher = new Flusher(_communicator, _traceLevels);
    _factory = new SubscriberFactory(_traceLevels, _flusher);

    //
    // Recreate each of the topics in the dictionary. If the topic
    // database doesn't exist then the topic was previously destroyed,
    // but not removed from the _topics dictionary. Normally this
    // should only occur upon a crash.
    //
    StringBoolDict::iterator p = _topics.begin();
    while(p != _topics.end())
    {
	assert(_topicIMap.find(p->first) == _topicIMap.end());
	try
	{
	    installTopic("recreate", p->first, false);
	    ++p;
	}
	catch(const Freeze::DBNotFoundException& ex)
	{
	    if(_traceLevels->topicMgr > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->topicMgrCat);
		out << ex;
	    }
	    StringBoolDict::iterator tmp = p;
	    ++p;
	    _topics.erase(tmp);
	}
    }
}

TopicManagerI::~TopicManagerI()
{
}

TopicPrx
TopicManagerI::create(const string& name, const Ice::Current&)
{
    // TODO: reader/writer mutex
    IceUtil::Mutex::Lock sync(*this);

    reap();

    if(_topicIMap.find(name) != _topicIMap.end())
    {
        TopicExists ex;
	ex.name = name;
        throw ex;
    }

    installTopic("create", name, true);
    _topics.insert(make_pair(name, true));

    //
    // The identity is the name of the Topic.
    //
    Ice::Identity id;
    id.name = name;
    return TopicPrx::uncheckedCast(_adapter->createProxy(id));
}

TopicPrx
TopicManagerI::retrieve(const string& name, const Ice::Current&) const
{
    IceUtil::Mutex::Lock sync(*this);

    TopicManagerI* const This = const_cast<TopicManagerI* const>(this);
    This->reap();

    if(_topicIMap.find(name) != _topicIMap.end())
    {
	Ice::Identity id;
	id.name = name;
	return TopicPrx::uncheckedCast(_adapter->createProxy(id));
    }

    NoSuchTopic ex;
    ex.name = name;
    throw ex;
}

//
// The arguments cannot be const & (for some reason)
//
struct TransformToTopicDict : public std::unary_function<TopicIMap::value_type, TopicDict::value_type>
{

  TransformToTopicDict(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
  {
  }

  TopicDict::value_type
  operator()(TopicIMap::value_type p)
  {
    Ice::Identity id;
    id.name = p.first;
    return TopicDict::value_type(p.first, TopicPrx::uncheckedCast(_adapter->createProxy(id)));
  }

  Ice::ObjectAdapterPtr _adapter;
};

TopicDict
TopicManagerI::retrieveAll(const Ice::Current&) const
{
    IceUtil::Mutex::Lock sync(*this);

    TopicManagerI* const This = const_cast<TopicManagerI* const>(this);
    This->reap();

    TopicDict all;
    transform(_topicIMap.begin(), _topicIMap.end(), inserter(all, all.begin()),
	      TransformToTopicDict(_adapter));

    return all;
}

void
TopicManagerI::subscribe(const QoS& qos, const Ice::ObjectPrx& subscriber, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    Ice::Identity ident = subscriber->ice_getIdentity();
    if(_traceLevels->topicMgr > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicMgrCat);
	out << "Subscribe: " << Ice::identityToString(ident);
	if(_traceLevels->topicMgr > 1)
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
    // Ensure that the identities category refers to an existing
    // channel.
    //
    TopicIMap::iterator elem = _topicIMap.find(ident.category);
    if(elem == _topicIMap.end())
    {
	NoSuchTopic ex;
	ex.name = ident.category;
	throw ex;
    }

    //
    // Subscribe to the topic.
    //
    elem->second->subscribe(subscriber, qos);
}

void
TopicManagerI::unsubscribe(const Ice::ObjectPrx& subscriber, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    Ice::Identity ident = subscriber->ice_getIdentity();

    if(_traceLevels->topicMgr > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicMgrCat);

	out << "Unsubscribe: " << Ice::identityToString(ident);
    }


    TopicIMap::iterator elem = _topicIMap.find(ident.category);
    if(elem != _topicIMap.end())
    {
	elem->second->unsubscribe(subscriber);
    }
}

void
TopicManagerI::shutdown(const Ice::Current&)
{
    _flusher->stopFlushing();
    _communicator->shutdown();
}

void
TopicManagerI::reap()
{
    //
    // Always Called with mutex locked
    //
    // IceUtil::Mutex::Lock sync(*this);
    //
    TopicIMap::iterator i = _topicIMap.begin();
    while(i != _topicIMap.end())
    {
	if(i->second->destroyed())
	{
	    if(_traceLevels->topicMgr > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->topicMgrCat);
		out << "Reaping " << i->first;
	    }
	    _topics.erase(i->first);
	    _topicIMap.erase(i++);
	}
	else
	{
	    ++i;
	}
    }
}

void
TopicManagerI::installTopic(const std::string& message, const std::string& name, bool create)
{
    if(_traceLevels->topicMgr > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicMgrCat);
	out << message << ' ' << name;
    }

    // TODO: instance
    // TODO: reserved names?
    // TODO: failure? cleanup database?
    Freeze::DBPtr db = _dbEnv->openDB(name, create);
    
    //
    // Create topic implementation
    //
    TopicIPtr topicI = new TopicI(_adapter, _traceLevels, name, _factory, db);
    
    //
    // The identity is the name of the Topic.
    //
    Ice::Identity id;
    id.name = name;
    _adapter->add(topicI, id);
    _topicIMap.insert(TopicIMap::value_type(name, topicI));
}
