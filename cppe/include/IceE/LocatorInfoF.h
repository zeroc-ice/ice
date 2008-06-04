// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCATOR_INFO_F_H
#define ICEE_LOCATOR_INFO_F_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace IceInternal
{

class LocatorManager;
IceUtil::Shared* upCast(LocatorManager*);
typedef Handle<LocatorManager> LocatorManagerPtr;

class LocatorInfo;
IceUtil::Shared* upCast(LocatorInfo*);
typedef Handle<LocatorInfo> LocatorInfoPtr;

class LocatorTable;
IceUtil::Shared* upCast(LocatorTable*);
typedef Handle<LocatorTable> LocatorTablePtr;

}

#endif

#endif
