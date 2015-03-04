// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FILESYSTEM_I_H
#define FILESYSTEM_I_H

#include <Filesystem.h>
#include <Freeze/Freeze.h>

namespace Filesystem
{

class FileI : public File
{
public:

    FileI(const Ice::CommunicatorPtr&, const std::string&);

    virtual std::string name(const Ice::Current&);
    virtual Lines read(const Ice::Current&);
    virtual void write(const Lines&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    static std::string filesDB();

private:

    void halt(const Freeze::DatabaseException&) const;

    const Ice::CommunicatorPtr _communicator;
    const std::string _envName;
};

class DirectoryI : public Directory
{
public:

    DirectoryI(const Ice::CommunicatorPtr&, const std::string&);

    virtual std::string name(const Ice::Current&);
    virtual NodeDescSeq list(const Ice::Current&);
    virtual NodeDesc find(const std::string&, const Ice::Current&);
    virtual DirectoryPrx createDirectory(const std::string&, const Ice::Current&);
    virtual FilePrx createFile(const std::string&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    static std::string directoriesDB();

private:

    void halt(const Freeze::DatabaseException&) const;

    const Ice::CommunicatorPtr _communicator;
    const std::string _envName;
};

}

#endif
