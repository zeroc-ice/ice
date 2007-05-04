// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ICE_H
#define ICEE_ICE_H

#include <IceE/Initialize.h>
#include <IceE/LocalException.h>
#include <IceE/Properties.h>
#include <IceE/Logger.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Communicator.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/ObjectAdapter.h>
#   include <IceE/Object.h>
#   include <IceE/UUID.h>
#endif
#include <IceE/Proxy.h>
#include <IceE/Connection.h>
#include <IceE/Functional.h>
#ifdef ICEE_HAS_WSTRING
#   include <IceE/Unicode.h>
#endif

#include <IceE/Thread.h>
#include <IceE/Monitor.h>
#include <IceE/Mutex.h>
#include <IceE/SafeStdio.h>

#endif
