// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

#ifdef ICE_CPP11_MAPPING
    virtual Ice::ObjectPtr locate(const Ice::Current&, ::std::shared_ptr<void>&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const ::std::shared_ptr<void>&);
#else
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
#endif
    virtual void deactivate(const std::string&);

public:

    bool _deactivated;
    Ice::ObjectPtr _router;
};

#endif
