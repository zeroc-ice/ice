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

#ifndef ICE_REFERENCE_FACTORY_H
#define ICE_REFERENCE_FACTORY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ReferenceFactoryF.h>
#include <Ice/Reference.h> // For Reference::Mode
#include <set>

namespace IceInternal
{

class ReferenceFactory : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    ReferencePtr create(const Ice::Identity&, const std::vector<std::string>&, Reference::Mode, bool,
			const std::string&, const std::vector<EndpointPtr>&,
			const RouterInfoPtr&, const LocatorInfoPtr&, const Ice::ObjectAdapterPtr&, bool);
    ReferencePtr create(const std::string&);
    ReferencePtr create(const Ice::Identity&, BasicStream*);

    void setDefaultRouter(const ::Ice::RouterPrx&);
    Ice::RouterPrx getDefaultRouter() const;

    void setDefaultLocator(const ::Ice::LocatorPrx&);
    Ice::LocatorPrx getDefaultLocator() const;

private:

    ReferenceFactory(const InstancePtr&);
    void destroy();
    friend class Instance;

    InstancePtr _instance;
    Ice::RouterPrx _defaultRouter;
    Ice::LocatorPrx _defaultLocator;
    std::set<ReferencePtr> _references;
    std::set<ReferencePtr>::iterator _referencesHint;
    int _evict;
};

}

#endif
