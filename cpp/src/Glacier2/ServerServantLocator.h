// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ServerBlobject.h>

namespace Glacier
{

class ServerServantLocator : public Ice::ServantLocator
{
public:

    ServerServantLocator(const Ice::ObjectAdapterPtr&);
    
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

private:

    Glacier::ServerBlobjectPtr _blobject;
};

}
