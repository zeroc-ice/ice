// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_EVICTOR_I_H
#define FREEZE_EVICTOR_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/DB.h>
#include <Freeze/Evictor.h>
#include <list>

namespace Freeze
{

class EvictorI : public Evictor, public JTCMutex
{
public:

    EvictorI(const Freeze::DBPtr&, EvictorPersistenceMode);
    virtual ~EvictorI();

    virtual DBPtr getDB();

    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize();
    
    virtual void createObject(const std::string&, const Ice::ObjectPtr&);
    virtual void destroyObject(const std::string&);

    virtual void installServantInitializer(const ServantInitializerPtr&);

    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const std::string&, const std::string&,
				  Ice::LocalObjectPtr&);
    virtual void finished(const Ice::ObjectAdapterPtr&, const std::string&, const Ice::ObjectPtr&, const std::string&,
			  const Ice::LocalObjectPtr&);
    virtual void deactivate();

private:

    struct EvictorElement : public Ice::LocalObject
    {
	Ice::ObjectPtr servant;
	std::list<std::string>::iterator position;
	int usageCount;
    };
    typedef IceUtil::Handle<EvictorElement> EvictorElementPtr;

    void evict();
    EvictorElementPtr add(const std::string&, const Ice::ObjectPtr&);
    void remove(const std::string&);
    
    std::map<std::string, EvictorElementPtr> _evictorMap;
    std::list<std::string> _evictorList;
    std::map<std::string, EvictorElementPtr>::size_type _evictorSize;

    bool _deactivated;
    Freeze::DBPtr _db;
    EvictorPersistenceMode _persistenceMode;
    ServantInitializerPtr _initializer;
    int _trace;
};

}

#endif
