// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_FACTORY_F_H
#define ICE_OBJECT_ADAPTER_FACTORY_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class ObjectAdapterFactory;
#ifdef ICE_CPP11_MAPPING
typedef ::std::shared_ptr<ObjectAdapterFactory> ObjectAdapterFactoryPtr;
#else
IceUtil::Shared* upCast(ObjectAdapterFactory*);
typedef IceInternal::Handle<ObjectAdapterFactory> ObjectAdapterFactoryPtr;
#endif

}

#endif
