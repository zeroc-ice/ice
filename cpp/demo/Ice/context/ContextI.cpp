// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <ContextI.h>

using namespace std;

void
ContextI::call(const Ice::Current& c)
{
    cout << "Type = ";
    Ice::Context::const_iterator p = c.ctx.find("type");
    if (p != c.ctx.end())
    {
    	cout << p->second;
    }
    else
    {
    	cout << "None";
    }
    cout << endl;
}

void
ContextI::shutdown(const Ice::Current& c)
{
    cout << "Shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}
