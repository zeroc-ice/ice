// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OPERATION_MODE_H
#define ICEE_OPERATION_MODE_H

#include <IceE/Config.h>

namespace IceEInternal
{

class BasicStream;

}

namespace IceE
{

enum OperationMode
{
    Normal,
    Nonmutating,
    Idempotent
};

ICEE_API void __write(::IceEInternal::BasicStream*, OperationMode);
ICEE_API void __read(::IceEInternal::BasicStream*, OperationMode&);

}

#endif
