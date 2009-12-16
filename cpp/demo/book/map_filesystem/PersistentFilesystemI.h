// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PERSISTENT_FILESYSTEM_I_H
#define PERSISTENT_FILESYSTEM_I_H

#include <PersistentFilesystem.h>
#include <IdentityNodeMap.h>
#include <IceUtil/IceUtil.h>

namespace Filesystem
{

class DirectoryI;
typedef IceUtil::Handle<DirectoryI> DirectoryIPtr;

class NodeI : virtual public Node
{
protected:

    NodeI(const DirectoryIPtr&);

    const DirectoryIPtr _parent;
    IceUtil::Mutex _nodeMutex;
    bool _destroyed;

public:

    static IdentityNodeMap* _map;
};

typedef IceUtil::Handle<NodeI> NodeIPtr;

class FileI : virtual public File, virtual public NodeI
{
public:

    virtual void destroy(const Ice::Current&);

    virtual std::string name(const Ice::Current&);
    virtual Lines read(const Ice::Current&);
    virtual void write(const Lines&, const Ice::Current&);

    FileI(const PersistentFilePtr&, const DirectoryIPtr&);

private:

    PersistentFilePtr _file;
};
typedef IceUtil::Handle<FileI> FileIPtr;

class DirectoryI : virtual public Directory, virtual public NodeI
{
public:

    virtual void destroy(const Ice::Current&);

    virtual std::string name(const Ice::Current&);
    virtual NodeDescSeq list(const Ice::Current&);
    virtual NodeDesc find(const std::string&, const Ice::Current&);
    virtual DirectoryPrx createDirectory(const std::string&, const Ice::Current&);
    virtual FilePrx createFile(const std::string&, const Ice::Current&);
    virtual void removeEntry(const std::string&);

    DirectoryI(const Ice::Identity& id, const PersistentDirectoryPtr&, const DirectoryIPtr&);

    static Ice::CommunicatorPtr _communicator;
    static Ice::ObjectAdapterPtr _adapter;

private:

    Ice::Identity _id;
    PersistentDirectoryPtr _dir;
};

}

#endif
