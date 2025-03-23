// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["java:package:com.zeroc"]]
[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

module Ice
{
    /// Mapping from type IDs to Slice checksums. This dictionary allows verification at runtime that a client and a
    /// server use essentially the same Slice definitions.
    /// @remark As of Ice 3.8, the Slice compilers no longer generate Slice checksums. This dictionary definition is
    /// provided for backward compatibility with earlier versions of Ice.
    dictionary<string, string> SliceChecksumDict;
}
