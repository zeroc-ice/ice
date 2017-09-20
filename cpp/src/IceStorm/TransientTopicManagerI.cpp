// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/TransientTopicManagerI.h>
#include <IceStorm/TransientTopicI.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Instance.h>
#include <IceStorm/Subscriber.h>

#include <Ice/Ice.h>

#include <Ice/SliceChecksums.h>

#include <functional>

using namespace IceStorm;
using namespace std;

TransientTopicManagerImpl::TransientTopicManagerImpl(const InstancePtr& instance) :
    _instance(instance)
{
}

TransientTopicManagerImpl::~TransientTopicManagerImpl()
{
}

TopicPrx
TransientTopicManagerImpl::create(const string& name, const Ice::Current&)
{
    Lock sync(*this);

    reap();

    if(_topics.find(name) != _topics.end())
    {
        throw TopicExists(name);
    }

    Ice::Identity id = IceStormInternal::nameToIdentity(_instance, name);

    //
    // Called by constructor or with 'this' mutex locked.
    //
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topicMgr > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
        out << "creating new topic \"" << name << "\". id: "
            << _instance->communicator()->identityToString(id);
    }

    //
    // Create topic implementation
    //
    TransientTopicImplPtr topicImpl = new TransientTopicImpl(_instance, name, id);

    //
    // The identity is the name of the Topic.
    //
    TopicPrx prx = TopicPrx::uncheckedCast(_instance->topicAdapter()->add(topicImpl, id));
    _topics.insert(map<string, TransientTopicImplPtr>::value_type(name, topicImpl));
    return prx;
}

TopicPrx
TransientTopicManagerImpl::retrieve(const string& name, const Ice::Current&) const
{
    Lock sync(*this);

    TransientTopicManagerImpl* This = const_cast<TransientTopicManagerImpl*>(this);
    This->reap();

    map<string, TransientTopicImplPtr>::const_iterator p = _topics.find(name);
    if(p == _topics.end())
    {
        throw NoSuchTopic(name);
    }

    // Here we cannot just reconstruct the identity since the
    // identity could be either instanceName/topic name, or if
    // created with pre-3.2 IceStorm / topic name.
    return TopicPrx::uncheckedCast(_instance->topicAdapter()->createProxy(p->second->id()));
}

TopicDict
TransientTopicManagerImpl::retrieveAll(const Ice::Current&) const
{
    Lock sync(*this);

    TransientTopicManagerImpl* This = const_cast<TransientTopicManagerImpl*>(this);
    This->reap();

    TopicDict all;
    for(map<string, TransientTopicImplPtr>::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
    {
        //
        // Here we cannot just reconstruct the identity since the
        // identity could be either "<instanceName>/topic.<topicname>"
        // name, or if created with pre-3.2 IceStorm "/<topicname>".
        //
        all.insert(TopicDict::value_type(
                       p->first, TopicPrx::uncheckedCast(_instance->topicAdapter()->createProxy(p->second->id()))));
    }

    return all;
}
Ice::SliceChecksumDict
TransientTopicManagerImpl::getSliceChecksums(const Ice::Current&) const
{
    return Ice::sliceChecksums();
}

IceStormElection::NodePrx
TransientTopicManagerImpl::getReplicaNode(const Ice::Current&) const
{
    return IceStormElection::NodePrx();
}

void
TransientTopicManagerImpl::reap()
{
    //
    // Always called with mutex locked.
    //
    // Lock sync(*this);
    //
    vector<string> reaped = _instance->topicReaper()->consumeReapedTopics();
    for(vector<string>::const_iterator p = reaped.begin(); p != reaped.end(); ++p)
    {
        map<string, TransientTopicImplPtr>::iterator i = _topics.find(*p);
        if(i != _topics.end() && i->second->destroyed())
        {
            Ice::Identity id = i->second->id();
            TraceLevelsPtr traceLevels = _instance->traceLevels();
            if(traceLevels->topicMgr > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
                out << "Reaping " << i->first;
            }

            try
            {
                _instance->topicAdapter()->remove(id);
            }
            catch(const Ice::ObjectAdapterDeactivatedException&)
            {
                // Ignore
            }

            _topics.erase(i);
        }
    }
}

void
TransientTopicManagerImpl::shutdown()
{
    Lock sync(*this);

    for(map<string, TransientTopicImplPtr>::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
    {
        p->second->shutdown();
    }
}
