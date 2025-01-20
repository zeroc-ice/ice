// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["js:module:ice"]]

[["python:pkgdir:Ice"]]

[["java:package:com.zeroc"]]

module Ice
{
    /// Mapping from type IDs to Slice checksums. The dictionary allows verification at run time that client and server
    /// use matching Slice definitions. Ice 3.8 no longer generates Slice checksums but we keep the dictionary definition
    /// for backward compatibility.
    dictionary<string, string> SliceChecksumDict;
}
