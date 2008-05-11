// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OPERATION_MODE_H
#define ICEE_OPERATION_MODE_H

#include <IceE/Config.h>

namespace IceInternal
{

class BasicStream;

}

namespace Ice
{

enum OperationMode
{
    Normal,
    Nonmutating,
    Idempotent
};

ICE_API void __write(::IceInternal::BasicStream*, OperationMode);
ICE_API void __read(::IceInternal::BasicStream*, OperationMode&);

}

#endif
