// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
