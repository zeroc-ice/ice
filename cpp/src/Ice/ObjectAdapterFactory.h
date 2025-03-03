// Copyright (c) ZeroC, Inc.

#ifndef ICE_OBJECT_ADAPTER_FACTORY_H
#define ICE_OBJECT_ADAPTER_FACTORY_H

#include "ObjectAdapterI.h"

#include <condition_variable>
#include <mutex>
#include <set>

namespace IceInternal
{
    class ObjectAdapterFactory : public std::enable_shared_from_this<ObjectAdapterFactory>
    {
    public:
        void shutdown() noexcept;
        void waitForShutdown() noexcept;
        void waitForShutdownAsync(std::function<void()> completed) noexcept;
        [[nodiscard]] bool isShutdown() const noexcept;
        void destroy() noexcept;

        void updateObservers(void (Ice::ObjectAdapterI::*)());

        Ice::ObjectAdapterPtr createObjectAdapter(
            std::string name,
            std::optional<Ice::RouterPrx>,
            std::optional<Ice::SSL::ServerAuthenticationOptions>);

        Ice::ObjectAdapterPtr findObjectAdapter(const IceInternal::ReferencePtr&);
        void removeObjectAdapter(const Ice::ObjectAdapterPtr&);
        void flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr&, Ice::CompressBatch) const;

        ObjectAdapterFactory(InstancePtr, Ice::CommunicatorPtr);
        virtual ~ObjectAdapterFactory();

    private:
        friend class Instance;

        InstancePtr _instance;
        Ice::CommunicatorPtr _communicator;
        std::set<std::string> _adapterNamesInUse;
        std::list<std::shared_ptr<Ice::ObjectAdapterI>> _adapters;
        std::vector<std::function<void()>> _shutdownCompletedCallbacks;
        std::thread _shutdownCompletedThread;
        bool _shutdownCompleted = false;

        mutable std::recursive_mutex _mutex;
        std::condition_variable_any _conditionVariable;
    };
}

#endif
