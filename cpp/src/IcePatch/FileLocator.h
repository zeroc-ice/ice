// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    FileLocator(const Ice::ObjectAdapterPtr&);

    virtual Ice::ObjectPtr locate(const Ice::ObjectAdapterPtr&, const Ice::Current&, Ice::LocalObjectPtr&);

    virtual void finished(const Ice::ObjectAdapterPtr&, const Ice::Current&, const Ice::ObjectPtr&,
			  const Ice::LocalObjectPtr&);

    virtual void deactivate();

private:

    DirectoryPtr _directory; // The stateless default servant for all directories.
    RegularPtr _regular; // The stateless default servant for all regular files.
};

}

#endif
