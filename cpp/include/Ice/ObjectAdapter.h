// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_H
#define ICE_OBJECT_ADAPTER_H

#include <Ice/ObjectAdapterF.h>
#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/CollectorF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/LocalException.h>
#include <Ice/Shared.h>
#include <map>

namespace Ice
{

class ICE_API ObjectAdapterI : virtual public ::__Ice::Shared, public JTCMutex
{
public:

    std::string name() const;
    Communicator communicator() const;

    void activate();
    void hold();
    void deactivate();

    void add(const Object&, const std::string&);
    void remove(const std::string&);
    std::string identity(const Object&);
    Object object(const std::string&);

    ObjectProxy proxy(const std::string&);

    ::__Ice::Instance __instance() const;
    Object __findServant(const std::string&) const;

private:

    ObjectAdapterI(const ::__Ice::Instance&, const std::string&,
		   const std::string&);
    virtual ~ObjectAdapterI();
    friend CommunicatorI; // May create ObjectAdapterIs

    ::__Ice::Instance instance_;
    std::string name_;
    std::vector< __Ice::CollectorFactory> collectorFactories_;
    std::map<std::string, Object> objects_;
};

}

#endif
