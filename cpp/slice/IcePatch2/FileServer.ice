// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_FILE_SERVER_ICE
#define ICE_PATCH2_FILE_SERVER_ICE

#include <IcePatch2/FileInfo.ice>

/**
 *
 * IcePatch can be used to update file hiearchies in a simple and
 * efficient manner. Checksums ensure file integrity, and data is
 * compressed before download.
 *
 **/
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
 * The [partition] argument for
 * [FileServer::getFileInfoSeq] was not in the range 0-255.
 *
 **/
exception PartitionOutOfRangeException
{
};

/**
 *
 * This exception is raised if [FileServer::getFileCompressed] cannot read the
 * contents of a file.
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
};

/**
 *
 * The interface that provides access to files.
 *
 **/
interface FileServer
{
    /**
     *
     * Return the [FileInfoSeq] for the specified partition. If the
     * partion number is out of range, the operation throws
     * [PartitionOutOfRangException].
     *
     * @param partition The partition number in the range 0-255.
     *
     * @return A sequence containing the [FileInfo] structures for
     * files in the specified partition.
     *
     **/
    ["ami", "nonmutating", "cpp:const"] idempotent FileInfoSeq getFileInfoSeq(int partition)
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
     * Read the specified file. If the read operation fails, the
     * operation throws [FileAccessException]. This operation may only
     * return fewer bytes than requested in case there was an
     * end-of-file condition.
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
     **/
    ["ami", "amd", "nonmutating", "cpp:const", "cpp:array"] 
    idempotent Ice::ByteSeq getFileCompressed(string path, int pos, int num)
        throws FileAccessException;
};

/**
 *
 * The IcePatch administrative interface. This must only be
 * accessible from inside the firewall.
 *
 **/
interface Admin
{
    /**
     *
     * Shut down the IcePatch server.
     *
     **/
    void shutdown();
};

};

#endif
