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

#ifndef SERVANT_LOCATOR_I_H
#define SERVANT_LOCATOR_I_H

#include <Ice/Ice.h>

class ServantLocatorI : public Ice::ServantLocator
{
public:

    ServantLocatorI();
    virtual ~ServantLocatorI();

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

public:

    bool _deactivated;
};

#endif
