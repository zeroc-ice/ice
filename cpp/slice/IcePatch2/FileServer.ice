// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_FILE_SERVER_ICE
#define ICE_PATCH2_FILE_SERVER_ICE

#include <IcePatch2/FileInfo.ice>

module IcePatch2
{

/**
 *
 * The number of partitions in the file set. [getCheckSumSeq]
 * returns a sequence with [NumPartitions] elements, and
 * [getFileInfoSeq] expects an argument in the range
 * <literal>0</literal> to <literal>NumPartitions - 1</literal>.
 *
 **/
const int NumPartitions = 256;

/**
 *
 * A sequence of byte sequences. Each element is the checksum for a partition.
 *
 **/
sequence<Ice::ByteSeq> ByteSeqSeq;

/**
 *
 * This exception is raised if the [partition] argument for [getFileInfoSeq]
 * is not in the range <literal>0</literal> to <literal>NumPartitions - 1</literal>.
 *
 **/
exception PartitionOutOfRangeException
{
};

/**
 *
 * This exception is raised if [getFileCompressed] cannot read the contents
 * of a file.
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

interface FileServer
{
    /**
     *
     * Return the [FileInfoSeq] for the specified partition. If the partion 
     * number is out of range, the operation throws [PartitionOutOfRangException].
     *
     * @param partition The partition number (in the range <literal>0</literal> to <NumPartitions> - 1</literal>).
     *
     * @return A sequence containing the [FileInfo] structures for files in the specified partition.
     **/
    ["ami"] nonmutating FileInfoSeq getFileInfoSeq(int partition)
	throws PartitionOutOfRangeException;

    /**
     *
     * Return the checksums for all partitions.
     *
     * @return A sequence containing [NumPartitions] checksums. Partitions
     * with a checksum that differs from the previous checksum for the same partition
     * contain updated files. Partitions with a checksum that is identical to the
     * previous checksum do not contain updated files.
     *
     **/
    nonmutating ByteSeqSeq getChecksumSeq();

    /**
     *
     * Return the master checksum for all partitions. If this checksum is the same
     * as for a previous run, the entire file set is up-to-date.
     *
     * @return The master checksum for the file set.
     *
     **/
    nonmutating Ice::ByteSeq getChecksum();

    /**
     *
     * Read the specified file. If the read operation fails, the operation
     * throws [FileAccessException].
     *
     * @param path The pathname (relative to the data directory) for the file to be read.
     *
     * @param pos The file offset at which to begin reading.
     *
     * @param num The number of bytes to be read.
     *
     * @return A sequence containing the (compressed) file contents. The operation may
     * return fewer bytes than requested, either because end-of-file was reached, or
     * because more than <literal>IcePatch2.MaxReadSize * 1024</literal> bytes were
     * requested.
     *
     **/
    ["ami"] nonmutating Ice::ByteSeq getFileCompressed(string path, int pos, int num)
	throws FileAccessException;
};

/**
 *
 * The &IcePatch2; administrative interface. This must only be
 * accessible from inside the firewall.
 *
 **/
interface Admin
{
    /**
     *
     * Shut down the &IcePatch2; server.
     *
     **/
    idempotent void shutdown();
};

};

#endif
