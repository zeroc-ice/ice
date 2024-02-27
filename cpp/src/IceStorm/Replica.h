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
        GroupNodeInfo(int i, LogUpdate l, Ice::ObjectPrxPtr o = std::nullopt);

        bool operator<(const GroupNodeInfo& rhs) const;
        bool operator==(const GroupNodeInfo& rhs) const;

        const int id;
        const LogUpdate llu;
        const Ice::ObjectPrxPtr observer;
    };

    class Replica
    {
    public:
        virtual LogUpdate getLastLogUpdate() const = 0;
        virtual void sync(const Ice::ObjectPrxPtr&) = 0;
        virtual void initMaster(const std::set<IceStormElection::GroupNodeInfo>&, const LogUpdate&) = 0;
        virtual Ice::ObjectPrxPtr getObserver() const = 0;
        virtual Ice::ObjectPrxPtr getSync() const = 0;
    };

}

#endif // RELICA_H
