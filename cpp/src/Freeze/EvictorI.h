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

#ifndef FREEZE_EVICTOR_I_H
#define FREEZE_EVICTOR_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/Evictor.h>
#include <Freeze/Strategy.h>
#include <Freeze/IdentityObjectRecordDict.h>

#include <list>

namespace Freeze
{

class EvictorI : virtual public Evictor, virtual public ObjectStore, virtual public IceUtil::Mutex
{
public:

    EvictorI(const Freeze::DBPtr&, const PersistenceStrategyPtr&);
    virtual ~EvictorI();

    virtual DBPtr getDB();
    virtual PersistenceStrategyPtr getPersistenceStrategy();

    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize();
    
    virtual void createObject(const Ice::Identity&, const Ice::ObjectPtr&);
    virtual void destroyObject(const Ice::Identity&);

    virtual void installServantInitializer(const ServantInitializerPtr&);
    virtual EvictorIteratorPtr getIterator();
    virtual bool hasObject(const Ice::Identity&);

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate();

    virtual void save(const Ice::Identity&, const Ice::ObjectPtr&);

private:

    struct EvictorElement : public Ice::LocalObject
    {
	ObjectRecord rec;
	std::list<Ice::Identity>::iterator position;
	int usageCount;
        bool destroyed;
        Ice::LocalObjectPtr strategyCookie;
    };
    typedef IceUtil::Handle<EvictorElement> EvictorElementPtr;

    void evict();
    EvictorElementPtr add(const Ice::Identity&, const ObjectRecord&);
    EvictorElementPtr remove(const Ice::Identity&);
    
    std::map<Ice::Identity, EvictorElementPtr> _evictorMap;
    std::list<Ice::Identity> _evictorList;
    std::map<Ice::Identity, EvictorElementPtr>::size_type _evictorSize;

    bool _deactivated;
    IdentityObjectRecordDict _dict;
    DBPtr _db;
    PersistenceStrategyPtr _strategy;
    ServantInitializerPtr _initializer;
    int _trace;
};

}

#endif
