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

exception NodeAccessException
{
    string reason;
};

interface Node
{
    string getPath();
};

sequence<Node> Nodes;

interface Directory extends Node
{
    Nodes getNodes()
	throws NodeAccessException;
};

interface File extends Node
{
    Ice::ByteSeq getChunk(int chunkSize, int n)
	throws NodeAccessException;
};

#endif

