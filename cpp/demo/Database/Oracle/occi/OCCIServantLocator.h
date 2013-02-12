// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef OCCI_SERVANT_LOCATOR_H
#define OCCI_SERVANT_LOCATOR_H

#include <DeptFactoryI.h>
#include <Ice/ServantLocator.h>
#include <occi.h>

class OCCIServantLocator : public Ice::ServantLocator
{
public:
    
    OCCIServantLocator(const DeptFactoryIPtr&);

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);
  
private:
    DeptFactoryIPtr _factory;
};

#endif
