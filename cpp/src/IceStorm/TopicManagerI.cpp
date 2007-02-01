// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
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

namespace IceStorm
{

string
identityToTopicName(const Ice::Identity& id)
{
    //
    // Work out the topic name. If the category is empty then we're in
    // backwards compatibility mode and the name is just
    // identity.name. Otherwise identity.name is topic.<topicname>.
    //
    if(id.category.empty())
    {
        return id.name;
    }

    assert(id.name.length() > 6 && id.name.compare(0, 6, "topic.") == 0);
    return id.name.substr(6);
}

}

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
        installTopic(identityToTopicName(p->first), p->first, p->second, false);
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

    // Identity is instanceName>/topic.<topicname>
    Ice::Identity id;
    id.category = _instance->instanceName();
    id.name = "topic." + name;

    _topics.put(PersistentTopicMap::value_type(id, LinkRecordSeq()));

    return installTopic(name, id, LinkRecordSeq(), true);
}

TopicPrx
TopicManagerI::retrieve(const string& name, const Ice::Current&) const
{
    IceUtil::Mutex::Lock sync(*this);

    TopicManagerI* This = const_cast<TopicManagerI*>(this);
    This->reap();

    TopicIMap::const_iterator p = _topicIMap.find(name);
    if(p == _topicIMap.end())
    {
        NoSuchTopic ex;
        ex.name = name;
        throw ex;
    }

    // Here we cannot just reconstruct the identity since the
    // identity could be either instanceName/topic name, or if
    // created with pre-3.2 IceStorm / topic name.
    return TopicPrx::uncheckedCast(_topicAdapter->createProxy(p->second->id()));
}

TopicDict
TopicManagerI::retrieveAll(const Ice::Current&) const
{
    IceUtil::Mutex::Lock sync(*this);

    TopicManagerI* This = const_cast<TopicManagerI*>(this);
    This->reap();

    TopicDict all;
    for(TopicIMap::const_iterator p = _topicIMap.begin(); p != _topicIMap.end(); ++p)
    {
        //
        // Here we cannot just reconstruct the identity since the
        // identity could be either "<instanceName>/topic.<topicname>"
        // name, or if created with pre-3.2 IceStorm "/<topicname>".
        //
        all.insert(TopicDict::value_type(
                       p->first, TopicPrx::uncheckedCast(_topicAdapter->createProxy(p->second->id()))));
    }

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
            Ice::Identity id = i->second->id();
            TraceLevelsPtr traceLevels = _instance->traceLevels();
            if(traceLevels->topicMgr > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
                out << "Reaping " << i->first;
            }

            _topics.erase(id);

            try
            {
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

TopicPrx
TopicManagerI::installTopic(const string& name, const Ice::Identity& id, const LinkRecordSeq& rec, bool create)
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
            out << "creating new topic \"" << name << "\". id: "
                << _instance->communicator()->identityToString(id);
        }
        else
        {
            out << "loading topic \"" << name << "\" from database. id: "
                << _instance->communicator()->identityToString(id);
        }
    }

    //
    // Create topic implementation
    //
    TopicIPtr topicI = new TopicI(_instance, name, id, rec, _envName, _dbName);

    //
    // The identity is the name of the Topic.
    //
    TopicPrx prx = TopicPrx::uncheckedCast(_topicAdapter->add(topicI, id));
    _topicIMap.insert(TopicIMap::value_type(name, topicI));
    return prx;
}
