// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH_FILE_LOCATOR_H
#define ICE_PATCH_FILE_LOCATOR_H

#include <Ice/Ice.h>
#include <IcePatch/IcePatch.h>

namespace IcePatch
{

class FileLocator: virtual public Ice::ServantLocator
{
public:

    FileLocator(const Ice::ObjectAdapterPtr&, const std::string&);

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);

    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);

    virtual void deactivate(const std::string&);

private:

    DirectoryPtr _directory; // The stateless default servant for all directories.
    RegularPtr _regular; // The stateless default servant for all regular files.

    const Ice::LoggerPtr _logger;
    const Ice::LoggerPtr _fileTraceLogger;
    const std::string _dir;
};

}

#endif
