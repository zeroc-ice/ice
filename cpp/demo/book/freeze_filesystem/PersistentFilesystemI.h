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
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

namespace Filesystem
{

class NodeI : virtual public PersistentNode,
              public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:

    static Freeze::EvictorPtr _evictor;

protected:

    NodeI();
    NodeI(const Ice::Identity&);

    bool _destroyed;

public:

    const Ice::Identity _id;
};
typedef IceUtil::Handle<NodeI> NodeIPtr;

class FileI : virtual public PersistentFile,
              virtual public NodeI
{
public:

    virtual std::string name(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    virtual Lines read(const Ice::Current&);
    virtual void write(const Lines&, const Ice::Current&);

    FileI();
    FileI(const Ice::Identity&);
};

class DirectoryI : virtual public PersistentDirectory,
                   virtual public NodeI
{
public:

    virtual std::string name(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    virtual NodeDescSeq list(const Ice::Current&);
    virtual NodeDesc find(const std::string&, const Ice::Current&);
    virtual DirectoryPrx createDirectory(const std::string&, const Ice::Current&);
    virtual FilePrx createFile(const std::string&, const Ice::Current&);
    virtual void removeNode(const std::string&, const Ice::Current&);

    DirectoryI();
    DirectoryI(const Ice::Identity&);

};

class NodeFactory : virtual public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();
};

class NodeInitializer : virtual public Freeze::ServantInitializer
{
public:

    virtual void initialize(const Ice::ObjectAdapterPtr&,
                            const Ice::Identity&,
                            const std::string&,
                            const Ice::ObjectPtr&);
};

}

#endif
