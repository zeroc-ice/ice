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

#include <IcePack/Admin.h>
#include <map>

class Forward : public Ice::ObjectLocator
{
public:

    Forward(const IcePack::AdminPtr&);

    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const std::string&, Ice::ObjectPtr&);
    virtual void finished(const Ice::ObjectAdapterPtr&, const std::string&, const Ice::ObjectPtr&,
			  const Ice::ObjectPtr&);

private:

    IcePack::AdminPtr _admin;
};

#endif
