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

class Evictor : public Ice::ServantLocator, public JTCMutex
{
public:

    Evictor(const Freeze::DBPtr&, Ice::Int);
    
    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const std::string&, Ice::ObjectPtr&);
    virtual void finished(const Ice::ObjectAdapterPtr&, const std::string&, const Ice::ObjectPtr&,
			  const Ice::ObjectPtr&);

private:

    Freeze::DBPtr _db;

    struct EvictorEntry
    {
	Ice::ObjectPtr _servant;
	std::list<std::string>::iterator _position;
    };
    std::map<std::string, EvictorEntry> _evictorMap;
    std::list<std::string> _evictorList;
    std::map<std::string, EvictorEntry>::size_type _evictorSize;
};

#endif
