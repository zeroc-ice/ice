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
#include <IcePack/Activator.h>
#include <map>

namespace IcePack
{

class Forward : public Ice::ObjectLocator
{
public:

    Forward(const Ice::CommunicatorPtr& communicator, const AdminPtr&);
    virtual ~Forward();
    
    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const std::string&, Ice::ObjectPtr&);
    virtual void finished(const Ice::ObjectAdapterPtr&, const std::string&, const Ice::ObjectPtr&,
			  const Ice::ObjectPtr&);

private:

    Ice::CommunicatorPtr _communicator;
    AdminPtr _admin;
    ActivatorHandle _activator;
};

}

#endif
