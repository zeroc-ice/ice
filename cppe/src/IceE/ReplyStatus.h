// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REPLY_STATUS_H
#define ICEE_REPLY_STATUS_H

#include <IceE/Config.h>

namespace IceInternal
{

static const Ice::Byte replyOK = 0;
static const Ice::Byte replyUserException = 1;
static const Ice::Byte replyObjectNotExist = 2;
static const Ice::Byte replyFacetNotExist = 3;
static const Ice::Byte replyOperationNotExist = 4;
static const Ice::Byte replyUnknownLocalException = 5;
static const Ice::Byte replyUnknownUserException = 6;
static const Ice::Byte replyUnknownException = 7;

}

#endif
