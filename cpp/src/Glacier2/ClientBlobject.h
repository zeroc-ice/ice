// Copyright (c) ZeroC, Inc.

#ifndef CLIENT_BLOBJECT_H
#define CLIENT_BLOBJECT_H

#include "Blobject.h"
#include "Glacier2/Session.h"

namespace Glacier2
{
    class FilterManager;
    class RoutingTable;

    class ClientBlobject final : public Glacier2::Blobject
    {
    public:
        ClientBlobject(
            std::shared_ptr<Instance>,
            std::shared_ptr<FilterManager>,
            const Ice::Context&,
            std::shared_ptr<RoutingTable>);

        void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*> inEncaps,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)> response,
            std::function<void(std::exception_ptr)> error,
            const Ice::Current& current) final;

        std::shared_ptr<StringSet> categories();
        std::shared_ptr<StringSet> adapterIds();
        std::shared_ptr<IdentitySet> identities();

    private:
        const std::shared_ptr<RoutingTable> _routingTable;
        const std::shared_ptr<FilterManager> _filters;
        const int _rejectTraceLevel;
    };
}

#endif
