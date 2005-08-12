// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REFERENCE_FACTORY_H
#define ICE_REFERENCE_FACTORY_H

#include <IceE/ReferenceFactoryF.h>
#include <IceE/ConnectionF.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Reference.h> // For Reference::Mode

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
    ReferencePtr create(const ::Ice::Identity&, const ::Ice::Context&, const ::std::string&,
			Reference::Mode, bool, const ::std::vector<EndpointPtr>&
#ifdef ICE_HAS_ROUTER
			, const RouterInfoPtr&
#endif
			);
    //
    // Create an indirect reference.
    //
#ifdef ICE_HAS_LOCATOR
    ReferencePtr create(const ::Ice::Identity&, const ::Ice::Context&, const ::std::string&,
			Reference::Mode, bool, const ::std::string&
#ifdef ICE_HAS_ROUTER
			, const RouterInfoPtr&
#endif
			, const LocatorInfoPtr&);
#endif
    //
    // Create a fixed reference.
    //
    ReferencePtr create(const ::Ice::Identity&, const ::Ice::Context&, const ::std::string&,
	                Reference::Mode, const ::std::vector< ::Ice::ConnectionPtr>&);

    //
    // Create a reference from a string.
    //
    ReferencePtr create(const ::std::string&);

    //
    // Create a reference by unmarshaling it from a stream.
    //
    ReferencePtr create(const ::Ice::Identity&, BasicStream*);

#ifdef ICE_HAS_ROUTER
    void setDefaultRouter(const ::Ice::RouterPrx&);
    ::Ice::RouterPrx getDefaultRouter() const;
#endif

#ifdef ICE_HAS_LOCATOR
    void setDefaultLocator(const ::Ice::LocatorPrx&);
    ::Ice::LocatorPrx getDefaultLocator() const;
#endif

private:

    ReferenceFactory(const InstancePtr&);
    void destroy();
    friend class Instance;

    InstancePtr _instance;
#ifdef ICE_HAS_ROUTER
    ::Ice::RouterPrx _defaultRouter;
#endif
#ifdef ICE_HAS_LOCATOR
    ::Ice::LocatorPrx _defaultLocator;
#endif
};

}

#endif
