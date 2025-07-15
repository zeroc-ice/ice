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

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// A simple collection of properties, represented as a dictionary of key/value pairs. Both key and value are
    /// strings.
    /// @see PropertiesAdmin#getPropertiesForPrefix
    ["cpp:type:std::map<std::string, std::string, std::less<>>"]
    dictionary<string, string> PropertyDict;
}
