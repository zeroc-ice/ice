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
			     const TraceLevelsPtr& traceLevels) :
    _communicator(communicator),
    _adapter(adapter),
    _traceLevels(traceLevels)

{
    _flusher = new Flusher(_communicator, _traceLevels);
}

TopicManagerI::~TopicManagerI()
{
}

TopicPrx
TopicManagerI::create(const string& name)
{
    // TODO: reader/writer mutex
    JTCSyncT<JTCMutex> sync(*this);

    reap();

    if (_topicIMap.find(name) != _topicIMap.end())
    {
	throw TopicExists();
    }

    if (_traceLevels->topicMgr > 0)
    {
	ostringstream s;
	s << "Create " << name;
	_communicator->getLogger()->trace(_traceLevels->topicMgrCat, s.str());
    }

    //
    // Create topic implementation
    //
    TopicIPtr topicI = new TopicI(_adapter, _traceLevels, _communicator->getLogger(), name, _flusher);
    _adapter->add(topicI, name);
    _topicIMap.insert(TopicIMap::value_type(name, topicI));

    return TopicPrx::uncheckedCast(_adapter->createProxy(name));
}

TopicPrx
TopicManagerI::retrieve(const string& name)
{
    JTCSyncT<JTCMutex> sync(*this);

    reap();

    if (_topicIMap.find(name) != _topicIMap.end())
	return TopicPrx::uncheckedCast(_adapter->createProxy(name));
    throw NoSuchTopic();
}

//
// The arguments cannot be const & (for some reason)
//
static TopicDict::value_type
transformToTopicDict(TopicIMap::value_type p, Ice::ObjectAdapterPtr adapter)
{
    return TopicDict::value_type(p.first, TopicPrx::uncheckedCast(adapter->createProxy(p.first)));
}

TopicDict
TopicManagerI::retrieveAll()
{
    JTCSyncT<JTCMutex> sync(*this);

    reap();

    TopicDict all;
    transform(_topicIMap.begin(), _topicIMap.end(), inserter(all, all.begin()),
		   bind2nd(ptr_fun(transformToTopicDict), _adapter));

    return all;
}

void
TopicManagerI::subscribe(const string& id, const QoS& qos, const StringSeq& topics, const Ice::ObjectPrx& tmpl)
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

    for (StringSeq::const_iterator i = topics.begin() ; i != topics.end() ; ++i)
    {
	TopicIMap::iterator elem = _topicIMap.find(*i);
	if (elem != _topicIMap.end())
	{
	    elem->second->subscribe(tmpl, id, qos);
	}
    }
}

void
TopicManagerI::unsubscribe(const string& id, const StringSeq& topics)
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
TopicManagerI::shutdown()
{
    _flusher->stopFlushing();
    _communicator->shutdown();
}

void
TopicManagerI::reap()
{
}

