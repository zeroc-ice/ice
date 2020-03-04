//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_ADMIN_CALLBACK_ROUTER_H
#define ICE_GRID_ADMIN_CALLBACK_ROUTER_H

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <map>

namespace IceGrid
{

//
// The AdminCallbackRouter routes callbacks from the servers, nodes etc. to the
// admin clients using the admin-client => registry connection.
//

class AdminCallbackRouter : public Ice::BlobjectArrayAsync
{
public:

    void addMapping(const std::string&, const std::shared_ptr<Ice::Connection>&);
    void removeMapping(const std::string&);

    void ice_invokeAsync(std::pair<const Ice::Byte*, const Ice::Byte*>,
                         std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)>,
                         std::function<void(std::exception_ptr)>,
                         const Ice::Current& current) override;

private:

    std::mutex _mutex;
    std::map<std::string, std::shared_ptr<Ice::Connection>> _categoryToConnection;
};

}

#endif
