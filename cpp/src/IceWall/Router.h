// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_WALL_ROUTER_H
#define ICE_WALL_ROUTER_H

#include <Ice/Ice.h>

namespace IceWall
{

class Router : public Ice::ServantLocator
{
public:

    Router();
    virtual ~Router();
    
    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::ObjectAdapterPtr&, const Ice::Current&,
			  const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate();
};

}

#endif
