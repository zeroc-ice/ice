// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_SERVANT_MANAGER_H
#define ICEE_SERVANT_MANAGER_H

#include <IceE/ServantManagerF.h>
#include <IceE/InstanceF.h>
#include <IceE/ObjectAdapter.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Identity.h>

namespace IceInternal
{

class ServantManager : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    void addServant(const Ice::ObjectPtr&, const Ice::Identity&, const std::string&);
    Ice::ObjectPtr removeServant(const Ice::Identity&, const std::string&);
    Ice::FacetMap removeAllFacets(const Ice::Identity&);
    Ice::ObjectPtr findServant(const Ice::Identity&, const std::string&) const;
    Ice::FacetMap findAllFacets(const Ice::Identity&) const;
    bool hasServant(const Ice::Identity&) const;

private:

    ServantManager(const InstancePtr&, const std::string&);
    ~ServantManager();
    void destroy();
    friend class Ice::ObjectAdapter;

    InstancePtr _instance;

    const std::string _adapterName;

    typedef std::map<Ice::Identity, Ice::FacetMap> ServantMapMap;

    ServantMapMap _servantMapMap;
    mutable ServantMapMap::iterator _servantMapMapHint;
};

}

#endif
