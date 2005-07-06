// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCATOR_INFO_F_H
#define ICEE_LOCATOR_INFO_F_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

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
