// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ADAPTER_FACTORY_H
#define ICE_PACK_ADAPTER_FACTORY_H

#include <IcePack/Internal.h>
#include <Freeze/EvictorF.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class AdapterFactory : public Ice::ObjectFactory
{
public:

    AdapterFactory(const Ice::ObjectAdapterPtr&, const TraceLevelsPtr&, const Freeze::DBEnvironmentPtr&);
    
    //
    // Ice::ObjectFactory method implementation.
    //
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();
    
    AdapterPrx createStandaloneAdapter(const std::string& name);

private:

    friend class StandaloneAdapterI;

    void destroy(const Ice::Identity&);
    
    Ice::ObjectAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;

    Freeze::EvictorPtr _evictor;
};

typedef ::IceUtil::Handle< AdapterFactory> AdapterFactoryPtr;

};

#endif
