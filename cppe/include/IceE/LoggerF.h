// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOGGER_F_H
#define ICEE_LOGGER_F_H

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace Ice
{

class Logger;

}

namespace IceInternal
{

ICE_API IceUtil::Shared* upCast(::Ice::Logger*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::Logger> LoggerPtr;

}

#endif
