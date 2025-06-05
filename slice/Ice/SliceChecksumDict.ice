// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Mapping from type IDs to Slice checksums. This dictionary allows verification at runtime that a client and a
    /// server use essentially the same Slice definitions.
    ["deprecated:As of Ice 3.8, the Slice compilers no longer generate Slice checksums."]
    dictionary<string, string> SliceChecksumDict;
}
