// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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

class FileDesc
{
};

sequence<FileDesc> FileDescSeq;

interface File
{
    FileDesc describe()
	throws FileAccessException;
};

interface Directory extends File
{
    FileDescSeq getContents()
	throws FileAccessException;
};

class DirectoryDesc extends FileDesc
{
    Directory* directory;
};

interface Regular extends File
{
    int getBZ2Size()
	throws FileAccessException;

    Ice::ByteSeq getBZ2(int pos, int num)
	throws FileAccessException;
};

class RegularDesc extends FileDesc
{
    Regular* regular;
    int size;
    Ice::ByteSeq md5;
};

};

#endif

