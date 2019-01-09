// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
