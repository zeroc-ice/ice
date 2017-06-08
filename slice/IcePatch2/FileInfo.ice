// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICEPATCH2_API", "objc:header-dir:objc", "python:pkgdir:IcePatch2"]]
[["cpp:include:IcePatch2/Config.h"]]

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
