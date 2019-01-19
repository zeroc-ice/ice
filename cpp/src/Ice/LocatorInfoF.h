//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
