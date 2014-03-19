// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

#include <Ice/BuiltinSequences.ice>

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

};


