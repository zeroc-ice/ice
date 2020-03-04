//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_QUERY_I_H
#define ICE_GRID_QUERY_I_H

#include <IceGrid/Registry.h>
#include <Ice/CommunicatorF.h>

namespace IceGrid
{

class Database;

class QueryI final : public Query
{
public:

    QueryI(const std::shared_ptr<Ice::Communicator>&, const std::shared_ptr<Database>&);

    std::shared_ptr<Ice::ObjectPrx> findObjectById(Ice::Identity, const Ice::Current&) const override;

    std::shared_ptr<Ice::ObjectPrx> findObjectByType(std::string, const Ice::Current&) const override;

    std::shared_ptr<Ice::ObjectPrx> findObjectByTypeOnLeastLoadedNode(std::string, LoadSample,
                                                                      const Ice::Current&) const override;

    Ice::ObjectProxySeq findAllObjectsByType(std::string, const Ice::Current&) const override;

    Ice::ObjectProxySeq findAllReplicas(std::shared_ptr<Ice::ObjectPrx>, const Ice::Current&) const override;

private:

    const std::shared_ptr<Ice::Communicator> _communicator;
    const std::shared_ptr<Database> _database;
};

}

#endif
