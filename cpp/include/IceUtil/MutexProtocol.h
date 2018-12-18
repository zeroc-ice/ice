// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_MUTEX_PROTOCOL_H
#define ICE_UTIL_MUTEX_PROTOCOL_H

#include <IceUtil/Config.h>

namespace IceUtil
{

enum MutexProtocol
{
    PrioInherit,
    PrioNone
};

ICE_API MutexProtocol getDefaultMutexProtocol();

} // End namespace IceUtil

#endif
