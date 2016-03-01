// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]
[["cpp:include:IcePatch2/Config.h"]]

#include <Ice/BuiltinSequences.ice>

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
};

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
};

/**
 *
 * A sequence with information about many files.
 *
 **/
sequence<LargeFileInfo> LargeFileInfoSeq;

};


