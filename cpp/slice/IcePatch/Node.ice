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

interface Node
{
};

sequence<Node*> Nodes;

interface Directory extends Node
{
    Nodes getNodes()
	throws NodeAccessException;
};

interface File extends Node
{
    Ice::ByteSeq getBytes(int startPos, int howMuch)
	throws NodeAccessException;
};

};

#endif

