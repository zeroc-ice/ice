// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_ADAPTER_I_H
#define ICE_PACK_ADAPTER_I_H

#include <IceUtil/Mutex.h>
#include <IcePack/Internal.h>
#include <IceUtil/AbstractMutex.h>

namespace IcePack
{

class AdapterFactory;
typedef IceUtil::Handle<AdapterFactory> AdapterFactoryPtr;

class StandaloneAdapterI : public StandaloneAdapter, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:

    StandaloneAdapterI(const AdapterFactoryPtr&);
    StandaloneAdapterI();

    virtual void activate_async(const AMD_Adapter_activatePtr&, const Ice::Current&);
    virtual Ice::ObjectPrx getDirectProxy(const Ice::Current&) const;
    virtual void setDirectProxy(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);

private:

    AdapterFactoryPtr _factory;
};

}

#endif
