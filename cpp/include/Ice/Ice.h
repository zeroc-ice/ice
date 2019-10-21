//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ICE_H
#define ICE_ICE_H

#include <IceUtil/PushDisableWarnings.h>
#include <IceUtil/CtrlCHandler.h>
#include <Ice/Config.h>
#include <Ice/Comparable.h>
#include <Ice/Initialize.h>
#include <Ice/Optional.h>
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
#include <Ice/Connection.h>
#include <Ice/ConnectionAsync.h>
#include <Ice/Functional.h>
#include <Ice/ImplicitContext.h>
#include <Ice/Locator.h>
#include <Ice/Router.h>
#include <Ice/DispatchInterceptor.h>
#include <Ice/Plugin.h>
#include <Ice/NativePropertiesAdmin.h>
#include <Ice/Instrumentation.h>
#include <Ice/Metrics.h>
#include <Ice/SliceChecksums.h>
#if !defined(ICE_OS_UWP) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
#   include <Ice/Service.h>
#endif
#include <Ice/RegisterPlugins.h>
#include <Ice/InterfaceByValue.h>
#include <Ice/StringConverter.h>
#include <Ice/IconvStringConverter.h>
#include <Ice/UUID.h>
#include <IceUtil/PopDisableWarnings.h>

#endif
