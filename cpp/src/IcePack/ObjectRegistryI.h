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

#ifndef ICE_PACK_OBJECT_REGISTRY_I_H
#define ICE_PACK_OBJECT_REGISTRY_I_H

#include <IcePack/Internal.h>
#include <IcePack/IdentityObjectDescDict.h>
#include <IcePack/StringObjectProxySeqDict.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ObjectRegistryI : public ObjectRegistry, public IceUtil::Mutex
{
public:

    ObjectRegistryI(const Ice::CommunicatorPtr& communicator,
		    const std::string& envName,
		    const std::string& objectsDbName,
		    const std::string& typesDbName,
		    const TraceLevelsPtr& traceLevels);

    virtual void add(const IcePack::ObjectDescription&, const ::Ice::Current&);
    virtual void remove(const Ice::ObjectPrx&, const ::Ice::Current&);

    virtual ObjectDescription getObjectDescription(const Ice::Identity&, const ::Ice::Current&) const;

    virtual Ice::ObjectPrx findById(const ::Ice::Identity&, const ::Ice::Current&) const;
    virtual Ice::ObjectPrx findByType(const std::string&, const ::Ice::Current&) const;
    virtual Ice::ObjectProxySeq findAllWithType(const std::string&, const ::Ice::Current&) const;

private:

    IdentityObjectDescDict _objects;
    StringObjectProxySeqDict _types;
    TraceLevelsPtr _traceLevels;
};

}

#endif
