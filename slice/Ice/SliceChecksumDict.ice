// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]

["objc:prefix:ICE"]
module Ice
{

/**
 * A mapping from type IDs to Slice checksums. The dictionary
 * allows verification at run time that client and server
 * use matching Slice definitions.
 **/
dictionary<string, string> SliceChecksumDict;

};

