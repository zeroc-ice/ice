// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_SERVANT_MANAGER_H
#define ICEE_SERVANT_MANAGER_H

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/ServantManagerF.h>
#include <IceE/InstanceF.h>
#include <IceE/Identity.h>
#include <IceE/FacetMap.h>

namespace IceEInternal
{

class ServantManager : public IceE::Shared, public IceE::Mutex
{
public:

    void addServant(const IceE::ObjectPtr&, const IceE::Identity&, const std::string&);
    IceE::ObjectPtr removeServant(const IceE::Identity&, const std::string&);
    IceE::FacetMap removeAllFacets(const IceE::Identity&);
    IceE::ObjectPtr findServant(const IceE::Identity&, const std::string&) const;
    IceE::FacetMap findAllFacets(const IceE::Identity&) const;
    bool hasServant(const IceE::Identity&) const;

private:

    ServantManager(const InstancePtr&, const std::string&);
    ~ServantManager();
    void destroy();
    friend class IceE::ObjectAdapter;

    InstancePtr _instance;

    const std::string _adapterName;

    typedef std::map<IceE::Identity, IceE::FacetMap> ServantMapMap;

    ServantMapMap _servantMapMap;
    mutable ServantMapMap::iterator _servantMapMapHint;
};

}

#endif
