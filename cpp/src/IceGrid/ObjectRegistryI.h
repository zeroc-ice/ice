// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_OBJECT_REGISTRY_I_H
#define ICE_GRID_OBJECT_REGISTRY_I_H

#include <IceGrid/Internal.h>
#include <IceGrid/IdentityObjectDescDict.h>
#include <IceGrid/StringObjectProxySeqDict.h>
#include <Freeze/ConnectionF.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ObjectRegistryI : public ObjectRegistry, public IceUtil::Mutex
{
public:

    ObjectRegistryI(const Ice::CommunicatorPtr& communicator, const std::string&, const TraceLevelsPtr& traceLevels);

    virtual void add(const IceGrid::ObjectDescriptor&, const ::Ice::Current&);
    virtual void update(const Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void remove(const Ice::Identity&, const ::Ice::Current&);

    virtual ObjectDescriptor getObjectDescriptor(const Ice::Identity&, const ::Ice::Current&) const;

    virtual Ice::ObjectPrx findById(const ::Ice::Identity&, const ::Ice::Current&) const;
    virtual Ice::ObjectPrx findByType(const std::string&, const ::Ice::Current&) const;
    virtual Ice::ObjectProxySeq findAllWithType(const std::string&, const ::Ice::Current&) const;
    virtual ObjectDescriptorSeq findAll(const std::string&, const ::Ice::Current&) const;
    
private:

    static const std::string _objectsDbName;
    static const std::string _typesDbName;

    Freeze::ConnectionPtr _connectionCache;
    IdentityObjectDescDict _objectsCache;
    StringObjectProxySeqDict _typesCache;
    const TraceLevelsPtr _traceLevels;
    const std::string _envName;
    const Ice::CommunicatorPtr _communicator;
};

}

#endif
