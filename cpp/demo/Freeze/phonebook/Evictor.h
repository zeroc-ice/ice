// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef EVICTOR_H
#define EVICTOR_H

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <list>

class Evictor;
typedef IceUtil::Handle<Evictor> EvictorPtr;

class Evictor : public Ice::ServantLocator, public JTCMutex
{
public:

    Evictor(const Freeze::DBPtr&, Ice::Int);
    
    void createObject(const std::string&, const Ice::ObjectPtr&);
    void destroyObject(const std::string&);

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
};

#endif
