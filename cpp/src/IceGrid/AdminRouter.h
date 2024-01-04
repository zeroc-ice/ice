//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_ADMIN_ROUTER_H
#define ICE_GRID_ADMIN_ROUTER_H

#include <Ice/Ice.h>
#include <IceGrid/TraceLevels.h>

namespace IceGrid
{

//
// An Admin Router routes requests to an admin object
//
class AdminRouter : public Ice::BlobjectArrayAsync
{
protected:

    AdminRouter(const std::shared_ptr<TraceLevels>&);

    void invokeOnTarget(const std::shared_ptr<Ice::ObjectPrx>&,
                        const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                        std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)>&&,
                        std::function<void(std::exception_ptr)>&&,
                        const Ice::Current&);

    const std::shared_ptr<TraceLevels> _traceLevels;
};

}
#endif
