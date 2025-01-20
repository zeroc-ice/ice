// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_ADMIN_CALLBACK_ROUTER_H
#define ICEGRID_ADMIN_CALLBACK_ROUTER_H

#include "Ice/Ice.h"
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
        void addMapping(const std::string&, const Ice::ConnectionPtr&);
        void removeMapping(const std::string&);

        void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*>,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current& current) override;

    private:
        std::mutex _mutex;
        std::map<std::string, Ice::ConnectionPtr> _categoryToConnection;
    };
}

#endif
