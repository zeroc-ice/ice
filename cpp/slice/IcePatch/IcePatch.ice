// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_ICE_PATCH_ICE
#define ICE_PATCH_ICE_PATCH_ICE

#include <Ice/BuiltinSequences.ice>

module IcePatch
{

exception FileAccessException
{
    string reason;
};

exception BusyException
{
};

class FileDesc
{
};

sequence<FileDesc> FileDescSeq;

interface File
{
    FileDesc describe()
	throws FileAccessException, BusyException;
};

interface Directory extends File
{
    FileDescSeq getContents()
	throws FileAccessException, BusyException;
};

class DirectoryDesc extends FileDesc
{
    Directory* directory;
};

interface Regular extends File
{
    int getBZ2Size()
	throws FileAccessException, BusyException;

    Ice::ByteSeq getBZ2(int pos, int num)
	throws FileAccessException, BusyException;

    Ice::ByteSeq getBZ2MD5(int size)
	throws FileAccessException, BusyException;
};

class RegularDesc extends FileDesc
{
    Regular* regular;
    Ice::ByteSeq md5;
};

};

#endif

