// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

