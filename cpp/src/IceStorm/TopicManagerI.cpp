// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/SliceChecksums.h>
#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TopicI.h>
#include <IceStorm/Flusher.h>
#include <IceStorm/TraceLevels.h>
#include <Freeze/Initialize.h>

#include <functional>
#include <ctype.h>

using namespace IceStorm;
using namespace std;

TopicManagerI::TopicManagerI(const Ice::CommunicatorPtr& communicator, const Ice::ObjectAdapterPtr& topicAdapter,
                             const Ice::ObjectAdapterPtr& publishAdapter, const TraceLevelsPtr& traceLevels,
                             const string& envName, const string& dbName) :
    _communicator(communicator),
    _topicAdapter(topicAdapter),
    _publishAdapter(publishAdapter),
    _traceLevels(traceLevels),
    _envName(envName),
    _dbName(dbName),
    _connection(Freeze::createConnection(_communicator, envName)),
    _topics(_connection, dbName)
{
    _flusher = new Flusher(_communicator, _traceLevels);
    _factory = new SubscriberFactory(_communicator, _traceLevels, _flusher);

    //
    // Recreate each of the topics in the persistent map
    //
    for(PersistentTopicMap::iterator p = _topics.begin(); p != _topics.end(); ++p)
    {
	installTopic(p->first, p->second, false);
    }
}

TopicManagerI::~TopicManagerI()
{
    _flusher->stopFlushing();
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
            if(_traceLevels->topicMgr > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->topicMgrCat);
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
}

void
TopicManagerI::installTopic(const string& name, const LinkRecordDict& links, bool create)
{
    //
    // Called by constructor or with 'this' mutex locked. 
    //

    if(_traceLevels->topicMgr > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicMgrCat);
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
    TopicIPtr topicI = new TopicI(_publishAdapter, _traceLevels, name, links, _factory, _envName, _dbName);
    
    //
    // The identity is the name of the Topic.
    //
    Ice::Identity id;
    id.name = name;
    _topicAdapter->add(topicI, id);
    _topicIMap.insert(TopicIMap::value_type(name, topicI));
}
