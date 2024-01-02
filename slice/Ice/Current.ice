//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]
[["js:cjs-module"]]

[["objc:dll-export:ICE_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Ice"]]

#include <Ice/ObjectAdapterF.ice>
#include <Ice/ConnectionF.ice>
#include <Ice/Context.ice>
#include <Ice/Identity.ice>
#include <Ice/OperationMode.ice>
#include <Ice/Version.ice>

[["java:package:com.zeroc"]]

["objc:prefix:ICE"]
module Ice
{

/// Information about the current method invocation for servers. Each operation on the server has a
/// <code>Current</code> as its implicit final parameter. <code>Current</code> is mostly used for Ice services. Most
/// applications ignore this parameter.
local struct Current
{
    /// The object adapter.
    ObjectAdapter adapter;

    /// Information about the connection over which the current method invocation was received. If the invocation is
    /// direct due to collocation optimization, this value is set to null.
    Connection con;

    /// The Ice object identity.
    Identity id;

    /// The facet.
    string facet;

    /// The operation name.
    string operation;

    /// The mode of the operation.
    OperationMode mode;

    /// The request context, as received from the client.
    Context ctx;

    /// The request id unless oneway (0).
    int requestId;

    /// The encoding version used to encode the input and output parameters.
    EncodingVersion encoding;
}

}
