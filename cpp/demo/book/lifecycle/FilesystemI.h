// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef __FilesystemI_h__
#define __FilesystemI_h__

#include <Ice/Ice.h>
#include <IceUtil/Mutex.h>
#include <Filesystem.h>
#include <map>

namespace FilesystemI
{
    class DirectoryI;
    typedef IceUtil::Handle<DirectoryI> DirectoryIPtr;

    class NodeI : virtual public Filesystem::Node
    {
    public:
        virtual std::string name(const Ice::Current&);
        Ice::Identity id() const;

    protected:
        NodeI(const ::std::string&, const DirectoryIPtr&);

        const ::std::string _name;
        const DirectoryIPtr _parent;
        bool _destroyed;
        Ice::Identity _id;
        IceUtil::Mutex _m;
    };

    typedef IceUtil::Handle<NodeI> NodeIPtr;

    class FileI : virtual public Filesystem::File, virtual public NodeI
    {
    public:
        virtual Filesystem::Lines read(const Ice::Current&);
        virtual void write(const Filesystem::Lines&, const Ice::Current&);
        virtual void destroy(const Ice::Current&);

        FileI(const Ice::ObjectAdapterPtr&, const std::string&, const DirectoryIPtr&);

    private:
        Filesystem::Lines _lines;
    };

    typedef IceUtil::Handle<FileI> FileIPtr;

    class DirectoryI : virtual public NodeI, virtual public Filesystem::Directory
    {
    public:
        virtual Filesystem::NodeDescSeq list(const Ice::Current&);
        virtual Filesystem::NodeDesc find(const std::string& name, const Ice::Current&);
        Filesystem::FilePrx createFile(const ::std::string&, const Ice::Current&);
        Filesystem::DirectoryPrx createDirectory(const ::std::string&, const Ice::Current&);
        virtual void destroy(const Ice::Current&);

        DirectoryI(const Ice::ObjectAdapterPtr&, const std::string& = "/", const DirectoryIPtr& = 0);
        void addChild(const ::std::string&, const NodeIPtr&);

        void addReapEntry(const ::std::string&);

        static IceUtil::StaticMutex _lcMutex;

    private:
        typedef ::std::map< ::std::string, NodeIPtr> Contents;
        Contents _contents;

        typedef ::std::map<DirectoryIPtr, ::std::vector< ::std::string> > ReapMap;
        static ReapMap _reapMap;

        static void reap();
    };
}

#endif
