// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_FORWARD_H
#define ICE_PACK_FORWARD_H

#include <IcePack/AdminF.h>
#ifndef _WIN32
#   include <IcePack/Activator.h>
#endif

namespace IcePack
{

class Forward : public Ice::ServantLocator
{
public:

    Forward(const Ice::CommunicatorPtr& communicator, const AdminPtr&);
    virtual ~Forward();
    
    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::ObjectAdapterPtr&, const Ice::Current&,
			  const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate();

private:

    Ice::CommunicatorPtr _communicator;
    AdminPtr _admin;
#ifndef _WIN32
    ActivatorPtr _activator;
    int _waitTime;
#endif
};

}

#endif
