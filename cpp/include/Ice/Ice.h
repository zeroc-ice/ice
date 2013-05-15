// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ICE_H
#define ICE_ICE_H

#include <IceUtil/Config.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/PropertiesAdmin.h>
#include <Ice/Properties.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Stats.h>
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
#ifndef ICE_OS_WINRT
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
#ifndef ICE_OS_WINRT
#   include <Ice/Service.h>
#endif
#ifndef _WIN32
#   include <Ice/IconvStringConverter.h>
#endif

#endif
