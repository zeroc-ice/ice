// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    ::Ice::ObjectPrx getAdapter(const ::std::string&);

private:
    
    ::std::map< ::std::string, ::Ice::ObjectPrx> _adapters;
};

typedef ::IceInternal::Handle< ServerLocatorRegistry> ServerLocatorRegistryPtr;

class ServerLocator : public ::Ice::Locator
{
public:

    ServerLocator(const ::ServerLocatorRegistryPtr&, const ::Ice::LocatorRegistryPrx&);

    virtual ::Ice::ObjectPrx findAdapterByName(const std::string&, const ::Ice::Current&) const;

    virtual ::Ice::LocatorRegistryPrx getRegistry(const ::Ice::Current&) const;

private:
    
    ServerLocatorRegistryPtr _registry;
    ::Ice::LocatorRegistryPrx _registryPrx;
};

#endif
