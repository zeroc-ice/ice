// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

class FileI : virtual public PersistentFile
{
public:

    FileI();

    virtual std::string name(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    virtual Lines read(const Ice::Current&);
    virtual void write(const Lines&, const Ice::Current&);

    static Freeze::EvictorPtr _evictor;

private:

    bool _destroyed;
    IceUtil::Mutex _mutex;
};

class DirectoryI : virtual public PersistentDirectory
{
public:

    DirectoryI();

    virtual std::string name(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    virtual NodeDescSeq list(const Ice::Current&);
    virtual NodeDesc find(const std::string&, const Ice::Current&);
    virtual DirectoryPrx createDirectory(const std::string&, const Ice::Current&);
    virtual FilePrx createFile(const std::string&, const Ice::Current&);
    virtual void removeNode(const std::string&, const Ice::Current&);

    static Freeze::EvictorPtr _evictor;

public:

    bool _destroyed;
    IceUtil::Mutex _mutex;
};

class NodeFactory : virtual public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();
};

}

#endif
