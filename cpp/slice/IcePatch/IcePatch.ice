// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH_ICE_PATCH_ICE
#define ICE_PATCH_ICE_PATCH_ICE

#include <Ice/BuiltinSequences.ice>

/**
 *
 * A patching service for software distributions. &IcePatch; automates
 * updating of individual files as well as complete directory
 * hierarchies. Only files that have changed are downloaded to the
 * client machine, using efficient compression algorithms.
 *
 **/
module IcePatch
{

/**
 *
 * This exception is raised if an error occurs while attempting to
 * access a file or directory.
 *
 **/
exception FileAccessException
{
    /**
     *
     * A description of the error.
     *
     **/
    string reason;
};

/**
 *
 * This exception is raised when the server is busy performing
 * housekeeping functions. The operation should be tried again
 * later.
 *
 **/
exception BusyException
{
};

/**
 *
 * Descriptive information common to files and directories.
 *
 **/
class FileDesc
{
    /**
     *
     * A sequence of 16 bytes containing the message digest of a
     * file or directory computed using the MD5 algorithm. The
     * message digest for a directory is computed using the names
     * and message digests of each of the files and directories
     * it contains.
     *
     **/
    Ice::ByteSeq md5;
};

/** A sequence of file descriptions. **/
sequence<FileDesc> FileDescSeq;

/**
 *
 * The base interface for files and directories.
 *
 **/
interface File
{
    /**
     *
     * Obtain a description of the file or directory.
     *
     * @return The description, which can be downcast to [RegularDesc]
     * or [DirectoryDesc].
     *
     * @throws FileAccessException Raised if the file or directory cannot
     * be accessed.
     *
     * @throws BusyException Raised if the server is busy.
     *
     **/
    nonmutating FileDesc describe()
	throws FileAccessException, BusyException;
};

/**
 *
 * The interface for a directory.
 *
 **/
interface Directory extends File
{
    /**
     *
     * Obtain descriptions for the immediate children of the directory.
     * This operation does not return the contents of subdirectories.
     *
     * @return The descriptions, each of which can be downcast to [RegularDesc]
     * or [DirectoryDesc].
     *
     * @throws FileAccessException Raised if a file or directory cannot
     * be accessed.
     *
     * @throws BusyException Raised if the server is busy.
     *
     **/
    nonmutating FileDescSeq getContents()
	throws FileAccessException, BusyException;

    /**
     *
     * Obtain the total number of bytes to be downloaded for a particular
     * configuration.
     *
     * @param md5 The message digest of the client's existing configuration.
     * If the given sequence is empty, then the return value represents the
     * size of the entire directory, including all subdirectories. If the
     * sequence is not empty, and the message digest corresponds to a
     * configuration that is known by the server, then the return value
     * represents the number of bytes to be downloaded in order to update
     * the client's directory to match the server's.
     *
     * @return The sum in bytes of the sizes of all BZ2 files to be
     * downloaded for the configuration. If the given message digest does
     * not match any known configurations, [-1] is returned.
     *
     * @throws FileAccessException Raised if a file or directory cannot
     * be accessed.
     *
     * @throws BusyException Raised if the server is busy.
     *
     **/
    nonmutating long getTotal(Ice::ByteSeq md5)
	throws FileAccessException, BusyException;
};

/**
 *
 * Descriptive information for a directory.
 *
 **/
class DirectoryDesc extends FileDesc
{
    /**
     *
     * A proxy for the directory object.
     *
     **/
    Directory* dir;
};

/**
 *
 * The interface for a regular file.
 *
 **/
interface Regular extends File
{
    /**
     *
     * Obtain the compressed size of the file. &IcePatch; compresses files
     * using the BZIP2 algorithm.
     *
     * @return The number of bytes in the compressed representation of the
     * file.
     *
     * @throws FileAccessException Raised if the file cannot be accessed.
     *
     * @throws BusyException Raised if the server is busy.
     *
     **/
    nonmutating int getBZ2Size()
	throws FileAccessException, BusyException;

    /**
     *
     * Obtains a portion of the compressed file.
     *
     * @param pos The starting position in bytes. The position must not
     * exceed the size of the file.
     *
     * @param num The maximum number of bytes to return. This number must
     * not exceed one megabyte.
     *
     * @return The requested bytes. The returned sequence may contain fewer
     * bytes than requested if [num] exceeds the remaining bytes in the
     * file.
     *
     * @throws FileAccessException Raised if the file cannot be accessed.
     *
     * @throws BusyException Raised if the server is busy.
     *
     **/
    nonmutating Ice::ByteSeq getBZ2(int pos, int num)
	throws FileAccessException, BusyException;

    /**
     *
     * Obtain the message digest for a portion of the compressed file,
     * allowing a client to determine whether a partially downloaded
     * file is still valid.
     *
     * @param size The number of bytes to use when computing the message
     * digest.
     *
     * @return The number of bytes in the compressed representation of the
     * file.
     *
     * @throws FileAccessException Raised if the file cannot be accessed.
     *
     * @throws BusyException Raised if the server is busy.
     *
     **/
    nonmutating Ice::ByteSeq getBZ2MD5(int size)
	throws FileAccessException, BusyException;
};

/**
 *
 * Descriptive information for a regular file.
 *
 **/
class RegularDesc extends FileDesc
{
    /**
     *
     * A proxy for the file object.
     *
     **/
    Regular* reg;
};

};

#endif

