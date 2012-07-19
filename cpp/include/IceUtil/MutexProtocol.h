// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <IceUtil/Config.h>

namespace IceUtil
{

enum MutexProtocol
{
    PrioInherit,
    PrioNone
};

ICE_UTIL_API MutexProtocol getDefaultMutexProtocol();

} // End namespace IceUtil
