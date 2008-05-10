// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <FilesystemI.h>

using namespace std;

// Slice Node::name() operation

std::string
Filesystem::NodeI::name(const Ice::Current&)
{
    return _name;
}

// NodeI constructor

Filesystem::NodeI::NodeI(const Ice::CommunicatorPtr& communicator, const string& name, const DirectoryIPtr& parent) :
    _name(name), _parent(parent)
{
    // Create an identity. The root directory has the fixed identity "RootDir"
    //
    // COMPILERFIX:
    //
    // The line below causes "thread synchronization error" exception on HP-UX (64-bit only):
    //
    // _id = communicator->stringToIdentity(parent ? IceUtil::generateUUID() : "RootDir");
    //
    // We've rearranged the code to avoid the problem.
    //
    if(parent)
    {
        _id = communicator->stringToIdentity(IceUtil::generateUUID());
    }
    else
    {
        _id = communicator->stringToIdentity("RootDir");
    }
}

// NodeI activate() member function

void
Filesystem::NodeI::activate(const Ice::ObjectAdapterPtr& a)
{
    NodePrx thisNode = NodePrx::uncheckedCast(a->add(this, _id));
    if(_parent)
    {
        _parent->addChild(thisNode);
    }
}

// Slice File::read() operation

Filesystem::Lines
Filesystem::FileI::read(const Ice::Current&)
{
    return _lines;
}

// Slice File::write() operation

void
Filesystem::FileI::write(const Filesystem::Lines& text, const Ice::Current&)
{
    _lines = text;
}

// FileI constructor

Filesystem::FileI::FileI(const Ice::CommunicatorPtr& communicator, const string& name, const DirectoryIPtr& parent) :
    NodeI(communicator, name, parent)
{
}

// Slice Directory::list() operation

Filesystem::NodeSeq
Filesystem::DirectoryI::list(const Ice::Current& c)
{
    return _contents;
}

// DirectoryI constructor

Filesystem::DirectoryI::DirectoryI(const Ice::CommunicatorPtr& communicator, const string& name,
                                   const DirectoryIPtr& parent) :
    NodeI(communicator, name, parent)
{
}

// addChild is called by the child in order to add
// itself to the _contents member of the parent

void
Filesystem::DirectoryI::addChild(const NodePrx& child)
{
    _contents.push_back(child);
}
