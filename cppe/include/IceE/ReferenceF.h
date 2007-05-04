// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REFERENCE_F_H
#define ICEE_REFERENCE_F_H

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace IceInternal
{

//
// The Reference mode enumeration is defined here rather than in
// IceInternal::Reference to allow the proxy code to inline the
// proxy mode methods.
//
enum ReferenceMode
{
    ReferenceModeTwoway,
    ReferenceModeOneway,
    ReferenceModeBatchOneway,
    ReferenceModeDatagram,
    ReferenceModeBatchDatagram,
    ReferenceModeLast = ReferenceModeBatchDatagram
};

class Reference;
ICE_API IceUtil::Shared* upCast(Reference*);
typedef IceInternal::Handle<Reference> ReferencePtr;

class FixedReference;
ICE_API IceUtil::Shared* upCast(FixedReference*);
typedef IceInternal::Handle<FixedReference> FixedReferencePtr;

class RoutableReference;
ICE_API IceUtil::Shared* upCast(RoutableReference*);
typedef IceInternal::Handle<RoutableReference> RoutableReferencePtr;

class DirectReference;
ICE_API IceUtil::Shared* upCast(DirectReference*);
typedef IceInternal::Handle<DirectReference> DirectReferencePtr;

class IndirectReference;
ICE_API IceUtil::Shared* upCast(IndirectReference*);
typedef IceInternal::Handle<IndirectReference> IndirectReferencePtr;


}

#endif
