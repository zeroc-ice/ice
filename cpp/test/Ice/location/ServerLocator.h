// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef SERVER_LOCATOR_H
#define SERVER_LOCATOR_H

#include <Ice/Locator.h>
#include <Ice/ProxyF.h>

class ServerLocatorRegistry : public Ice::LocatorRegistry
{
public:

    ServerLocatorRegistry();
    
    virtual void setAdapterDirectProxy(const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);

    //
    // Internal method
    //
    ::Ice::ObjectPrx getAdapter(const ::std::string&) const;
    ::Ice::ObjectPrx getObject(const ::Ice::Identity&) const;
    void addObject(const ::Ice::ObjectPrx&);

private:
    
    ::std::map< ::std::string, ::Ice::ObjectPrx> _adapters;
    ::std::map< ::Ice::Identity, ::Ice::ObjectPrx> _objects;
};

typedef ::IceInternal::Handle< ServerLocatorRegistry> ServerLocatorRegistryPtr;

class ServerLocator : public ::Ice::Locator
{
public:

    ServerLocator(const ::ServerLocatorRegistryPtr&, const ::Ice::LocatorRegistryPrx&);

    virtual ::Ice::ObjectPrx findObjectById(const Ice::Identity&, const ::Ice::Current&) const;

    virtual ::Ice::ObjectPrx findAdapterById(const std::string&, const ::Ice::Current&) const;

    virtual ::Ice::LocatorRegistryPrx getRegistry(const ::Ice::Current&) const;

private:
    
    ServerLocatorRegistryPtr _registry;
    ::Ice::LocatorRegistryPrx _registryPrx;
};

#endif
