// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:map"]]
[["cpp:include:string"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// A simple collection of properties, represented as a dictionary of key/value pairs. Both key and value are
    /// strings.
    /// @see PropertiesAdmin#getPropertiesForPrefix
    dictionary<string, string> PropertyDict;
}
