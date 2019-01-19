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

#include <IcePatch2/FileInfo.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

/**
 *
 * IcePatch can be used to update file hierarchies in a simple and
 * efficient manner. Checksums ensure file integrity, and data is
 * compressed before downloading.
 *
 **/
["objc:prefix:ICEPATCH2"]
module IcePatch2
{

/**
 *
 * A sequence of byte sequences. Each element is the checksum for a
 * partition.
 *
 **/
sequence<Ice::ByteSeq> ByteSeqSeq;

/**
 *
 * A <code>partition</code> argument was not in the range 0-255.
 *
 **/
exception PartitionOutOfRangeException
{
}

/**
 *
 * This exception is raised if a file's contents cannot be read.
 *
 **/
exception FileAccessException
{
    /**
     *
     * An explanation of the reason for the failure.
     *
     **/
    string reason;
}

/**
 *
 * This exception is raised if an operation tries to use a file whose size is
 * larger than 2.1 GB. Use the "large" versions of the operations instead.
 *
 **/
exception FileSizeRangeException extends FileAccessException
{
}

/**
 *
 * The interface that provides access to files.
 *
 **/
interface FileServer
{
    /**
     *
     * Return file information for the specified partition.
     *
     * <p class="Deprecated"> This operation is deprecated and only present for
     * compatibility with old Ice clients (older than version 3.6).
     *
     * @param partition The partition number in the range 0-255.
     *
     * @return A sequence containing information about the files in the
     * specified partition.
     *
     * @throws PartitionOutOfRangeException If the partition number is out of range.
     * @throws FileSizeRangeException If a file is larger than 2.1GB.
     **/
    ["deprecate:getFileInfoSeq() is deprecated, use getLargeFileInfoSeq() instead.",
     "nonmutating", "cpp:const"] idempotent FileInfoSeq getFileInfoSeq(int partition)
        throws PartitionOutOfRangeException, FileSizeRangeException;

    /**
     *
     * Returns file information for the specified partition.
     *
     * @param partition The partition number in the range 0-255.
     *
     * @return A sequence containing information about the files in the
     * specified partition.
     *
     * @throws PartitionOutOfRangeException If the partition number is out of range.
     **/
    ["nonmutating", "cpp:const"] idempotent LargeFileInfoSeq getLargeFileInfoSeq(int partition)
        throws PartitionOutOfRangeException;

    /**
     *
     * Return the checksums for all partitions.
     *
     * @return A sequence containing 256 checksums. Partitions with a
     * checksum that differs from the previous checksum for the same
     * partition contain updated files. Partitions with a checksum
     * that is identical to the previous checksum do not contain
     * updated files.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent ByteSeqSeq getChecksumSeq();

    /**
     *
     * Return the master checksum for all partitions. If this checksum is the same
     * as for a previous run, the entire file set is up-to-date.
     *
     * @return The master checksum for the file set.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::ByteSeq getChecksum();

    /**
     *
     * Read the specified file. This operation may only return fewer bytes than requested
     * in case there was an end-of-file condition.
     *
     * <p class="Deprecated"> This operation is deprecated and only present for
     * compatibility with old Ice clients (older than version 3.6).
     *
     * @param path The pathname (relative to the data directory) for
     * the file to be read.
     *
     * @param pos The file offset at which to begin reading.
     *
     * @param num The number of bytes to be read.
     *
     * @return A sequence containing the compressed file contents.
     *
     * @throws FileAccessException If an error occurred while trying to read the file.
     * @throws FileSizeRangeException If a file is larger than 2.1GB.
     *
     **/
    ["deprecate:getFileCompressed() is deprecated, use getLargeFileCompressed() instead.",
     "amd", "nonmutating", "cpp:const", "cpp:array"]
    idempotent Ice::ByteSeq getFileCompressed(string path, int pos, int num)
        throws FileAccessException, FileSizeRangeException;

    /**
     *
     * Read the specified file. This operation may only return fewer bytes than requested
     * in case there was an end-of-file condition.
     *
     * @param path The pathname (relative to the data directory) for
     * the file to be read.
     *
     * @param pos The file offset at which to begin reading.
     *
     * @param num The number of bytes to be read.
     *
     * @return A sequence containing the compressed file contents.
     *
     * @throws FileAccessException If an error occurred while trying to read the file.
     *
     **/
    ["amd", "nonmutating", "cpp:const", "cpp:array"]
    idempotent Ice::ByteSeq getLargeFileCompressed(string path, long pos, int num)
        throws FileAccessException;
}

}
