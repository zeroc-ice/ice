// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SERVER_LOCATOR_H
#define SERVER_LOCATOR_H

#include <IceE/Config.h>

#ifndef ICE_NO_LOCATOR

#include <IceE/Locator.h>
#include <IceE/ProxyF.h>

class ServerLocatorRegistry : public IceE::LocatorRegistry
{
public:

    ServerLocatorRegistry();
    
    virtual void setAdapterDirectProxy(const ::std::string&, const ::IceE::ObjectPrx&, const ::IceE::Current&);

    //
    // Internal method
    //
    ::IceE::ObjectPrx getAdapter(const ::std::string&) const;
    ::IceE::ObjectPrx getObject(const ::IceE::Identity&) const;
    void addObject(const ::IceE::ObjectPrx&);

private:
    
    ::std::map< ::std::string, ::IceE::ObjectPrx> _adapters;
    ::std::map< ::IceE::Identity, ::IceE::ObjectPrx> _objects;
};

typedef ::IceEInternal::Handle< ServerLocatorRegistry> ServerLocatorRegistryPtr;

class ServerLocator : public ::IceE::Locator
{
public:

    ServerLocator(const ::ServerLocatorRegistryPtr&, const ::IceE::LocatorRegistryPrx&);

    virtual ::IceE::ObjectPrx findObjectById(const ::IceE::Identity&, const ::IceE::Current&) const;

    virtual ::IceE::ObjectPrx findAdapterById(const ::std::string&, const ::IceE::Current&) const;

    virtual ::IceE::LocatorRegistryPrx getRegistry(const ::IceE::Current&) const;

private:
    
    ServerLocatorRegistryPtr _registry;
    ::IceE::LocatorRegistryPrx _registryPrx;
};

#endif

#endif
