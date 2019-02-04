//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GLACIER2_H
#define GLACIER2_H

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/Config.h>
#include <Glacier2/Router.h>
#include <Glacier2/Session.h>
#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/Metrics.h>
#if (!defined(__APPLE__) || TARGET_OS_IPHONE == 0) && !defined(ICE_OS_UWP)
#   include <Glacier2/Application.h>
#endif
#include <Glacier2/SessionHelper.h>
#include <IceUtil/PopDisableWarnings.h>

#endif
