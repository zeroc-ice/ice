// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_SSL_TRACE_LEVELS_F_H
#define ICE_SSL_TRACE_LEVELS_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class TraceLevels;
typedef IceInternal::Handle<TraceLevels> TraceLevelsPtr;

}

namespace IceInternal
{

void incRef(IceSSL::TraceLevels*);
void decRef(IceSSL::TraceLevels*);

}

#endif
