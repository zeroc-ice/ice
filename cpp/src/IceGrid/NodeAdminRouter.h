// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_NODE_ADMIN_ROUTER_H
#define ICEGRID_NODE_ADMIN_ROUTER_H

#include "AdminRouter.h"
#include "NodeI.h"

namespace IceGrid
{
    //
    // Routes requests to a server's admin object through the Node
    //
    class NodeServerAdminRouter final : public AdminRouter
    {
    public:
        NodeServerAdminRouter(const std::shared_ptr<NodeI>&);

        void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*>,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current& current) override;

    private:
        std::shared_ptr<NodeI> _node;
    };
}
#endif
