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
                   const AdapterPrxPtr&, const std::string&, bool);
    ~ServerAdapterI() override;

    void activateAsync(std::function<void(const Ice::ObjectPrx&)>,
                       std::function<void(std::exception_ptr)>,
                       const Ice::Current&) override;
    Ice::ObjectPrx getDirectProxy(const Ice::Current&) const override;
    void setDirectProxy(Ice::ObjectPrx, const ::Ice::Current&) override;

    void destroy();
    void updateEnabled();
    void clear();
    void activationFailed(const std::string&);
    void activationCompleted();

    AdapterPrxPtr getProxy() const;

private:

    const std::shared_ptr<NodeI> _node;
    const AdapterPrxPtr _this;
    const std::string _serverId;
    const std::string _id;
    const std::string _replicaId;
    ServerI* _server;

    Ice::ObjectPrx _proxy;
    bool _enabled;
    std::vector<std::function<void(const Ice::ObjectPrx&)>> _activateCB;
    bool _activateAfterDeactivating;

    mutable std::mutex _mutex;
};

}

#endif
