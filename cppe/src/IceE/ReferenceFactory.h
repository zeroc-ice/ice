// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REFERENCE_FACTORY_H
#define ICEE_REFERENCE_FACTORY_H

#include <IceE/ReferenceFactoryF.h>
#include <IceE/ConnectionF.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Reference.h> // For ReferenceMode

namespace IceInternal
{

class ReferenceFactory : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    //
    // Make a polymorphic copy of a reference.
    //
    ReferencePtr copy(const Reference* r) const;

    //
    // Create a direct reference.
    //
    ReferencePtr create(const ::Ice::Identity&, const Ice::Context&, const ::std::string&, ReferenceMode, bool,
                        const ::std::vector<EndpointPtr>&
#ifdef ICEE_HAS_ROUTER
			, const RouterInfoPtr&
#endif
			);
    //
    // Create an indirect reference.
    //
#ifdef ICEE_HAS_LOCATOR
    ReferencePtr create(const ::Ice::Identity&, const Ice::Context&, const ::std::string&, ReferenceMode, bool,
			const ::std::string&
#ifdef ICEE_HAS_ROUTER
			, const RouterInfoPtr&
#endif
			, const LocatorInfoPtr&);
#endif
    //
    // Create a fixed reference.
    //
    ReferencePtr create(const ::Ice::Identity&, const Ice::Context&, const ::std::string&, ReferenceMode,
                        const ::std::vector< ::Ice::ConnectionPtr>&);

    //
    // Create a reference from a string.
    //
    ReferencePtr create(const ::std::string&);

    //
    // Create a reference by unmarshaling it from a stream.
    //
    ReferencePtr create(const ::Ice::Identity&, BasicStream*);

    //
    // Create a reference from a property set.
    //
    ReferencePtr createFromProperties(const ::std::string&);

#ifdef ICEE_HAS_ROUTER
    void setDefaultRouter(const ::Ice::RouterPrx&);
    ::Ice::RouterPrx getDefaultRouter() const;
#endif

#ifdef ICEE_HAS_LOCATOR
    void setDefaultLocator(const ::Ice::LocatorPrx&);
    ::Ice::LocatorPrx getDefaultLocator() const;
#endif

private:

    ReferenceFactory(const InstancePtr&, const Ice::CommunicatorPtr&);
    void destroy();
    friend class Instance;

    InstancePtr _instance;
    Ice::CommunicatorPtr _communicator;
#ifdef ICEE_HAS_ROUTER
    ::Ice::RouterPrx _defaultRouter;
#endif
#ifdef ICEE_HAS_LOCATOR
    ::Ice::LocatorPrx _defaultLocator;
#endif
};

}

#endif
