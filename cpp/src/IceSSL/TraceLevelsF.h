// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
