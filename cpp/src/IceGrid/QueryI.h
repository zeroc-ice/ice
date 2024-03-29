//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_QUERY_I_H
#define ICE_GRID_QUERY_I_H

#include <IceGrid/Registry.h>
#include "Ice/CommunicatorF.h"

namespace IceGrid
{
    class Database;

    class QueryI final : public Query
    {
    public:
        QueryI(const Ice::CommunicatorPtr&, const std::shared_ptr<Database>&);

        std::optional<Ice::ObjectPrx> findObjectById(Ice::Identity, const Ice::Current&) const override;

        std::optional<Ice::ObjectPrx> findObjectByType(std::string, const Ice::Current&) const override;

        std::optional<Ice::ObjectPrx>
        findObjectByTypeOnLeastLoadedNode(std::string, LoadSample, const Ice::Current&) const override;

        Ice::ObjectProxySeq findAllObjectsByType(std::string, const Ice::Current&) const override;

        Ice::ObjectProxySeq findAllReplicas(std::optional<Ice::ObjectPrx>, const Ice::Current&) const override;

    private:
        const Ice::CommunicatorPtr _communicator;
        const std::shared_ptr<Database> _database;
    };
}

#endif
