//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEBOX_API"]]
[["cpp:doxygen:include:IceBox/IceBox.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:IceBox/Config.h"]]

[["ice-prefix"]]

[["js:module:ice"]]
[["js:cjs-module"]]

[["objc:header-dir:objc"]]

[["python:pkgdir:IceBox"]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/CommunicatorF.ice>

[["java:package:com.zeroc"]]

/// IceBox is an application server specifically for Ice applications. IceBox can easily run and administer Ice
/// services that are dynamically loaded as a DLL, shared library, or Java class.
module IceBox
{

/// This exception is a general failure notification. It is thrown for errors such as a service encountering an error
/// during initialization, or the service manager being unable to load a service executable.
["cpp:ice_print"]
local exception FailureException
{
    /// The reason for the failure.
    string reason;
}

/// An application service managed by a {@link ServiceManager}.
local interface Service
{
    /// Start the service. The given communicator is created by the {@link ServiceManager} for use by the service. This
    /// communicator may also be used by other services, depending on the service configuration.
    /// <p class="Note">The {@link ServiceManager} owns this communicator, and is responsible for destroying it.
    /// @param name The service's name, as determined by the configuration.
    /// @param communicator A communicator for use by the service.
    /// @param args The service arguments that were not converted into properties.
    /// @throws FailureException Raised if {@link #start} failed.
    void start(string name, Ice::Communicator communicator, Ice::StringSeq args);

    /// Stop the service.
    void stop();
}

}
