//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEPATCH2_API"]]
[["cpp:doxygen:include:IcePatch2/IcePatch2.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:IcePatch2/Config.h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:header-dir:objc"]]

[["python:pkgdir:IcePatch2"]]

#include <Ice/BuiltinSequences.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICEPATCH2"]
module IcePatch2
{

/**
 *
 * Basic information about a single file.
 *
 **/
struct FileInfo
{
    /** The pathname. **/
    string path;

    /** The SHA-1 checksum of the file. **/
    Ice::ByteSeq checksum;

    /** The size of the compressed file in number of bytes. **/
    int size;

    /** The executable flag. */
    bool executable;
}

/**
 *
 * A sequence with information about many files.
 *
 **/
sequence<FileInfo> FileInfoSeq;

/**
 *
 * Basic information about a single file.
 *
 **/
struct LargeFileInfo
{
    /** The pathname. **/
    string path;

    /** The SHA-1 checksum of the file. **/
    Ice::ByteSeq checksum;

    /** The size of the compressed file in number of bytes. **/
    long size;

    /** The executable flag. */
    bool executable;
}

/**
 *
 * A sequence with information about many files.
 *
 **/
sequence<LargeFileInfo> LargeFileInfoSeq;

}
