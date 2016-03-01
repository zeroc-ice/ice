// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SERVANT_MANAGER_H
#define ICE_SERVANT_MANAGER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ServantManagerF.h>
#include <Ice/InstanceF.h>
#include <Ice/ServantLocatorF.h>
#include <Ice/Identity.h>
#include <Ice/FacetMap.h>

namespace Ice
{

class ObjectAdapterI;

}

namespace IceInternal
{

class ServantManager : public IceUtil::Shared, public IceUtil::Mutex
{
public:

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

private:

    ServantManager(const InstancePtr&, const std::string&);
    ~ServantManager();
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
};

}

#endif
