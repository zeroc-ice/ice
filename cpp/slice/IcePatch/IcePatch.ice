// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
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
    Ice::ByteSeq md5;
};

sequence<FileDesc> FileDescSeq;

interface File
{
    nonmutating FileDesc describe()
	throws FileAccessException, BusyException;
};

interface Directory extends File
{
    nonmutating FileDescSeq getContents()
	throws FileAccessException, BusyException;
};

class DirectoryDesc extends FileDesc
{
    Directory* dir;
};

interface Regular extends File
{
    nonmutating int getBZ2Size()
	throws FileAccessException, BusyException;

    nonmutating Ice::ByteSeq getBZ2(int pos, int num)
	throws FileAccessException, BusyException;

    nonmutating Ice::ByteSeq getBZ2MD5(int size)
	throws FileAccessException, BusyException;
};

class RegularDesc extends FileDesc
{
    Regular* reg;
};

};

#endif

