// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
};

#endif
