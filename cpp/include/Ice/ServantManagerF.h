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
