// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_PACK_ADAPTER_I_H
#define ICE_PACK_ADAPTER_I_H

#include <IceUtil/Mutex.h>
#include <IcePack/Internal.h>

namespace IcePack
{

class AdapterFactory;
typedef IceUtil::Handle<AdapterFactory> AdapterFactoryPtr;

class StandaloneAdapterI : public StandaloneAdapter, public IceUtil::Mutex
{
public:

    StandaloneAdapterI(const AdapterFactoryPtr&);
    StandaloneAdapterI();

    virtual void getDirectProxy_async(const AMD_Adapter_getDirectProxyPtr& cb, bool, const Ice::Current&);
    virtual void setDirectProxy(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);

private:

    AdapterFactoryPtr _factory;
};

}

#endif
