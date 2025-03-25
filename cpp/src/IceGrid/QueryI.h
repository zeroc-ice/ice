// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_QUERY_I_H
#define ICEGRID_QUERY_I_H

#include "Ice/CommunicatorF.h"
#include "IceGrid/Registry.h"

namespace IceGrid
{
    class Database;

    class QueryI final : public Query
    {
    public:
        QueryI(Ice::CommunicatorPtr, const std::shared_ptr<Database>&);

        [[nodiscard]] std::optional<Ice::ObjectPrx> findObjectById(Ice::Identity, const Ice::Current&) override;

        [[nodiscard]] std::optional<Ice::ObjectPrx> findObjectByType(std::string, const Ice::Current&) override;

        [[nodiscard]] std::optional<Ice::ObjectPrx>
        findObjectByTypeOnLeastLoadedNode(std::string, LoadSample, const Ice::Current&) override;

        [[nodiscard]] Ice::ObjectProxySeq findAllObjectsByType(std::string, const Ice::Current&) override;

        [[nodiscard]] Ice::ObjectProxySeq findAllReplicas(std::optional<Ice::ObjectPrx>, const Ice::Current&) override;

    private:
        const Ice::CommunicatorPtr _communicator;
        const std::shared_ptr<Database> _database;
    };
}

#endif
