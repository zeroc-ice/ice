//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_SERVER_ADAPTER_I_H
#define ICE_GRID_SERVER_ADAPTER_I_H

#include <IceGrid/Internal.h>

namespace IceGrid
{

class NodeI;
class ServerI;

class ServerAdapterI : public Adapter
{
public:

    ServerAdapterI(const std::shared_ptr<NodeI>&, ServerI*, const std::string&,
                   const std::shared_ptr<AdapterPrx>&, const std::string&, bool);
    ~ServerAdapterI() override;

    void activateAsync(std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                       std::function<void(std::exception_ptr)>,
                       const Ice::Current&) override;
    std::shared_ptr<Ice::ObjectPrx> getDirectProxy(const Ice::Current&) const override;
    void setDirectProxy(std::shared_ptr<Ice::ObjectPrx>, const ::Ice::Current&) override;

    void destroy();
    void updateEnabled();
    void clear();
    void activationFailed(const std::string&);
    void activationCompleted();

    std::shared_ptr<AdapterPrx> getProxy() const;

private:

    const std::shared_ptr<NodeI> _node;
    const std::shared_ptr<AdapterPrx> _this;
    const std::string _serverId;
    const std::string _id;
    const std::string _replicaId;
    ServerI* _server;

    std::shared_ptr<Ice::ObjectPrx> _proxy;
    bool _enabled;
    std::vector<std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>> _activateCB;
    bool _activateAfterDeactivating;

    mutable std::mutex _mutex;
};

}

#endif
