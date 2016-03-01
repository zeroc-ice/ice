// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ICE_H
#define ICE_ICE_H

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/Config.h>
#include <Ice/DeprecatedStringConverter.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/PropertiesAdmin.h>
#include <Ice/Properties.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>
#include <Ice/RemoteLogger.h>
#include <Ice/Communicator.h>
#include <Ice/CommunicatorAsync.h>
#include <Ice/ObjectFactory.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/ServantLocator.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/SlicedData.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/IncomingAsync.h>
#include <Ice/Process.h>
#if !defined(ICE_OS_WINRT) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
#   include <Ice/Application.h>
#endif
#include <Ice/Connection.h>
#include <Ice/ConnectionAsync.h>
#include <Ice/Functional.h>
#include <Ice/Stream.h>
#include <Ice/ImplicitContext.h>
#include <Ice/Locator.h>
#include <Ice/Router.h>
#include <Ice/DispatchInterceptor.h>
#include <Ice/Plugin.h>
#include <Ice/NativePropertiesAdmin.h>
#include <Ice/Instrumentation.h>
#include <Ice/Metrics.h>
#include <Ice/SliceChecksums.h>
#if !defined(ICE_OS_WINRT) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
#   include <Ice/Service.h>
#endif
#include <Ice/RegisterPlugins.h>
#include <IceUtil/PopDisableWarnings.h>

#endif
