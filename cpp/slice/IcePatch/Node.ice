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
    Ice::ByteSeq getBlock(int n)
	throws NodeAccessException;
};

class FileDesc extends NodeDesc
{
    File* file;
    Ice::ByteSeq md5;
};

};

#endif

