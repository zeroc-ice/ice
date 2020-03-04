//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_NODE_ADMIN_ROUTER_H
#define ICE_GRID_NODE_ADMIN_ROUTER_H

#include <IceGrid/NodeI.h>
#include <IceGrid/AdminRouter.h>

namespace IceGrid
{

//
// Routes requests to a server's admin object through the Node
//
class NodeServerAdminRouter final : public AdminRouter
{
public:

    NodeServerAdminRouter(const std::shared_ptr<NodeI>&);

    void ice_invokeAsync(std::pair<const Ice::Byte*, const Ice::Byte*>,
                         std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)>,
                         std::function<void(std::exception_ptr)>,
                         const Ice::Current& current) override;

private:

    std::shared_ptr<NodeI> _node;
};

}
#endif
