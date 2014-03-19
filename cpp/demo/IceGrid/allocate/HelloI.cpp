// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

HelloI::HelloI(const string& name) : _name(name)
{
}

void
HelloI::sayHello(const Ice::Current&)
{
    cout << _name << " says Hello World!" << endl;
}

void
HelloI::shutdown(const Ice::Current& c)
{
    cout << _name << " shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}
