// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_SERVANT_MANAGER_F_H
#define ICE_SERVANT_MANAGER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ServantManager;
void incRef(ServantManager*);
void decRef(ServantManager*);
typedef Handle<ServantManager> ServantManagerPtr;

}

#endif
