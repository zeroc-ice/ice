// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:map"]]
[["cpp:include:string"]]

[["js:module:ice"]]

[["python:pkgdir:Ice"]]

[["java:package:com.zeroc"]]

module Ice
{
    /// A string-string dictionary, used to transmit additional information with an Ice request. This context is only
    /// transmitted with requests, from clients to servers; the Ice protocol does not provide response contexts.
    ["cpp:type:std::map<std::string, std::string, std::less<>>"]
    dictionary<string, string> Context;
}
