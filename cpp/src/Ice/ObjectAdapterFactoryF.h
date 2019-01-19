//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_ADAPTER_FACTORY_F_H
#define ICE_OBJECT_ADAPTER_FACTORY_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class ObjectAdapterFactory;
#ifdef ICE_CPP11_MAPPING
using ObjectAdapterFactoryPtr = ::std::shared_ptr<ObjectAdapterFactory>;
#else
IceUtil::Shared* upCast(ObjectAdapterFactory*);
typedef IceInternal::Handle<ObjectAdapterFactory> ObjectAdapterFactoryPtr;
#endif

}

#endif
