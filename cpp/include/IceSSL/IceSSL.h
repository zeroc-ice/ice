// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_ICE_SSL_H
#define ICE_SSL_ICE_SSL_H

#include <IceUtil/PushDisableWarnings.h>
#include <IceUtil/Config.h>
#if !defined(ICE_OS_WINRT) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
#   include <IceSSL/Plugin.h>
#endif
#include <IceSSL/EndpointInfo.h>
#include <IceSSL/ConnectionInfo.h>
#include <IceUtil/PopDisableWarnings.h>

#endif
