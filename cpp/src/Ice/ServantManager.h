//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SERVANT_MANAGER_H
#define ICE_SERVANT_MANAGER_H

#include <Ice/ServantManagerF.h>
#include <Ice/InstanceF.h>
#include <Ice/Identity.h>
#include <Ice/FacetMap.h>

#include <mutex>

namespace Ice
{

class ObjectAdapterI;
class ServantLocator;

}

namespace IceInternal
{

class ServantManager
{
public:

    ServantManager(const InstancePtr&, const std::string&);
    ~ServantManager();
    void addServant(const std::shared_ptr<Ice::Object>&, const Ice::Identity&, const std::string&);
    void addDefaultServant(const std::shared_ptr<Ice::Object>&, const std::string&);
    std::shared_ptr<Ice::Object> removeServant(const Ice::Identity&, const std::string&);
    std::shared_ptr<Ice::Object> removeDefaultServant(const std::string&);
    Ice::FacetMap removeAllFacets(const Ice::Identity&);
    std::shared_ptr<Ice::Object> findServant(const Ice::Identity&, const std::string&) const;
    std::shared_ptr<Ice::Object> findDefaultServant(const std::string&) const;
    Ice::FacetMap findAllFacets(const Ice::Identity&) const;
    bool hasServant(const Ice::Identity&) const;

    void addServantLocator(const std::shared_ptr<Ice::ServantLocator>& locator, const std::string&);
    std::shared_ptr<Ice::ServantLocator> removeServantLocator(const std::string&);
    std::shared_ptr<Ice::ServantLocator> findServantLocator(const std::string&) const;

private:

    void destroy();
    friend class Ice::ObjectAdapterI;

    InstancePtr _instance;

    const std::string _adapterName;

    typedef std::map<Ice::Identity, Ice::FacetMap> ServantMapMap;
    typedef std::map<std::string, std::shared_ptr<Ice::Object>> DefaultServantMap;

    ServantMapMap _servantMapMap;
    mutable ServantMapMap::iterator _servantMapMapHint;

    DefaultServantMap _defaultServantMap;

    std::map<std::string, std::shared_ptr<Ice::ServantLocator>> _locatorMap;
    mutable std::map<std::string, std::shared_ptr<Ice::ServantLocator>>::iterator _locatorMapHint;
    mutable std::mutex _mutex;
};

}

#endif
