// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <FilesystemI.h>

using namespace std;

Ice::ObjectAdapterPtr Filesystem::NodeI::_adapter;      // static member

// Slice Node::name() operation

std::string
Filesystem::NodeI::name(const Ice::Current &)
{
    return _name;
}

// NodeI constructor

Filesystem::NodeI::NodeI(const Ice::CommunicatorPtr & ic, const string & name, const DirectoryIPtr & parent) :
    _name(name), _parent(parent)
{
    // Create an identity. The parent has the fixed identity "RootDir"
    //
    // COMPILERFIX:
    //
    // The line below causes a "thread synchronization error" exception on HP-UX (64 bit only):
    //
    // Ice::Identity myID = Ice::stringToIdentity(parent ? IceUtil::generateUUID() : "RootDir");
    //
    // We've rearranged the code to avoid the problem.
    //
    Ice::Identity myID;
    if (parent)
        myID = ic->stringToIdentity(IceUtil::generateUUID());
    else
        myID = ic->stringToIdentity("RootDir");

    // Create a proxy for the new node and add it as a child to the parent
    //
    NodePrx thisNode = NodePrx::uncheckedCast(_adapter->createProxy(myID));
    if (parent)
        parent->addChild(thisNode);

    // Activate the servant
    //
    _adapter->add(this, myID);
}

// Slice File::read() operation

Filesystem::Lines
Filesystem::FileI::read(const Ice::Current &)
{
    return _lines;
}

// Slice File::write() operation

void
Filesystem::FileI::write(const Filesystem::Lines & text,
                         const Ice::Current &)
{
    _lines = text;
}

// FileI constructor

Filesystem::FileI::FileI(const Ice::CommunicatorPtr & ic, const string & name, const DirectoryIPtr & parent) : 
    NodeI(ic, name, parent)
{
}

// Slice Directory::list() operation

Filesystem::NodeSeq
Filesystem::DirectoryI::list(const Ice::Current &)
{
    return _contents;
}

// DirectoryI constructor

Filesystem::DirectoryI::DirectoryI(const Ice::CommunicatorPtr & ic, const string & name, const DirectoryIPtr & parent) :
    NodeI(ic, name, parent)
{
}

// addChild is called by the child in order to add
// itself to the _contents member of the parent

void
Filesystem::DirectoryI::addChild(const NodePrx child)
{
    _contents.push_back(child);
}
