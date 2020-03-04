//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_LOCATOR_I_H
#define ICE_GRID_LOCATOR_I_H

#include <IceGrid/Internal.h>
#include <IceGrid/Registry.h>

#include <set>

namespace IceGrid
{

class Database;
class LocatorI;
class TraceLevels;
class WellKnownObjectsManager;

struct LocatorAdapterInfo;
typedef std::vector<LocatorAdapterInfo> LocatorAdapterInfoSeq;

class LocatorI : public Locator, public std::enable_shared_from_this<LocatorI>
{
public:

    class Request : public std::enable_shared_from_this<Request>
    {
    public:
        virtual void execute() = 0;
        virtual void activating(const std::string&) = 0;
        virtual void response(const std::string&, const std::shared_ptr<Ice::ObjectPrx>&) = 0;
        virtual void exception(const std::string&, std::exception_ptr) = 0;
    };

    LocatorI(const std::shared_ptr<Ice::Communicator>&, const std::shared_ptr<Database>&,
             const std::shared_ptr<WellKnownObjectsManager>&, const std::shared_ptr<RegistryPrx>&,
             const std::shared_ptr<QueryPrx>&);

    void findObjectByIdAsync(Ice::Identity,
                            std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                            std::function<void(std::exception_ptr)>,
                            const Ice::Current&) const override;

    void findAdapterByIdAsync(std::string,
                              std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                              std::function<void(std::exception_ptr)>,
                              const Ice::Current&) const override;

    std::shared_ptr<Ice::LocatorRegistryPrx> getRegistry(const Ice::Current&) const override;
    std::shared_ptr<RegistryPrx> getLocalRegistry(const Ice::Current&) const override;
    std::shared_ptr<QueryPrx> getLocalQuery(const Ice::Current&) const override;

    const std::shared_ptr<Ice::Communicator>& getCommunicator() const;
    const std::shared_ptr<TraceLevels>& getTraceLevels() const;

    bool getDirectProxy(const LocatorAdapterInfo&, const std::shared_ptr<Request>&);
    void getDirectProxyResponse(const LocatorAdapterInfo&, const std::shared_ptr<Ice::ObjectPrx>&);
    void getDirectProxyException(const LocatorAdapterInfo&, std::exception_ptr);

protected:

    const std::shared_ptr<Ice::Communicator> _communicator;
    const std::shared_ptr<Database> _database;
    const std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
    const std::shared_ptr<RegistryPrx> _localRegistry;
    const std::shared_ptr<QueryPrx> _localQuery;

    using PendingRequests = std::vector<std::shared_ptr<Request>>;
    using PendingRequestsMap = std::map<std::string, PendingRequests>;
    PendingRequestsMap _pendingRequests;
    std::set<std::string> _activating;

    std::mutex _mutex;
};

}

#endif
