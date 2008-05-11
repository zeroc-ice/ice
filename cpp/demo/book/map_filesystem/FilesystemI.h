// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef __FilesystemI_h__
#define __FilesystemI_h__

#include <IceUtil/Mutex.h>
#include <Ice/Ice.h>
#include <Filesystem.h>
#include <IdentityNodeMap.h>

namespace FilesystemI
{

class DirectoryI;
typedef IceUtil::Handle<DirectoryI> DirectoryIPtr;

class NodeI : virtual public Filesystem::Node
{
public:

    virtual ::std::string name(const Ice::Current&);
    Ice::Identity id() const;
    Ice::ObjectPrx activate(const Ice::ObjectAdapterPtr&);

    static ::std::string _envName;
    static ::std::string _dbName;

protected:

    NodeI(const Ice::CommunicatorPtr&, const Ice::Identity&, const DirectoryIPtr&);

    virtual Filesystem::PersistentNodePtr getPersistentNode() const = 0;
    Filesystem::PersistentNodePtr findNode(const Ice::Identity&) const;

    IdentityNodeMap _map;
    DirectoryIPtr _parent;
    bool _destroyed;
    const Ice::Identity _id;
    IceUtil::Mutex _m;
};

typedef IceUtil::Handle<NodeI> NodeIPtr;

class FileI : virtual public Filesystem::File, virtual public NodeI
{
public:

    virtual Filesystem::Lines read(const Ice::Current&);
    virtual void write(const Filesystem::Lines&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    FileI(const Ice::CommunicatorPtr&, const Ice::Identity&,
          const Filesystem::PersistentFilePtr&, const DirectoryIPtr&);

private:

    virtual Filesystem::PersistentNodePtr getPersistentNode() const;

    Filesystem::PersistentFilePtr _file;
};

typedef IceUtil::Handle<FileI> FileIPtr;

class DirectoryI : virtual public Filesystem::Directory, virtual public NodeI
{
public:

    virtual Filesystem::NodeDescSeq list(const Ice::Current&);
    virtual Filesystem::NodeDesc find(const ::std::string&, const Ice::Current&);
    virtual Filesystem::FilePrx createFile(const ::std::string&, const Ice::Current&);
    virtual Filesystem::DirectoryPrx createDirectory(const ::std::string&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    DirectoryI(const Ice::CommunicatorPtr&, const Ice::Identity&,
               const Filesystem::PersistentDirectoryPtr&, const DirectoryIPtr&);

    void addReapEntry(const std::string&);

    static void reap();

    static IceUtil::StaticMutex _lcMutex;

private:

    virtual Filesystem::PersistentNodePtr getPersistentNode() const;

    Filesystem::PersistentDirectoryPtr _dir;

    typedef std::map<DirectoryIPtr, std::vector<std::string> > ReapMap;
    static ReapMap _reapMap;
};

}

#endif
