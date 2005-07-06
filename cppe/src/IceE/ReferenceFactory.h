// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REFERENCE_FACTORY_H
#define ICEE_REFERENCE_FACTORY_H

#include <IceE/ReferenceFactoryF.h>
#include <IceE/ConnectionF.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Reference.h> // For Reference::Mode

namespace IceInternal
{

class ReferenceFactory : public ::Ice::Shared, public ::Ice::Mutex
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
			Reference::Mode, const ::std::vector<EndpointPtr>&
#ifndef ICEE_NO_ROUTER
			, const RouterInfoPtr&
#endif
			);
    //
    // Create an indirect reference.
    //
#ifndef ICEE_NO_LOCATOR
    ReferencePtr create(const ::Ice::Identity&, const ::Ice::Context&, const ::std::string&,
			Reference::Mode, const ::std::string&
#ifndef ICEE_NO_ROUTER
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

#ifndef ICEE_NO_ROUTER
    void setDefaultRouter(const ::Ice::RouterPrx&);
    ::Ice::RouterPrx getDefaultRouter() const;
#endif

#ifndef ICEE_NO_LOCATOR
    void setDefaultLocator(const ::Ice::LocatorPrx&);
    ::Ice::LocatorPrx getDefaultLocator() const;
#endif

private:

    ReferenceFactory(const InstancePtr&);
    void destroy();
    friend class Instance;

    InstancePtr _instance;
#ifndef ICEE_NO_ROUTER
    ::Ice::RouterPrx _defaultRouter;
#endif
#ifndef ICEE_NO_LOCATOR
    ::Ice::LocatorPrx _defaultLocator;
#endif
};

}

#endif
