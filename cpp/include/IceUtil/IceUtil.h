//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_ICE_UTIL_H
#define ICE_UTIL_ICE_UTIL_H

#include <IceUtil/PushDisableWarnings.h>

//
// This file must include *all* headers of IceUtil, except
// for DisableWarnings.h and headers with only IceUtilInternal symbols
//

#include <IceUtil/Config.h>
#include <IceUtil/ConsoleUtil.h>
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
#   include <IceUtil/CtrlCHandler.h>
#endif
#include <IceUtil/Exception.h>
#include <IceUtil/StringConverter.h>
#include <IceUtil/Timer.h>
#include <IceUtil/UUID.h>

#include <IceUtil/PopDisableWarnings.h>
#endif
