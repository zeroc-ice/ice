// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_LOCATOR_INFO_F_H
#define ICE_LOCATOR_INFO_F_H

#include <Ice/Handle.h>

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

class LocatorAdapterTable;
void incRef(LocatorAdapterTable*);
void decRef(LocatorAdapterTable*);
typedef Handle<LocatorAdapterTable> LocatorAdapterTablePtr;

}

#endif
