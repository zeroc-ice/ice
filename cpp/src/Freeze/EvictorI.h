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

class EvictorI;
typedef IceUtil::Handle<EvictorI> EvictorIPtr;

class EvictorI : public Evictor, public JTCMutex
{
public:

    EvictorI(const Freeze::DBPtr&);
    
    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize();
    
    virtual void createObject(const std::string&, const Ice::ObjectPtr&);
    virtual void destroyObject(const std::string&);

    virtual void installServantInitializer(const ServantInitializerPtr&);

    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const std::string&, Ice::ObjectPtr&);
    virtual void finished(const Ice::ObjectAdapterPtr&, const std::string&, const Ice::ObjectPtr&,
			  const Ice::ObjectPtr&);
    virtual void deactivate();

private:

    void evict();
    void add(const std::string&, const Ice::ObjectPtr&);

    Freeze::DBPtr _db;

    struct EvictorElement
    {
	Ice::ObjectPtr servant;
	std::list<std::string>::iterator position;
    };
    std::map<std::string, EvictorElement> _evictorMap;
    std::list<std::string> _evictorList;
    std::map<std::string, EvictorElement>::size_type _evictorSize;

    ServantInitializerPtr _initializer;
};

}

#endif
