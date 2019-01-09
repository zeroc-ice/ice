// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_SERVANT_MANAGER_F_H
#define ICE_SERVANT_MANAGER_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class ServantManager;
ICE_API IceUtil::Shared* upCast(ServantManager*);
typedef Handle<ServantManager> ServantManagerPtr;

}

#endif
