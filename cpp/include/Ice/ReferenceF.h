// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REFERENCE_F_H
#define ICE_REFERENCE_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Reference;
ICE_API void incRef(Reference*);
ICE_API void decRef(Reference*);
typedef IceInternal::Handle<Reference> ReferencePtr;

class FixedReference;
ICE_API void incRef(FixedReference*);
ICE_API void decRef(FixedReference*);
typedef IceInternal::Handle<FixedReference> FixedReferencePtr;

class RoutableReference;
ICE_API void incRef(RoutableReference*);
ICE_API void decRef(RoutableReference*);
typedef IceInternal::Handle<RoutableReference> RoutableReferencePtr;

class DirectReference;
ICE_API void incRef(DirectReference*);
ICE_API void decRef(DirectReference*);
typedef IceInternal::Handle<DirectReference> DirectReferencePtr;

class IndirectReference;
ICE_API void incRef(IndirectReference*);
ICE_API void decRef(IndirectReference*);
typedef IceInternal::Handle<IndirectReference> IndirectReferencePtr;


}

#endif
