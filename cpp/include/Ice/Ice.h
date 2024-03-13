//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ICE_H
#define ICE_ICE_H

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/Config.h>
#include <Ice/Comparable.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Proxy.h>
#include <Ice/Current.h>
#include <Ice/LocalException.h>
#include <optional>
#include <Ice/Object.h>
#include <Ice/SlicedData.h>
#include <Ice/MarshaledResult.h>
#include <Ice/FactoryTable.h>
#include <Ice/FactoryTableInit.h>
#include <Ice/ExceptionHelpers.h> // temporary

#ifndef ICE_BUILDING_GENERATED_CODE

// We don't need to see the following headers when building the generated code.

#    include <Ice/Initialize.h>
#    include <Ice/ProxyFunctions.h>
#    include <Ice/Properties.h>
#    include <Ice/Logger.h>
#    include <Ice/LoggerUtil.h>
#    include <Ice/Communicator.h>
#    include <Ice/ObjectAdapter.h>
#    include <Ice/ServantLocator.h>
#    if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
#        include <Ice/CtrlCHandler.h>
#        include <Ice/Service.h>
#    endif
#    include <Ice/Connection.h>
#    include <Ice/ImplicitContext.h>
#    include <Ice/Plugin.h>
#    include <Ice/NativePropertiesAdmin.h>
#    include <Ice/Instrumentation.h>
#    include <Ice/RegisterPlugins.h>
#    include <Ice/StringConverter.h>
#    include <Ice/IconvStringConverter.h>
#    include <Ice/UUID.h>

// Generated header files:
#    include <Ice/EndpointTypes.h>
#    include <Ice/Locator.h>
#    include <Ice/Metrics.h>
#    include <Ice/Process.h>
#    include <Ice/PropertiesAdmin.h>
#    include <Ice/RemoteLogger.h>
#    include <Ice/Router.h>

#endif
#include <IceUtil/PopDisableWarnings.h>

#endif
