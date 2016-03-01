// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_RANDOM_H
#define ICE_UTIL_RANDOM_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

namespace IceUtilInternal
{

ICE_UTIL_API void generateRandom(char*, size_t);
ICE_UTIL_API unsigned int random(int = 0);

}

#endif
