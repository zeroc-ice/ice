// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_NODE_I_H
#define ICE_PATCH_NODE_I_H

#include <Ice/Ice.h>
#include <IcePatch/Node.h>

namespace IcePatch
{

class NodeI : virtual public Node
{
public:

    static std::string normalizePath(const std::string&);
};

class DirectoryI : virtual public Directory,
		   virtual public NodeI
{
public:
    
    virtual Nodes getNodes(const Ice::Current&);
};

class FileI : virtual public File,
	      virtual public NodeI
{
public:
    
    virtual Ice::ByteSeq getBytes(Ice::Int, Ice::Int, const Ice::Current&);
};

}

#endif
