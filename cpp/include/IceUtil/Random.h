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
#include <IceUtil/Exception.h>

namespace IceUtilInternal
{

ICE_UTIL_API void generateRandom(char*, int);
ICE_UTIL_API unsigned int random(int = 0);

}
