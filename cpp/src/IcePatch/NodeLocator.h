// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_NODE_LOCATOR_H
#define ICE_PATCH_NODE_LOCATOR_H

#include <IcePatch/NodeI.h>

namespace IcePatch
{

class NodeLocator: virtual public Ice::ServantLocator
{
public:

    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const Ice::Current&, Ice::LocalObjectPtr&);

    virtual void finished(const Ice::ObjectAdapterPtr&, const Ice::Current&, const Ice::ObjectPtr&,
			  const Ice::LocalObjectPtr&);

    virtual void deactivate();
};

}

#endif
