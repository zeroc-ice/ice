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

#ifndef FREEZE_STRATEGY_I_H
#define FREEZE_STRATEGY_I_H

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <Freeze/Strategy.h>

#include <list>

namespace Freeze
{

class EvictionStrategyI : virtual public EvictionStrategy
{
public:

    virtual Ice::LocalObjectPtr activatedObject(const Ice::Identity&,
                                                const Ice::ObjectPtr&);

    virtual void destroyedObject(const Ice::Identity&, const Ice::LocalObjectPtr&);

    virtual void evictedObject(const ObjectStorePtr&,
                               const Ice::Identity&,
                               const Ice::ObjectPtr&,
                               const Ice::LocalObjectPtr&);

    virtual void preOperation(const ObjectStorePtr&,
                              const Ice::Identity&,
                              const Ice::ObjectPtr&,
                              bool,
                              const Ice::LocalObjectPtr&);

    virtual void postOperation(const ObjectStorePtr&,
                               const Ice::Identity&,
                               const Ice::ObjectPtr&,
                               bool,
                               const Ice::LocalObjectPtr&);

    virtual void destroy();

private:

    struct Cookie : public Ice::LocalObject
    {
        bool mutated;
    };
    typedef IceUtil::Handle<Cookie> CookiePtr;
};

class IdleStrategyI : virtual public IdleStrategy
{
public:

    virtual Ice::LocalObjectPtr activatedObject(const Ice::Identity&,
                                                const Ice::ObjectPtr&);

    virtual void destroyedObject(const Ice::Identity&, const Ice::LocalObjectPtr&);

    virtual void evictedObject(const ObjectStorePtr&,
                               const Ice::Identity&,
                               const Ice::ObjectPtr&,
                               const Ice::LocalObjectPtr&);

    virtual void preOperation(const ObjectStorePtr&,
                              const Ice::Identity&,
                              const Ice::ObjectPtr&,
                              bool,
                              const Ice::LocalObjectPtr&);

    virtual void postOperation(const ObjectStorePtr&,
                               const Ice::Identity&,
                               const Ice::ObjectPtr&,
                               bool,
                               const Ice::LocalObjectPtr&);

    virtual void destroy();

private:

    struct Cookie : public Ice::LocalObject
    {
        bool mutated;
        Ice::Int mutatingCount;
    };
    typedef IceUtil::Handle<Cookie> CookiePtr;
};

}

#endif
