// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
HELLO_API ::IceBox::Service*
create(::Ice::CommunicatorPtr communicator)
{
    return new HelloServiceI;
}

}

HelloServiceI::HelloServiceI()
{
}

HelloServiceI::~HelloServiceI()
{
}

void
HelloServiceI::start(const string& name,
		     const ::Ice::CommunicatorPtr& communicator,
		     const ::Ice::StringSeq& args)
{
    string id = communicator->getProperties()->getProperty("Identity");

    _adapter = communicator->createObjectAdapter(name);
    ::Ice::ObjectPtr object = new HelloI;
    _adapter->add(object, Ice::stringToIdentity(id));
    _adapter->activate();
}

void
HelloServiceI::stop()
{
    _adapter->deactivate();
}
