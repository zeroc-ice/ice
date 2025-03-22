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
    /// Mapping from type IDs to Slice checksums.
    /// The dictionary allows verification at runtime that a client and a server use matching Slice definitions. Ice 3.8
    /// no longer generates Slice checksums; this dictionary definition is only provided for backward compatibility with
    /// earlier versions of Ice.
    dictionary<string, string> SliceChecksumDict;
}
