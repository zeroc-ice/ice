// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    ReferencePtr create(const Ice::Identity&, const std::string&, Reference::Mode, bool, bool,
			const std::vector<EndpointPtr>&, const std::vector<EndpointPtr>&,
			const RouterInfoPtr&, const Ice::ObjectAdapterPtr&);
    ReferencePtr create(const std::string&);
    ReferencePtr create(const Ice::Identity&, BasicStream*);

    void setDefaultRouter(const ::Ice::RouterPrx&);
    Ice::RouterPrx getDefaultRouter() const;

private:

    ReferenceFactory(const InstancePtr&);
    void destroy();
    friend class Instance;

    InstancePtr _instance;
    Ice::RouterPrx _defaultRouter;
    std::set<ReferencePtr> _references;
    std::set<ReferencePtr>::iterator _referencesHint;
    int _evict;
};

}

#endif
