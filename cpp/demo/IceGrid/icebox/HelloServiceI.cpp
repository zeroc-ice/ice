// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloServiceI.h>
#include <HelloI.h>

using namespace std;

extern "C"
{

//
// Factory function
//
HELLO_API IceBox::Service*
create(Ice::CommunicatorPtr communicator)
{
    return new HelloServiceI;
}

}

void
HelloServiceI::start(const string& name, const Ice::CommunicatorPtr& communicator, const Ice::StringSeq& args)
{
    _adapter = communicator->createObjectAdapter("Hello-" + name);

    
    string helloIdentity = communicator->getProperties()->getProperty("Hello.Identity");

    _adapter->add(new HelloI(name), communicator->stringToIdentity(helloIdentity));
    _adapter->activate();
}

void
HelloServiceI::stop()
{
    _adapter->destroy();
}
