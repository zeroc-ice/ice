// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TopicI.h>
#include <IceStorm/SubscriberPool.h>
#include <IceStorm/BatchFlusher.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Instance.h>
#include <Freeze/Initialize.h>

#include <Ice/SliceChecksums.h>

#include <functional>
#include <ctype.h>


using namespace IceStorm;
using namespace std;

TopicManagerI::TopicManagerI(
    const InstancePtr& instance,
    const Ice::ObjectAdapterPtr& topicAdapter,
    const string& envName,
    const string& dbName) :
    _instance(instance),
    _topicAdapter(topicAdapter),
    _envName(envName),
    _dbName(dbName),
    _connection(Freeze::createConnection(instance->communicator(), envName)),
    _topics(_connection, dbName)
{
    //
    // Recreate each of the topics in the persistent map
    //
    for(PersistentTopicMap::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
    {
	installTopic(p->first, p->second, false);
    }
}

TopicManagerI::~TopicManagerI()
{
}

TopicPrx
TopicManagerI::create(const string& name, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    reap();

    if(_topicIMap.find(name) != _topicIMap.end())
    {
        TopicExists ex;
	ex.name = name;
        throw ex;
    }

    _topics.put(PersistentTopicMap::value_type(name, LinkRecordDict()));
    installTopic(name, LinkRecordDict(), true);

    //
    // The identity is the name of the Topic.
    //
    Ice::Identity id;
    id.name = name;
    return TopicPrx::uncheckedCast(_topicAdapter->createProxy(id));
}

TopicPrx
TopicManagerI::retrieve(const string& name, const Ice::Current&) const
{
    IceUtil::Mutex::Lock sync(*this);

    TopicManagerI* This = const_cast<TopicManagerI*>(this);
    This->reap();

    if(_topicIMap.find(name) != _topicIMap.end())
    {
	Ice::Identity id;
	id.name = name;
	return TopicPrx::uncheckedCast(_topicAdapter->createProxy(id));
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

    TopicManagerI* This = const_cast<TopicManagerI*>(this);
    This->reap();

    TopicDict all;
    transform(_topicIMap.begin(), _topicIMap.end(), inserter(all, all.begin()),
	      TransformToTopicDict(_topicAdapter));

    return all;
}

Ice::SliceChecksumDict
TopicManagerI::getSliceChecksums(const Ice::Current&) const
{
    return Ice::sliceChecksums();
}

void
TopicManagerI::reap()
{
    //
    // Always called with mutex locked.
    //
    // IceUtil::Mutex::Lock sync(*this);
    //
    TopicIMap::iterator i = _topicIMap.begin();
    while(i != _topicIMap.end())
    {
        if(i->second->destroyed())
        {
	    TraceLevelsPtr traceLevels = _instance->traceLevels();
            if(traceLevels->topicMgr > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
                out << "Reaping " << i->first;
            }

            _topics.erase(i->first);

            try
            {
                Ice::Identity id;
                id.name = i->first;
                _topicAdapter->remove(id);
            }
            catch(const Ice::ObjectAdapterDeactivatedException&)
            {
                // Ignore
            }

            _topicIMap.erase(i++);
        }
        else
        {
            ++i;
        }
    }
}

void
TopicManagerI::shutdown()
{
    IceUtil::Mutex::Lock sync(*this);

    reap(); 
    for(TopicIMap::const_iterator p = _topicIMap.begin(); p != _topicIMap.end(); ++p)
    {
	p->second->reap();
    }
}

void
TopicManagerI::installTopic(const string& name, const LinkRecordDict& rec, bool create)
{
    //
    // Called by constructor or with 'this' mutex locked. 
    //

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topicMgr > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
	if(create)
	{
	    out << "creating new topic \"" << name << "\"";
	}
	else
	{
	    out << "loading topic \"" << name << "\" from database";
	}
    }

    //
    // Create topic implementation
    //
    TopicIPtr topicI = new TopicI(_instance, name, rec, _envName, _dbName);
    
    //
    // The identity is the name of the Topic.
    //
    Ice::Identity id;
    id.name = name;
    _topicAdapter->add(topicI, id);
    _topicIMap.insert(TopicIMap::value_type(name, topicI));
}
