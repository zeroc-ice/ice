// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SERVER_LOCATOR_H
#define SERVER_LOCATOR_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

#include <IceE/Locator.h>
#include <IceE/ProxyF.h>

class ServerLocatorRegistry : public Ice::LocatorRegistry
{
public:

    ServerLocatorRegistry();
    
    virtual void setAdapterDirectProxy(const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void setReplicatedAdapterDirectProxy(const std::string&, const ::std::string&, const ::Ice::ObjectPrx&,
						 const ::Ice::Current&);

    //
    // Internal method
    //
    ::Ice::ObjectPrx getAdapter(const ::std::string&) const;
    ::Ice::ObjectPrx getObject(const ::Ice::Identity&) const;
    void addObject(const ::Ice::ObjectPrx&);
    void clear();

private:
    
    ::std::map< ::std::string, ::Ice::ObjectPrx> _adapters;
    ::std::map< ::Ice::Identity, ::Ice::ObjectPrx> _objects;
};

typedef ::IceInternal::Handle< ServerLocatorRegistry> ServerLocatorRegistryPtr;

class ServerLocator : public ::Ice::Locator
{
public:

    ServerLocator(const ::ServerLocatorRegistryPtr&, const ::Ice::LocatorRegistryPrx&);

    virtual ::Ice::ObjectPrx findObjectById(const ::Ice::Identity&, const ::Ice::Current&) const;

    virtual ::Ice::ObjectPrx findAdapterById(const ::std::string&, const ::Ice::Current&) const;

    virtual ::Ice::LocatorRegistryPrx getRegistry(const ::Ice::Current&) const;

private:
    
    ServerLocatorRegistryPtr _registry;
    ::Ice::LocatorRegistryPrx _registryPrx;
};

#endif

#endif
