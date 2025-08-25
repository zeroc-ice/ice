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
    /// Represents additional information carried by an Ice request. This information is filled in by the application
    /// and transmitted by Ice as-is, from the client to the server.
    /// @remark The Ice protocol provides request contexts but does not provide response contexts.
    ["cpp:type:std::map<std::string, std::string, std::less<>>"]
    dictionary<string, string> Context;
}
