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
    while (p != _topics.end())
    {
	assert(_topicIMap.find(p->first) == _topicIMap.end());
	try
	{
	    installTopic("recreate", p->first, false);
	    ++p;
	}
	catch(const Freeze::DBNotFoundException& ex)
	{
	    if (_traceLevels->topicMgr > 0)
	    {
		ostringstream s;
		s << ex;
		_communicator->getLogger()->trace(_traceLevels->topicMgrCat, s.str());
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
    JTCSyncT<JTCMutex> sync(*this);

    reap();

    if (_topicIMap.find(name) != _topicIMap.end())
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
TopicManagerI::retrieve(const string& name, const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this);

    reap();

    if (_topicIMap.find(name) != _topicIMap.end())
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
static TopicDict::value_type
transformToTopicDict(TopicIMap::value_type p, Ice::ObjectAdapterPtr adapter)
{
    Ice::Identity id;
    id.name = p.first;
    return TopicDict::value_type(p.first, TopicPrx::uncheckedCast(adapter->createProxy(id)));
}

TopicDict
TopicManagerI::retrieveAll(const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this);

    reap();

    TopicDict all;
    transform(_topicIMap.begin(), _topicIMap.end(), inserter(all, all.begin()),
		   bind2nd(ptr_fun(transformToTopicDict), _adapter));

    return all;
}

void
TopicManagerI::subscribe(const string& id, const QoS& qos, const StringSeq& topics, const Ice::ObjectPrx& tmpl,
			 const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_traceLevels->topicMgr > 0)
    {
	ostringstream s;
	s << "Subscribe: " << id;
	if (_traceLevels->topicMgr > 1)
	{
	    s << " QoS: ";
	    for (QoS::const_iterator qi = qos.begin(); qi != qos.end() ; ++qi)
	    {
		if (qi != qos.begin())
		{
		    s << ',';
		}
		s << '[' << qi->first << "," << qi->second << ']';
	    }
	    s << " Topics: ";
	    for (StringSeq::const_iterator ti = topics.begin(); ti != topics.end() ; ++ti)
	    {
		if (ti != topics.begin())
		{
		    s << ",";
		}
		s << *ti;
	    }
	}
	_communicator->getLogger()->trace(_traceLevels->topicMgrCat, s.str());
    }

    //
    // First scan the set of topics to ensure that each exists.
    //
    // TODO: This could be slightly optimized by remembering the
    // TopicIPtr's so that the list doesn't need to scanned again.
    //
    StringSeq::const_iterator i;
    for (i = topics.begin() ; i != topics.end() ; ++i)
    {
	TopicIMap::iterator elem = _topicIMap.find(*i);
	if (elem == _topicIMap.end())
	{
	    NoSuchTopic ex;
	    ex.name = *i;
	    throw ex;
	}
    }

    //
    // Subscribe to each Topic.
    //
    for (i = topics.begin() ; i != topics.end() ; ++i)
    {
	TopicIMap::iterator elem = _topicIMap.find(*i);
	if (elem != _topicIMap.end())
	{
	    elem->second->subscribe(tmpl, id, qos);
	}
    }
}

void
TopicManagerI::unsubscribe(const string& id, const StringSeq& topics, const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_traceLevels->topicMgr > 0)
    {
	ostringstream s;
	s << "Unsubscribe: " << id;
	if (_traceLevels->topicMgr > 1)
	{
	    s << " Topics: ";
	    for (StringSeq::const_iterator ti = topics.begin(); ti != topics.end() ; ++ti)
	    {
		if (ti != topics.begin())
		{
		    s << ",";
		}
		s << *ti;
	    }
	}
	_communicator->getLogger()->trace(_traceLevels->topicMgrCat, s.str());
    }

    //
    // Unsubscribe to each Topic.
    //
    for (StringSeq::const_iterator i = topics.begin() ; i != topics.end() ; ++i)
    {
	TopicIMap::iterator elem = _topicIMap.find(*i);
	if (elem != _topicIMap.end())
	{
	    elem->second->unsubscribe(id);
	}
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
    // JTCSyncT<JTCMutex> sync(*this);
    //
    TopicIMap::iterator i = _topicIMap.begin();
    while (i != _topicIMap.end())
    {
	if (i->second->destroyed())
	{
	    if (_traceLevels->topicMgr > 0)
	    {
		ostringstream s;
		s << "Reaping " << i->first;
		_communicator->getLogger()->trace(_traceLevels->topicMgrCat, s.str());
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
    if (_traceLevels->topicMgr > 0)
    {
	ostringstream s;
	s << message << ' ' << name;
	_communicator->getLogger()->trace(_traceLevels->topicMgrCat, s.str());
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
