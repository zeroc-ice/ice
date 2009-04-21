// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

void
HelloI::sayHello(const Ice::Current& current)
{
    current.adapter->getCommunicator()->getLogger()->print("Hello World!");
}

void
HelloI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->getLogger()->print("Shutting down...");
    current.adapter->getCommunicator()->shutdown();
}
