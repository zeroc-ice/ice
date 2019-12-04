//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CLIENT_BLOBJECT_H
#define CLIENT_BLOBJECT_H

#include <Glacier2/Blobject.h>
#include <Glacier2/Session.h>

namespace Glacier2
{

class FilterManager;
class RoutingTable;

class ClientBlobject final : public Glacier2::Blobject
{
public:

    ClientBlobject(std::shared_ptr<Instance>, std::shared_ptr<FilterManager>, const Ice::Context&,
                   std::shared_ptr<RoutingTable>);

    void ice_invokeAsync(std::pair<const Ice::Byte*, const Ice::Byte*> inEncaps,
                         std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                         std::function<void(std::exception_ptr)> error,
                         const Ice::Current& current) override;

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
