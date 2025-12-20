// Copyright (c) ZeroC, Inc.

#ifndef ICE_SERVANT_MANAGER_H
#define ICE_SERVANT_MANAGER_H

#include "Ice/FacetMap.h"
#include "Ice/Identity.h"
#include "Ice/InstanceF.h"
#include "Ice/Object.h"
#include "Ice/ServantLocator.h"
#include "ServantManagerF.h"

#include <mutex>

namespace Ice
{
    class ObjectAdapterI;
}

namespace IceInternal
{
    /// A dispatcher that manages servants and servant locators for an object adapter.
    class ServantManager final : public Ice::Object
    {
    public:
        ServantManager(InstancePtr, std::string);
        ~ServantManager() override;
        void addServant(Ice::ObjectPtr, Ice::Identity, std::string);
        void addDefaultServant(Ice::ObjectPtr, std::string);
        Ice::ObjectPtr removeServant(const Ice::Identity&, std::string_view);
        Ice::ObjectPtr removeDefaultServant(std::string_view);
        Ice::FacetMap removeAllFacets(const Ice::Identity&);
        [[nodiscard]] Ice::ObjectPtr findServant(const Ice::Identity&, std::string_view) const;
        [[nodiscard]] Ice::ObjectPtr findDefaultServant(std::string_view) const;
        [[nodiscard]] Ice::FacetMap findAllFacets(const Ice::Identity&) const;
        [[nodiscard]] bool hasServant(const Ice::Identity&) const;

        void addServantLocator(Ice::ServantLocatorPtr locator, std::string);
        Ice::ServantLocatorPtr removeServantLocator(std::string_view);
        [[nodiscard]] Ice::ServantLocatorPtr findServantLocator(std::string_view) const;

        void dispatch(Ice::IncomingRequest&, std::function<void(Ice::OutgoingResponse)>) final;

    private:
        void destroy();
        friend class Ice::ObjectAdapterI;

        InstancePtr _instance;

        const std::string _adapterName;

        using ServantMapMap = std::map<Ice::Identity, Ice::FacetMap>;
        using DefaultServantMap = std::map<std::string, Ice::ObjectPtr, std::less<>>;

        ServantMapMap _servantMapMap;
        mutable ServantMapMap::iterator _servantMapMapHint;

        DefaultServantMap _defaultServantMap;

        std::map<std::string, Ice::ServantLocatorPtr, std::less<>> _locatorMap;
        mutable std::map<std::string, Ice::ServantLocatorPtr, std::less<>>::iterator _locatorMapHint;
        mutable std::mutex _mutex;
    };
}

#endif
