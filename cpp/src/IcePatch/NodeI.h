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

    NodeI(const Ice::ObjectAdapterPtr&);

protected:

    Ice::ObjectAdapterPtr _adapter;
};

class DirectoryI : virtual public Directory,
		   virtual public NodeI
{
public:
    
    DirectoryI(const Ice::ObjectAdapterPtr&);

    virtual NodeDescPtr describe(const Ice::Current&);
    virtual NodeDescSeq getContents(const Ice::Current&);
};

class FileI : virtual public File,
	      virtual public NodeI
{
public:
    
    FileI(const Ice::ObjectAdapterPtr&);

    virtual NodeDescPtr describe(const Ice::Current&);
    virtual Ice::Int getSizeBZ2(const Ice::Current&);
    virtual Ice::ByteSeq getBytesBZ2(Ice::Int, Ice::Int, const Ice::Current&);
};

}

#endif
