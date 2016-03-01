// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOCATOR_INFO_F_H
#define ICE_LOCATOR_INFO_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

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
