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
