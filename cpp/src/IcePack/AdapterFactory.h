// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    AdapterFactory(const Ice::ObjectAdapterPtr&, const TraceLevelsPtr&, const std::string&);
    
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

}

#endif
