// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOCATOR_INFO_F_H
#define ICE_LOCATOR_INFO_F_H

#include <IceE/Config.h>

#ifdef ICE_HAS_LOCATOR

#include <IceE/Handle.h>

namespace IceInternal
{

class LocatorManager;
void incRef(LocatorManager*);
void decRef(LocatorManager*);
typedef Handle<LocatorManager> LocatorManagerPtr;

class LocatorInfo;
void incRef(LocatorInfo*);
void decRef(LocatorInfo*);
typedef Handle<LocatorInfo> LocatorInfoPtr;

class LocatorTable;
void incRef(LocatorTable*);
void decRef(LocatorTable*);
typedef Handle<LocatorTable> LocatorTablePtr;

}

#endif

#endif
