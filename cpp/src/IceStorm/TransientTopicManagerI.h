//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TRANSIENT_TOPIC_MANAGER_I_H
#define TRANSIENT_TOPIC_MANAGER_I_H

#include <IceStorm/IceStormInternal.h>

namespace IceStorm
{

//
// Forward declarations.
//
class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class TransientTopicImpl;
typedef IceUtil::Handle<TransientTopicImpl> TransientTopicImplPtr;

//
// TopicManager implementation.
//
class TransientTopicManagerImpl : public TopicManagerInternal, public IceUtil::Mutex
{
public:

    TransientTopicManagerImpl(const InstancePtr&);
    ~TransientTopicManagerImpl();

    // TopicManager methods.
    virtual TopicPrx create(const std::string&, const Ice::Current&);
    virtual TopicPrx retrieve(const std::string&, const Ice::Current&) const;
    virtual TopicDict retrieveAll(const Ice::Current&) const;
    virtual IceStormElection::NodePrx getReplicaNode(const Ice::Current&) const;

    void reap();

    void shutdown();

    Ice::ObjectPtr getServant() const;

private:

    const InstancePtr _instance;
    std::map<std::string, TransientTopicImplPtr> _topics;
};
typedef IceUtil::Handle<TransientTopicManagerImpl> TransientTopicManagerImplPtr;

} // End namespace IceStorm

#endif
