// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REFERENCE_F_H
#define ICEE_REFERENCE_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class Reference;
ICEE_API void incRef(Reference*);
ICEE_API void decRef(Reference*);
typedef IceInternal::Handle<Reference> ReferencePtr;

class FixedReference;
ICEE_API void incRef(FixedReference*);
ICEE_API void decRef(FixedReference*);
typedef IceInternal::Handle<FixedReference> FixedReferencePtr;

class RoutableReference;
ICEE_API void incRef(RoutableReference*);
ICEE_API void decRef(RoutableReference*);
typedef IceInternal::Handle<RoutableReference> RoutableReferencePtr;

class DirectReference;
ICEE_API void incRef(DirectReference*);
ICEE_API void decRef(DirectReference*);
typedef IceInternal::Handle<DirectReference> DirectReferencePtr;

class IndirectReference;
ICEE_API void incRef(IndirectReference*);
ICEE_API void decRef(IndirectReference*);
typedef IceInternal::Handle<IndirectReference> IndirectReferencePtr;


}

#endif
