// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_ADMIN_ROUTER_H
#define ICEGRID_ADMIN_ROUTER_H

#include "Ice/Ice.h"
#include "TraceLevels.h"

namespace IceGrid
{
    //
    // An Admin Router routes requests to an admin object
    //
    class AdminRouter : public Ice::BlobjectArrayAsync
    {
    protected:
        AdminRouter(const std::shared_ptr<TraceLevels>&);

        void invokeOnTarget(
            const Ice::ObjectPrx&,
            std::pair<const std::byte*, const std::byte*>,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)>&&,
            std::function<void(std::exception_ptr)>&&,
            const Ice::Current&);

        const std::shared_ptr<TraceLevels> _traceLevels;
    };
}
#endif
