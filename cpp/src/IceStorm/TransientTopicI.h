//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TRANSIENT_TOPIC_I_H
#define TRANSIENT_TOPIC_I_H

#include <IceStorm/IceStormInternal.h>

namespace IceStorm
{

// Forward declarations.
class Instance;
class Subscriber;

class TransientTopicImpl : public TopicInternal
{
public:

    static std::shared_ptr<TransientTopicImpl> create(const std::shared_ptr<Instance>&, const std::string&,
                                                      const Ice::Identity&);

    std::string getName(const Ice::Current&) const override;
    std::shared_ptr<Ice::ObjectPrx> getNonReplicatedPublisher(const Ice::Current&) const override;
    std::shared_ptr<Ice::ObjectPrx> getPublisher(const Ice::Current&) const override;
    std::shared_ptr<Ice::ObjectPrx> subscribeAndGetPublisher(QoS, std::shared_ptr<Ice::ObjectPrx>, const Ice::Current&) override;
    void unsubscribe(std::shared_ptr<Ice::ObjectPrx>, const Ice::Current&) override;
    std::shared_ptr<TopicLinkPrx> getLinkProxy(const Ice::Current&) override;
    void link(std::shared_ptr<TopicPrx>, int, const Ice::Current&) override;
    void unlink(std::shared_ptr<TopicPrx>, const Ice::Current&) override;
    LinkInfoSeq getLinkInfoSeq(const Ice::Current&) const override;
    Ice::IdentitySeq getSubscribers(const Ice::Current&) const override;
    void destroy(const Ice::Current&) override;
    void reap(Ice::IdentitySeq, const Ice::Current&) override;

    // Internal methods
    bool destroyed() const;
    Ice::Identity id() const;
    void publish(bool, const EventDataSeq&);

    void shutdown();

private:

    TransientTopicImpl(std::shared_ptr<Instance>, const std::string&, const Ice::Identity&);

    //
    // Immutable members.
    //
    const std::shared_ptr<Instance> _instance;
    const std::string _name; // The topic name
    const Ice::Identity _id; // The topic identity

    std::shared_ptr<Ice::ObjectPrx> _publisherPrx;
    std::shared_ptr<TopicLinkPrx> _linkPrx;

    //
    // We keep a vector of subscribers since the optimized behaviour
    // should be publishing events, not searching through the list of
    // subscribers for a particular subscriber. I tested
    // vector/list/map and although there was little difference vector
    // was the fastest of the three.
    //
    std::vector<std::shared_ptr<Subscriber>> _subscribers;

    bool _destroyed; // Has this Topic been destroyed?

    mutable std::mutex _mutex;
};

} // End namespace IceStorm

#endif
