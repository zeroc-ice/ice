//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export:ICE_API]]
[[cpp:doxygen:include:Ice/Ice.h]]
[[cpp:header-ext:h]]

[[suppress-warning:reserved-identifier]]
[[js:module:ice]]

[[python:pkgdir:Ice]]

[[java:package:com.zeroc]]
[cs:namespace:ZeroC]
module Ice
{
    /// A request context. <code>Context</code> is used to transmit metadata about a
    /// request from the server to the client, such as Quality-of-Service
    /// (QoS) parameters. Each operation on the client has a <code>Context</code> as
    /// its implicit final parameter.
    dictionary<string, string> Context;
}
