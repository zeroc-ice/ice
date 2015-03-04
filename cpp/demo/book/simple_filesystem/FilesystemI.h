// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef __FilesystemI_h__
#define __FilesystemI_h__

#include <Ice/Ice.h>
#include <Filesystem.h>

namespace Filesystem
{
    class DirectoryI;
    typedef IceUtil::Handle<DirectoryI> DirectoryIPtr;

    class NodeI : virtual public Node
    {
    public:

        virtual std::string name(const Ice::Current&);
        NodeI(const Ice::CommunicatorPtr&, const std::string&, const DirectoryIPtr&);
        void activate(const Ice::ObjectAdapterPtr&);

    private:

        std::string _name;
        Ice::Identity _id;
        DirectoryIPtr _parent;
        NodeI(const NodeI&);            // Copy forbidden
        void operator=(const NodeI&);   // Assignment forbidden
    };
    typedef IceUtil::Handle<NodeI> NodeIPtr;

    class FileI : virtual public File, virtual public NodeI
    {
    public:

        virtual Lines read(const Ice::Current&);
        virtual void write(const Lines&,
                           const Ice::Current& = Ice::Current());
        FileI(const Ice::CommunicatorPtr&, const std::string&, const DirectoryIPtr&);

    private:

        Lines _lines;
    };
    typedef IceUtil::Handle<FileI> FileIPtr;

    class DirectoryI : virtual public Directory, virtual public NodeI
    {
    public:

        virtual NodeSeq list(const Ice::Current&);
        DirectoryI(const Ice::CommunicatorPtr&, const std::string&, const DirectoryIPtr&);
        void addChild(const Filesystem::NodePrx&);

    private:

        Filesystem::NodeSeq _contents;
    };
}

#endif
