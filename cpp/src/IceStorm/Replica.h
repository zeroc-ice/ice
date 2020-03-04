//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef REPLICA_H
#define REPLICA_H

#include <Ice/Ice.h>
#include <IceStorm/Election.h>
#include <set>

namespace IceStormElection
{

struct GroupNodeInfo
{
    explicit GroupNodeInfo(int i);
    GroupNodeInfo(int i, LogUpdate l, std::shared_ptr<Ice::ObjectPrx> o = nullptr);

    bool operator<(const GroupNodeInfo& rhs) const;
    bool operator==(const GroupNodeInfo& rhs) const;

    const int id;
    const LogUpdate llu;
    const std::shared_ptr<Ice::ObjectPrx> observer;
};

class Replica
{
public:

    virtual LogUpdate getLastLogUpdate() const = 0;
    virtual void sync(const std::shared_ptr<Ice::ObjectPrx>&) = 0;
    virtual void initMaster(const std::set<IceStormElection::GroupNodeInfo>&, const LogUpdate&) = 0;
    virtual std::shared_ptr<Ice::ObjectPrx> getObserver() const = 0;
    virtual std::shared_ptr<Ice::ObjectPrx> getSync() const = 0;
};

}

#endif // RELICA_H
