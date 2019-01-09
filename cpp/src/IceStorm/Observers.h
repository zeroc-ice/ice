// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef OBSERVERS_H
#define OBSERVERS_H

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/Election.h>
#include <IceStorm/Replica.h>

#ifdef __SUNPRO_CC
#  pragma error_messages(off,hidef)
#endif

namespace IceStorm
{
class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;
}

namespace IceStormElection
{

class Observers : public IceUtil::Shared, public IceUtil::Mutex
{
public:
    Observers(const IceStorm::InstancePtr&);

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

    const IceStorm::TraceLevelsPtr _traceLevels;
    unsigned int _majority;
    struct ObserverInfo
    {
        ObserverInfo(int i, const ReplicaObserverPrx& o, const Ice::AsyncResultPtr& r = 0) :
            id(i), observer(o), result (r) {}
        int id;
        ReplicaObserverPrx observer;
        ::Ice::AsyncResultPtr result;
    };
    std::vector<ObserverInfo> _observers;
    IceUtil::Mutex _reapedMutex;
    std::vector<int> _reaped;
};
typedef IceUtil::Handle<Observers> ObserversPtr;

}

#ifdef __SUNPRO_CC
#  pragma error_messages(default,hidef)
#endif

#endif // OBSERVERS_H
