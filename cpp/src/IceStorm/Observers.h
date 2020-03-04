//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef OBSERVERS_H
#define OBSERVERS_H

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/Election.h>
#include <IceStorm/Replica.h>

namespace IceStorm
{
class TraceLevels;
}

namespace IceStormElection
{

class Observers
{
public:
    Observers(std::shared_ptr<IceStorm::TraceLevels>);

    void setMajority(unsigned int);

    // Check that we have enough nodes for replication.
    bool check();
    void clear();

    void init(const std::set<IceStormElection::GroupNodeInfo>&, const LogUpdate&, const TopicContentSeq&);
    void createTopic(const LogUpdate&, const std::string&);
    void destroyTopic(const LogUpdate&, const std::string&);
    void addSubscriber(const LogUpdate&, const std::string&, const IceStorm::SubscriberRecord&);
    void removeSubscriber(const LogUpdate&, const std::string&, const Ice::IdentitySeq&);
    void getReapedSlaves(std::vector<int>&);

private:

    void wait(const std::string&);

    const std::shared_ptr<IceStorm::TraceLevels> _traceLevels;
    unsigned int _majority;
    struct ObserverInfo
    {
        int id;
        std::shared_ptr<ReplicaObserverPrx> observer;
        std::future<void> future;
    };
    std::vector<ObserverInfo> _observers;

    std::mutex _reapedMutex;
    std::vector<int> _reaped;

    std::mutex _mutex;
};

}

#endif // OBSERVERS_H
