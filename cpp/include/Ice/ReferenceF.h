// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REFERENCE_F_H
#define ICE_REFERENCE_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class Reference;
ICE_API IceUtil::Shared* upCast(Reference*);
typedef IceInternal::Handle<Reference> ReferencePtr;

class FixedReference;
ICE_API IceUtil::Shared* upCast(FixedReference*);
typedef IceInternal::Handle<FixedReference> FixedReferencePtr;

class RoutableReference;
ICE_API IceUtil::Shared* upCast(RoutableReference*);
typedef IceInternal::Handle<RoutableReference> RoutableReferencePtr;

}

#endif
