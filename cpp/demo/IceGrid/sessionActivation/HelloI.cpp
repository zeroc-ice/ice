// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
HelloI::sayHello(const Ice::Current&) const
{
    cout << _name << " says Hello World!" << endl;
}
