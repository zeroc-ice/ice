//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SERVANT_MANAGER_H
#define ICE_SERVANT_MANAGER_H

#include "ServantManagerF.h"
#include "Ice/InstanceF.h"
#include "Ice/Identity.h"
#include "Ice/FacetMap.h"
#include "Ice/Object.h"
#include "Ice/ServantLocator.h"

#include <mutex>

namespace Ice
{
    class ObjectAdapterI;
}

namespace IceInternal
{
    // A dispatcher that manages servants and servant locators for an object adapter.
    class ServantManager final : public Ice::Object
    {
    public:
        ServantManager(const InstancePtr&, const std::string&);
        ~ServantManager();
        void addServant(const Ice::ObjectPtr&, const Ice::Identity&, const std::string&);
        void addDefaultServant(const Ice::ObjectPtr&, const std::string&);
        Ice::ObjectPtr removeServant(const Ice::Identity&, const std::string&);
        Ice::ObjectPtr removeDefaultServant(const std::string&);
        Ice::FacetMap removeAllFacets(const Ice::Identity&);
        Ice::ObjectPtr findServant(const Ice::Identity&, const std::string&) const;
        Ice::ObjectPtr findDefaultServant(const std::string&) const;
        Ice::FacetMap findAllFacets(const Ice::Identity&) const;
        bool hasServant(const Ice::Identity&) const;

        void addServantLocator(const Ice::ServantLocatorPtr& locator, const std::string&);
        Ice::ServantLocatorPtr removeServantLocator(const std::string&);
        Ice::ServantLocatorPtr findServantLocator(const std::string&) const;

        void dispatch(Ice::IncomingRequest&, std::function<void(Ice::OutgoingResponse)>) final;

    private:
        void destroy();
        friend class Ice::ObjectAdapterI;

        InstancePtr _instance;

        const std::string _adapterName;

        typedef std::map<Ice::Identity, Ice::FacetMap> ServantMapMap;
        typedef std::map<std::string, Ice::ObjectPtr> DefaultServantMap;

        ServantMapMap _servantMapMap;
        mutable ServantMapMap::iterator _servantMapMapHint;

        DefaultServantMap _defaultServantMap;

        std::map<std::string, Ice::ServantLocatorPtr> _locatorMap;
        mutable std::map<std::string, Ice::ServantLocatorPtr>::iterator _locatorMapHint;
        mutable std::mutex _mutex;
    };
}

#endif
