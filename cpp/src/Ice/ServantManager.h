// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

namespace Ice
{

class ObjectAdapterI;

}

namespace IceInternal
{

class ServantManager : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    void addServant(const Ice::ObjectPtr&, const Ice::Identity&);
    void removeServant(const Ice::Identity&);
    Ice::ObjectPtr findServant(const Ice::Identity&) const;

    void addServantLocator(const Ice::ServantLocatorPtr& locator, const std::string&);
    Ice::ServantLocatorPtr findServantLocator(const std::string&) const;

private:

    ServantManager(const InstancePtr&, const std::string&);
    ~ServantManager();
    void destroy();
    friend class Ice::ObjectAdapterI;

    InstancePtr _instance;

    const std::string _adapterName;

    Ice::ObjectDict _servantMap;
    mutable Ice::ObjectDict::iterator _servantMapHint;

    std::map<std::string, Ice::ServantLocatorPtr> _locatorMap;
    mutable std::map<std::string, Ice::ServantLocatorPtr>::iterator _locatorMapHint;
};

}

#endif
