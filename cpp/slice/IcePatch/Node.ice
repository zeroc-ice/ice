// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_NODE_ICE
#define ICE_PATCH_NODE_ICE

#include <Ice/BuiltinSequences.ice>

module IcePatch
{

exception NodeAccessException
{
    string reason;
};

class NodeDesc
{
};

sequence<NodeDesc> NodeDescSeq;

interface Node
{
    NodeDesc describe()
	throws NodeAccessException;
};

interface Directory extends Node
{
    NodeDescSeq getContents()
	throws NodeAccessException;
};

class DirectoryDesc extends NodeDesc
{
    Directory* directory;
};

interface File extends Node
{
    int getSizeBZ2()
	throws NodeAccessException;

    Ice::ByteSeq getBytesBZ2(int pos, int num)
	throws NodeAccessException;
};

class FileDesc extends NodeDesc
{
    File* file;
    int size;
    Ice::ByteSeq md5;
};

};

#endif

